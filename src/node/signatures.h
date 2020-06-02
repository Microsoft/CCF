// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once
#include "crypto/hash.h"
#include "entities.h"
#include "kv/map.h"
#include "raw_signature.h"

#include <msgpack/msgpack.hpp>
#include <string>
#include <vector>

namespace ccf
{
  struct Signature : public RawSignature
  {
    NodeId node;
    ObjectId seqno = 0;
    ObjectId view = 0;
    ObjectId commit_seqno = 0;
    ObjectId commit_view = 0;
    crypto::Sha256Hash root;
    std::vector<uint8_t> tree = {0};

    MSGPACK_DEFINE(
      MSGPACK_BASE(RawSignature),
      node,
      seqno,
      view,
      commit_seqno,
      commit_view,
      root,
      tree);

    Signature() {}

    Signature(NodeId node_, ObjectId seqno_) : node(node_), seqno(seqno_) {}

    Signature(const crypto::Sha256Hash& root_) : root(root_) {}

    Signature(
      NodeId node_,
      ObjectId seqno_,
      ObjectId view_,
      ObjectId commit_seqno_,
      ObjectId commit_view_,
      const crypto::Sha256Hash root_,
      const std::vector<uint8_t>& sig_,
      const std::vector<uint8_t>& tree_) :
      RawSignature{sig_},
      node(node_),
      seqno(seqno_),
      view(view_),
      commit_seqno(commit_seqno_),
      commit_view(commit_view_),
      root(root_),
      tree(tree_)
    {}
  };
  DECLARE_JSON_TYPE_WITH_BASE(Signature, RawSignature)
  DECLARE_JSON_REQUIRED_FIELDS(
    Signature, node, seqno, view, commit_seqno, commit_view, root)
  using Signatures = kv::Map<ObjectId, Signature>;
}