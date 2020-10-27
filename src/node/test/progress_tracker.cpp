// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.

#include "node/progress_tracker.h"

#include "kv/store.h"
#include "node/nodes.h"
#include "node/request_tracker.h"
#include "consensus/aft/impl/view_change_tracker.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <string>
#include <trompeloeil/include/trompeloeil.hpp>

class StoreMock : public ccf::ProgressTrackerStore
{
public:
  MAKE_MOCK1(write_backup_signatures, void(ccf::BackupSignatures&), override);
  MAKE_MOCK0(
    get_backup_signatures, std::optional<ccf::BackupSignatures>(), override);
  MAKE_MOCK1(write_nonces, void(aft::RevealedNonces&), override);
  MAKE_MOCK0(get_nonces, std::optional<aft::RevealedNonces>(), override);
  MAKE_MOCK4(
    verify_signature,
    bool(kv::NodeId, crypto::Sha256Hash&, uint32_t, uint8_t*),
    override);

  MAKE_MOCK1(sign_view_change, void(ccf::ViewChange& view_change), override);
};

void ordered_execution(
  uint32_t my_node_id, std::unique_ptr<ccf::ProgressTracker> pt)
{
  kv::Consensus::View view = 0;
  kv::Consensus::SeqNo seqno = 42;
  uint32_t node_count = 4;
  uint32_t node_count_quorum =
    2; // Takes into account that counting starts at 0
  bool am_i_primary = (my_node_id == 0);

  crypto::Sha256Hash root;
  std::array<uint8_t, MBEDTLS_ECDSA_MAX_LEN> sig;
  ccf::Nonce nonce;
  auto h = pt->hash_data(nonce);
  ccf::Nonce hashed_nonce;
  std::copy(h.h.begin(), h.h.end(), hashed_nonce.h.begin());

  INFO("Adding signatures");
  {
    auto result =
      pt->record_primary({view, seqno}, 0, root, hashed_nonce, node_count);
    REQUIRE(result == kv::TxHistory::Result::OK);

    for (uint32_t i = 1; i < node_count; ++i)
    {
      if (i == my_node_id)
      {
        auto h = pt->get_my_hashed_nonce({view, seqno});
        std::copy(h.h.begin(), h.h.end(), hashed_nonce.h.begin());
      }
      else
      {
        std::copy(h.h.begin(), h.h.end(), hashed_nonce.h.begin());
      }

      auto result = pt->add_signature(
        {view, seqno},
        i,
        MBEDTLS_ECDSA_MAX_LEN,
        sig,
        hashed_nonce,
        node_count,
        am_i_primary);
      REQUIRE(
        ((result == kv::TxHistory::Result::OK && i != node_count_quorum) ||
         (result == kv::TxHistory::Result::SEND_SIG_RECEIPT_ACK &&
          i == node_count_quorum)));
    }
  }

  INFO("Add signature acks");
  {
    for (uint32_t i = 0; i < node_count; ++i)
    {
      auto result = pt->add_signature_ack({view, seqno}, i, node_count);
      REQUIRE(
        ((result == kv::TxHistory::Result::OK && i != node_count_quorum) ||
         (result == kv::TxHistory::Result::SEND_REPLY_AND_NONCE &&
          i == node_count_quorum)));
    }
  }

  INFO("Add nonces here");
  {
    for (uint32_t i = 0; i < node_count; ++i)
    {
      if (my_node_id == i)
      {
        pt->add_nonce_reveal(
          {view, seqno},
          pt->get_my_nonce({view, seqno}),
          i,
          node_count,
          am_i_primary);
      }
      else
      {
        pt->add_nonce_reveal({view, seqno}, nonce, i, node_count, am_i_primary);
      }

      if (i < 2)
      {
        REQUIRE(pt->get_highest_committed_nonce() == 0);
      }
      else
      {
        REQUIRE(pt->get_highest_committed_nonce() == seqno);
      }
    }
  }
}

void ordered_execution_primary(
  uint32_t my_node_id,
  std::unique_ptr<ccf::ProgressTracker> pt,
  StoreMock& store_mock)
{
  using trompeloeil::_;

  REQUIRE_CALL(store_mock, write_backup_signatures(_));
  REQUIRE_CALL(store_mock, write_nonces(_));

  ordered_execution(my_node_id, std::move(pt));
}

