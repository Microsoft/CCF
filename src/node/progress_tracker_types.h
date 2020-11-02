// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once
#include "backup_signatures.h"
#include "consensus/aft/revealed_nonces.h"
#include "node_signature.h"
#include "tls/hash.h"
#include "tls/tls.h"
#include "tls/verifier.h"
#include "view_change.h"

namespace ccf
{
  struct BftNodeSignature : public ccf::NodeSignature
  {
    bool is_primary;
    Nonce nonce;

    BftNodeSignature(const NodeSignature& ns) :
      NodeSignature(ns),
      is_primary(false)
    {}

    BftNodeSignature(
      const std::vector<uint8_t>& sig_, NodeId node_, Nonce hashed_nonce_) :
      NodeSignature(sig_, node_, hashed_nonce_),
      is_primary(false)
    {}
  };

  struct CommitCert
  {
    CommitCert(crypto::Sha256Hash& root_, Nonce my_nonce_) :
      root(root_),
      my_nonce(my_nonce_),
      have_primary_signature(true)
    {}

    CommitCert() = default;

    crypto::Sha256Hash root;
    std::map<kv::NodeId, BftNodeSignature> sigs;
    std::set<kv::NodeId> sig_acks;
    std::set<kv::NodeId> nonce_set;
    std::map<kv::NodeId, Nonce> unmatched_nonces;
    Nonce my_nonce;
    bool have_primary_signature = false;
    bool ack_sent = false;
    bool reply_and_nonce_sent = false;
    bool nonces_committed_to_ledger = false;
  };

  class ProgressTrackerStore
  {
  public:
    virtual ~ProgressTrackerStore() = default;
    virtual void write_backup_signatures(ccf::BackupSignatures& sig_value) = 0;
    virtual std::optional<ccf::BackupSignatures> get_backup_signatures() = 0;
    virtual void write_nonces(aft::RevealedNonces& nonces) = 0;
    virtual std::optional<aft::RevealedNonces> get_nonces() = 0;
    virtual bool verify_signature(
      kv::NodeId node_id,
      crypto::Sha256Hash& root,
      uint32_t sig_size,
      uint8_t* sig) = 0;
    virtual void sign_view_change(
      ViewChange& view_change,
      kv::Consensus::View view,
      kv::Consensus::SeqNo seqno,
      crypto::Sha256Hash& root) = 0;
    virtual bool verify_view_change(
      ViewChange& view_change,
      kv::NodeId from,
      kv::Consensus::View view,
      kv::Consensus::SeqNo seqno,
      crypto::Sha256Hash& root) = 0;
  };

  class ProgressTrackerStoreAdapter : public ProgressTrackerStore
  {
  public:
    ProgressTrackerStoreAdapter(
      kv::AbstractStore& store_,
      tls::KeyPair& kp_,
      ccf::Nodes& nodes_,
      ccf::BackupSignaturesMap& backup_signatures_,
      aft::RevealedNoncesMap& revealed_nonces_) :
      store(store_),
      kp(kp_),
      nodes(nodes_),
      backup_signatures(backup_signatures_),
      revealed_nonces(revealed_nonces_)
    {}

    void write_backup_signatures(ccf::BackupSignatures& sig_value) override
    {
      kv::Tx tx(&store);
      auto backup_sig_view = tx.get_view(backup_signatures);

      backup_sig_view->put(0, sig_value);
      auto r = tx.commit();
      LOG_TRACE_FMT("Adding signatures to ledger, result:{}", r);
      CCF_ASSERT_FMT(
        r == kv::CommitSuccess::OK,
        "Commiting backup signatures failed r:{}",
        r);
    }

    std::optional<ccf::BackupSignatures> get_backup_signatures() override
    {
      kv::Tx tx(&store);
      auto sigs_tv = tx.get_view(backup_signatures);
      auto sigs = sigs_tv->get(0);
      if (!sigs.has_value())
      {
        LOG_FAIL_FMT("No signatures found in signatures map");
        throw ccf::ccf_logic_error("No signatures found in signatures map");
      }
      return sigs;
    }

    void write_nonces(aft::RevealedNonces& nonces) override
    {
      kv::Tx tx(&store);
      auto nonces_tv = tx.get_view(revealed_nonces);

      nonces_tv->put(0, nonces);
      auto r = tx.commit();
      if (r != kv::CommitSuccess::OK)
      {
        LOG_FAIL_FMT(
          "Failed to write nonces, view:{}, seqno:{}",
          nonces.tx_id.term,
          nonces.tx_id.version);
        throw ccf::ccf_logic_error(fmt::format(
          "Failed to write nonces, view:{}, seqno:{}",
          nonces.tx_id.term,
          nonces.tx_id.version));
      }
    }

    std::optional<aft::RevealedNonces> get_nonces() override
    {
      kv::Tx tx(&store);
      auto nonces_tv = tx.get_view(revealed_nonces);
      auto nonces = nonces_tv->get(0);
      if (!nonces.has_value())
      {
        LOG_FAIL_FMT("No signatures found in signatures map");
        throw ccf::ccf_logic_error("No signatures found in signatures map");
      }
      return nonces;
    }

    bool verify_signature(
      kv::NodeId node_id,
      crypto::Sha256Hash& root,
      uint32_t sig_size,
      uint8_t* sig) override
    {
      kv::Tx tx(&store);
      auto ni_tv = tx.get_view(nodes);

      auto ni = ni_tv->get(node_id);
      if (!ni.has_value())
      {
        LOG_FAIL_FMT(
          "No node info, and therefore no cert for node {}", node_id);
        return false;
      }
      tls::VerifierPtr from_cert = tls::make_verifier(ni.value().cert);
      return from_cert->verify_hash(
        root.h.data(), root.h.size(), sig, sig_size);
    }

    void sign_view_change(
      ViewChange& view_change,
      kv::Consensus::View view,
      kv::Consensus::SeqNo seqno,
      crypto::Sha256Hash& root) override
    {
      crypto::Sha256Hash h = hash_view_change(view_change, view, seqno, root);
      view_change.signature = kp.sign_hash(h.h.data(), h.h.size());
    }

    bool verify_view_change(
      ViewChange& view_change,
      kv::NodeId from,
      kv::Consensus::View view,
      kv::Consensus::SeqNo seqno,
      crypto::Sha256Hash& root

      ) override
    {
      crypto::Sha256Hash h = hash_view_change(view_change, view, seqno, root);

      kv::Tx tx(&store);
      auto ni_tv = tx.get_view(nodes);

      auto ni = ni_tv->get(from);
      if (!ni.has_value())
      {
        LOG_FAIL_FMT("No node info, and therefore no cert for node {}", from);
        return false;
      }
      tls::VerifierPtr from_cert = tls::make_verifier(ni.value().cert);
      return from_cert->verify_hash(
        h.h.data(),
        h.h.size(),
        view_change.signature.data(),
        view_change.signature.size());
    }

  private:
    kv::AbstractStore& store;
    tls::KeyPair& kp;
    ccf::Nodes& nodes;
    ccf::BackupSignaturesMap& backup_signatures;
    aft::RevealedNoncesMap& revealed_nonces;

    crypto::Sha256Hash hash_view_change(
      const ViewChange& v,
      kv::Consensus::View view,
      kv::Consensus::SeqNo seqno,
      crypto::Sha256Hash& root) const
    {
      crypto::CSha256Hash ch;

      ch.update(view);
      ch.update(seqno);
      ch.update(root);

      for (auto& s : v.signatures)
      {
        ch.update(s.sig);
      }

      return ch.finalize();
    }
  };
}
