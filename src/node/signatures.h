// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once
#include "entities.h"
#include "rawsignature.h"

#include <msgpack-c/msgpack.hpp>
#include <string>
#include <vector>

namespace ccf
{
  struct Signature : public RawSignature
  {
    NodeId node;
    ObjectId index;
    ObjectId term;
    ObjectId commit;

    MSGPACK_DEFINE(node, index, term, commit, sig);

    Signature() {}

    Signature(NodeId node_, ObjectId index_) :
      node(node_),
      index(index_),
      term(0),
      commit(0)
    {}

    Signature(
      NodeId node_,
      ObjectId index_,
      ObjectId term_,
      ObjectId commit_,
      const std::vector<uint8_t> sig_) :
      RawSignature{sig_},
      node(node_),
      index(index_),
      term(term_),
      commit(commit_)
    {}
  };
  DECLARE_REQUIRED_JSON_FIELDS_WITH_BASE(
    Signature, RawSignature, node, index, term, commit)
  using Signatures = Store::Map<ObjectId, Signature>;
}