void run_ordered_execution(uint32_t my_node_id)
{
  using trompeloeil::_;

  auto store = std::make_unique<StoreMock>();
  StoreMock& store_mock = *store.get();
  auto pt =
    std::make_unique<ccf::ProgressTracker>(std::move(store), my_node_id);

  REQUIRE_CALL(store_mock, verify_signature(_, _, _, _))
    .RETURN(true)
    .TIMES(AT_LEAST(2));

  if (my_node_id == 0)
  {
    ordered_execution_primary(my_node_id, std::move(pt), store_mock);
  }
  else
  {
    ordered_execution(my_node_id, std::move(pt));
  }
}

TEST_CASE("Ordered Execution")
{
  for (uint32_t i = 0; i < 4; ++i)
  {
    run_ordered_execution(i);
  }
}

TEST_CASE("Request tracker")
{
  INFO("Can add and remove from progress tracker");
  {
    aft::RequestTracker t;
    crypto::Sha256Hash h;
    h.h.fill(0);
    for (uint32_t i = 0; i < 10; ++i)
    {
      h.h[0] = i;
      t.insert(h, std::chrono::milliseconds(i));
      REQUIRE(t.oldest_entry() == std::chrono::milliseconds(0));
    }

    h.h[0] = 2;
    REQUIRE(t.remove(h));
    REQUIRE(t.oldest_entry() == std::chrono::milliseconds(0));

    h.h[0] = 0;
    REQUIRE(t.remove(h));
    REQUIRE(t.oldest_entry() == std::chrono::milliseconds(1));

    h.h[0] = 99;
    REQUIRE(t.remove(h) == false);
    REQUIRE(t.oldest_entry() == std::chrono::milliseconds(1));
  }

  INFO("Entry that was deleted is not tracked after it is added");
  {
    aft::RequestTracker t;
    crypto::Sha256Hash h;
    h.h.fill(0);
    REQUIRE(t.oldest_entry().has_value() == false);

    h.h[0] = 0;
    REQUIRE(t.remove(h) == false);
    t.insert_deleted(h, std::chrono::milliseconds(100));
    t.insert(h, std::chrono::milliseconds(0));
    REQUIRE(t.oldest_entry().has_value() == false);

    h.h[1] = 1;
    REQUIRE(t.remove(h) == false);
    t.insert_deleted(h, std::chrono::milliseconds(100));
    t.tick(std::chrono::milliseconds(120));
    t.insert(h, std::chrono::milliseconds(0));
    REQUIRE(t.oldest_entry().has_value() == false);

    h.h[2] = 2;
    REQUIRE(t.remove(h) == false);
    t.insert_deleted(h, std::chrono::milliseconds(100));
    t.tick(std::chrono::minutes(3));
    REQUIRE(t.is_empty());
    t.insert(h, std::chrono::milliseconds(0));
    REQUIRE(t.oldest_entry().has_value());
  }

  INFO("Can enter multiple items");
  {
    aft::RequestTracker t;
    crypto::Sha256Hash h;
    h.h.fill(0);

    t.insert(h, std::chrono::milliseconds(0));

    for (uint32_t i = 1; i < 4; ++i)
    {
      h.h[0] = 1;
      t.insert(h, std::chrono::milliseconds(i));
    }

    h.h[0] = 2;
    t.insert(h, std::chrono::milliseconds(4));
    REQUIRE(t.oldest_entry() == std::chrono::milliseconds(0));

    h.h[0] = 1;
    REQUIRE(t.remove(h));
    REQUIRE(t.oldest_entry() == std::chrono::milliseconds(0));

    h.h[0] = 0;
    t.remove(h);
    REQUIRE(t.oldest_entry() == std::chrono::milliseconds(2));

    h.h[0] = 1;
    t.remove(h);
    REQUIRE(t.oldest_entry() == std::chrono::milliseconds(3));
    t.remove(h);
    REQUIRE(t.oldest_entry() == std::chrono::milliseconds(4));
    t.remove(h);
    REQUIRE(!t.is_empty());

    h.h[0] = 2;
    t.remove(h);
    REQUIRE(t.is_empty());
  }

  INFO("Verify seqno and time of last sig stored correctly");
  {
    aft::RequestTracker t;

    auto r = t.get_seqno_time_last_request();
    REQUIRE(std::get<0>(r) == -1);
    REQUIRE(std::get<1>(r) == std::chrono::milliseconds(0));

    t.insert_signed_request(2, std::chrono::milliseconds(2));
    r = t.get_seqno_time_last_request();
    REQUIRE(std::get<0>(r) == 2);
    REQUIRE(std::get<1>(r) == std::chrono::milliseconds(2));

    t.insert_signed_request(1, std::chrono::milliseconds(1));
    r = t.get_seqno_time_last_request();
    REQUIRE(std::get<0>(r) == 2);
    REQUIRE(std::get<1>(r) == std::chrono::milliseconds(2));
  }
}

