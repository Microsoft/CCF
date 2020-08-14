// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "kv/kv_types.h"
#include "enclave/rpc_context.h"
#include "enclave/rpc_handler.h"
#include "kv/store.h"

#include <functional>
#include <vector>

namespace ccf
{
  class NodeToNode;
}

namespace aft
{
  enum AftMsgType : ccf::Node2NodeMsg
  {
    aft_message = 1000,
    encrypted_aft_message
  };

#pragma pack(push, 1)
  struct AftHeader
  {
    AftMsgType msg;
    kv::NodeId from_node;
  };

#pragma pack(pop)
  class RequestMessage;
  class EnclaveNetwork;
  struct RequestCtx
  {
    std::shared_ptr<enclave::RpcContext> ctx;
    std::shared_ptr<enclave::RpcHandler> frontend;
  };

  using ReplyCallback = std::function<bool(
    void* owner,
    kv::TxHistory::RequestID caller_rid,
    int status,
    std::vector<uint8_t>& data)>;

  class IStore
  {
  public:
    virtual ~IStore() = default;
    virtual void compact(kv::Version v) = 0;
    virtual kv::Version current_version() = 0;
  };

  class IStateMachine
  {
  public:
    IStateMachine() = default;
    virtual ~IStateMachine() = default;

    virtual void receive_request(std::unique_ptr<RequestMessage> request) = 0;
    virtual void receive_message(OArray&& oa, kv::NodeId from) = 0;
    virtual void add_node(kv::NodeId node_id, const std::vector<uint8_t>& cert) = 0;
    virtual bool is_primary() = 0;
    virtual kv::NodeId primary() = 0;
    virtual kv::Consensus::View view() = 0;
    virtual kv::Consensus::View get_view_for_version(kv::Version version) = 0;
    virtual kv::Version get_last_committed_version() = 0;
    virtual void attempt_to_open_network() = 0;
  };

  std::unique_ptr<IStateMachine> create_state_machine(
    kv::NodeId my_node_id,
    const std::vector<uint8_t>& cert,
    IStore& store,
    std::shared_ptr<EnclaveNetwork> network);

  std::unique_ptr<IStore> create_store_adaptor(
    std::shared_ptr<kv::Store> store);
}