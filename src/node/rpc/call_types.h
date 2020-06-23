// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once
#include "ds/json_schema.h"
#include "kv/kv_types.h"
#include "node/identity.h"
#include "node/ledger_secrets.h"
#include "node/nodes.h"
#include "node_call_types.h"
#include "tx_status.h"

#include <nlohmann/json.hpp>

namespace ccf
{
  struct GetCommit
  {
    struct Out
    {
      kv::Consensus::View view;
      kv::Consensus::SeqNo seqno;
    };
  };

  struct GetTxStatus
  {
    struct In
    {
      kv::Consensus::View view;
      kv::Consensus::SeqNo seqno;
    };

    struct Out
    {
      TxStatus status;
    };
  };

  struct GetMetrics
  {
    struct HistogramResults
    {
      int low = {};
      int high = {};
      size_t overflow = {};
      size_t underflow = {};
      nlohmann::json buckets = {};
    };

    struct Out
    {
      HistogramResults histogram;
      nlohmann::json tx_rates;
    };
  };

  struct GetPrimaryInfo
  {
    struct Out
    {
      NodeId primary_id;
      std::string primary_host;
      std::string primary_port;
      kv::Consensus::View current_view;
    };
  };

  struct GetNetworkInfo
  {
    struct NodeInfo
    {
      NodeId node_id;
      std::string host;
      std::string port;
    };

    struct Out
    {
      std::vector<NodeInfo> nodes = {};
      std::optional<NodeId> primary_id = {};
    };
  };

  struct CallerInfo
  {
    CallerId caller_id;
  };

  struct GetUserId
  {
    struct In
    {
      std::vector<uint8_t> cert;
    };

    using Out = CallerInfo;
  };

  struct ListMethods
  {
    struct Out
    {
      std::vector<std::string> methods;
    };
  };

  struct GetSchema
  {
    struct In
    {
      std::string method = {};
    };

    struct Out
    {
      ds::json::JsonSchema params_schema = {};
      ds::json::JsonSchema result_schema = {};
    };
  };

  struct GetReceipt
  {
    struct In
    {
      int64_t commit = 0;
    };

    struct Out
    {
      std::vector<std::uint8_t> receipt = {};
    };
  };

  struct VerifyReceipt
  {
    struct In
    {
      std::vector<std::uint8_t> receipt = {};
    };

    struct Out
    {
      bool valid = false;
    };
  };
}