TEST_CASE("View Changes")
{
  using trompeloeil::_;

  uint32_t my_node_id = 0;
  auto store = std::make_unique<StoreMock>();
  StoreMock& store_mock = *store.get();
  ccf::ProgressTracker pt(std::move(store), my_node_id);

  kv::Consensus::View view = 0;
  kv::Consensus::SeqNo seqno = 42;
  uint32_t node_count = 4;
  uint32_t node_count_quorum =
    2; // Takes into account that counting starts at 0
  crypto::Sha256Hash root;
  root.h.fill(1);
  ccf::Nonce nonce;
  auto h = pt.hash_data(nonce);
  ccf::Nonce hashed_nonce;
  std::copy(h.h.begin(), h.h.end(), hashed_nonce.h.begin());
  std::array<uint8_t, MBEDTLS_ECDSA_MAX_LEN> sig;

  INFO("find first view-change message");
  {
    REQUIRE_CALL(store_mock, verify_signature(_, _, _, _))
      .RETURN(true)
      .TIMES(AT_LEAST(2));
    REQUIRE_CALL(store_mock, sign_view_change(_)).TIMES(AT_LEAST(2));
    auto result =
      pt.record_primary({view, seqno}, 0, root, hashed_nonce, node_count);
    REQUIRE(result == kv::TxHistory::Result::OK);

    for (uint32_t i = 1; i < node_count; ++i)
    {
      auto result = pt.add_signature(
        {view, seqno},
        i,
        MBEDTLS_ECDSA_MAX_LEN,
        sig,
        hashed_nonce,
        node_count,
        false);
      REQUIRE(
        ((result == kv::TxHistory::Result::OK && i != node_count_quorum) ||
         (result == kv::TxHistory::Result::SEND_SIG_RECEIPT_ACK &&
          i == node_count_quorum)));

      if (i < 2)
      {
        CHECK_THROWS(pt.get_view_change_message(view));
      }
      else
      {
        auto vc = pt.get_view_change_message(view);
        REQUIRE(vc != nullptr);
        REQUIRE(vc->view == view);
        REQUIRE(vc->seqno == seqno);
        REQUIRE(vc->root == root);
      }
    }
  }

  INFO("Update latest prepared");
  {
    kv::Consensus::SeqNo new_seqno = 84;

    REQUIRE_CALL(store_mock, verify_signature(_, _, _, _))
      .RETURN(true)
      .TIMES(AT_LEAST(2));
    REQUIRE_CALL(store_mock, sign_view_change(_)).TIMES(AT_LEAST(2));
    auto result =
      pt.record_primary({view, new_seqno}, 0, root, hashed_nonce, node_count);
    REQUIRE(result == kv::TxHistory::Result::OK);

    for (uint32_t i = 1; i < node_count; ++i)
    {
      auto result = pt.add_signature(
        {view, new_seqno},
        i,
        MBEDTLS_ECDSA_MAX_LEN,
        sig,
        hashed_nonce,
        node_count,
        false);
      REQUIRE(
        ((result == kv::TxHistory::Result::OK && i != node_count_quorum) ||
         (result == kv::TxHistory::Result::SEND_SIG_RECEIPT_ACK &&
          i == node_count_quorum)));

      if (i < 2)
      {
        auto vc = pt.get_view_change_message(view);
        REQUIRE(vc != nullptr);
        REQUIRE(vc->seqno == seqno);
      }
      else
      {
        auto vc = pt.get_view_change_message(view);
        REQUIRE(vc != nullptr);
        REQUIRE(vc->seqno == new_seqno);
      }
    }
    seqno = new_seqno;
  }

  INFO("Update older prepared");
  {
    kv::Consensus::SeqNo new_seqno = 21;

    REQUIRE_CALL(store_mock, verify_signature(_, _, _, _))
      .RETURN(true)
      .TIMES(AT_LEAST(2));
    REQUIRE_CALL(store_mock, sign_view_change(_)).TIMES(AT_LEAST(2));
    auto result =
      pt.record_primary({view, new_seqno}, 0, root, hashed_nonce, node_count);
    REQUIRE(result == kv::TxHistory::Result::OK);

    for (uint32_t i = 1; i < node_count; ++i)
    {
      auto result = pt.add_signature(
        {view, new_seqno},
        i,
        MBEDTLS_ECDSA_MAX_LEN,
        sig,
        hashed_nonce,
        node_count,
        false);
      REQUIRE(
        ((result == kv::TxHistory::Result::OK && i != node_count_quorum) ||
         (result == kv::TxHistory::Result::SEND_SIG_RECEIPT_ACK &&
          i == node_count_quorum)));

      auto vc = pt.get_view_change_message(view);
      REQUIRE(vc != nullptr);
      REQUIRE(vc->seqno == seqno);
    }
  }
}

