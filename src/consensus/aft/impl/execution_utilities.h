// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "consensus/aft/raft_types.h"
#include "consensus/pbft/pbft_requests.h"
#include "enclave/rpc_map.h"

namespace enclave
{
  class RPCSessions;
  class RPCMap;
}

namespace aft
{
  class RequestMessage;

  class ExecutionUtilities
  {
  public:
    static std::unique_ptr<RequestCtx> create_request_ctx(
      uint8_t* req_start,
      size_t req_size,
      std::shared_ptr<enclave::RPCMap>& rpc_map);

    static std::unique_ptr<RequestCtx> create_request_ctx(
      pbft::Request& request, std::shared_ptr<enclave::RPCMap>& rpc_map);

    static kv::Version execute_request(
      std::unique_ptr<RequestMessage> request, bool is_create_request);

    static std::unique_ptr<aft::RequestMessage> create_request_message(
      const kv::TxHistory::RequestCallbackArgs& args,
      std::shared_ptr<enclave::RPCSessions>& rpc_sessions,
      std::shared_ptr<enclave::RPCMap>& rpc_map);
  };
}