TEST_CASE("Serialization")
{
  std::vector<uint8_t> serialized;
  INFO("view-change serialization");
  {
    ccf::ViewChange v;
    v.view = 1;
    v.seqno = 2;
    v.root.h.fill(3);
    
    for (uint32_t i = 10; i < 110; i += 10)
    {
      ccf::Nonce n;
      n.h.fill(i+2);
      v.signatures.push_back({{static_cast<uint8_t>(i)}, i + 1, n});
    }

    v.signature = {5};
    serialized.resize(v.get_serialized_size());

    uint8_t* data = serialized.data();
    size_t size = serialized.size();

    v.serialize(data, size);
    REQUIRE(size == 0);
  }

  INFO("view-change deserialization");
  {
    const uint8_t* data = serialized.data();
    size_t size = serialized.size();
    ccf::ViewChange v = ccf::ViewChange::deserialize(data, size);
    REQUIRE(v.view == 1);
    REQUIRE(v.seqno == 2);
    crypto::Sha256Hash h;
    h.h.fill(3);
    REQUIRE(v.root.h == h.h);

    REQUIRE(v.signatures.size() == 10);
    for (uint32_t i = 1; i < 11; ++i)
    {
      ccf::Nonce n;
      n.h.fill(i*10+2);
      ccf::NodeSignature& ns = v.signatures[i-1];
      REQUIRE(ns.sig.size() == 1);
      REQUIRE(ns.sig[0] == i*10);
      REQUIRE(ns.node == i*10+1);
      REQUIRE(ns.hashed_nonce.h == n.h);
    }

    REQUIRE(v.signature.size() == 1);
    REQUIRE(v.signature[0] == 5);
  }
}

TEST_CASE("view-change-tracker tests")
{
  INFO("Check timeout works correctly");
  {
    aft::ViewChangeTracker vct(0, 0, std::chrono::seconds(10));
    REQUIRE(vct.should_send_view_change(std::chrono::seconds(1)) == false);
    REQUIRE(vct.get_target_view() == 0);
    REQUIRE(vct.should_send_view_change(std::chrono::seconds(11)));
    REQUIRE(vct.get_target_view() == 1);
    REQUIRE(vct.should_send_view_change(std::chrono::seconds(12)) == false);
    REQUIRE(vct.get_target_view() == 1);
    REQUIRE(vct.should_send_view_change(std::chrono::seconds(100)));
    REQUIRE(vct.get_target_view() == 2);
  }
}