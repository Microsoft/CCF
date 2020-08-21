// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "consensus/consensus_types.h"
#include "ds/ring_buffer_types.h"
#include "enclave/rpc_context.h"
#include "enclave/rpc_handler.h"
#include "kv/kv_types.h"

#include <chrono>
#include <cstdint>
#include <limits>

namespace aft
{
  using Index = int64_t;
  using Term = int64_t;
  using NodeId = uint64_t;
  using Node2NodeMsg = uint64_t;

  using ReplyCallback = std::function<bool(
    void* owner,
    kv::TxHistory::RequestID caller_rid,
    int status,
    std::vector<uint8_t>& data)>;


  static constexpr NodeId NoNode = std::numeric_limits<NodeId>::max();

  template <typename S>
  class Store
  {
  public:
    virtual ~Store() {}
    virtual S deserialise(
      const std::vector<uint8_t>& data,
      bool public_only = false,
      Term* term = nullptr) = 0;
    virtual void compact(Index v) = 0;
    virtual void rollback(Index v, std::optional<Term> t = std::nullopt) = 0;
    virtual void set_term(Term t) = 0;
    virtual kv::Version current_version() = 0;
  };

  template <typename T, typename S>
  class Adaptor : public Store<S>
  {
  private:
    std::weak_ptr<T> x;

  public:
    Adaptor(std::shared_ptr<T> x) : x(x) {}

    S deserialise(
      const std::vector<uint8_t>& data,
      bool public_only = false,
      Term* term = nullptr) override
    {
      auto p = x.lock();
      if (p)
      {
        return p->deserialise(data, public_only, term);
      }
      return S::FAILED;
    }

    void compact(Index v) override
    {
      auto p = x.lock();
      if (p)
      {
        p->compact(v);
      }
    }

    void rollback(Index v, std::optional<Term> t = std::nullopt) override
    {
      auto p = x.lock();
      if (p)
      {
        p->rollback(v, t);
      }
    }

    void set_term(Term t) override
    {
      auto p = x.lock();
      if (p)
      {
        p->set_term(t);
      }
    }

    kv::Version current_version() override
    {
      auto p = x.lock();
      if (p)
      {
        return p->current_version();
      }
      return kv::NoVersion;
    }
  };

  enum RaftMsgType : Node2NodeMsg
  {
    raft_append_entries = 0,
    raft_append_entries_response,
    raft_request_vote,
    raft_request_vote_response,

    bft_Request,
    bft_Status,
    bft_RequestData,
    bft_OpenNetwork,
    bft_OpenNetworkResp,
  };

#pragma pack(push, 1)
  struct RaftHeader
  {
    RaftMsgType msg;
    NodeId from_node;
  };

  struct AppendEntries : consensus::ConsensusHeader<RaftMsgType>,
                         consensus::AppendEntriesIndex
  {
    Term term;
    Term prev_term;
    Index leader_commit_idx;
    Term term_of_idx;
  };

  struct AppendEntriesResponse : RaftHeader
  {
    Term term;
    Index last_log_idx;
    bool success;
  };

  struct RequestVote : RaftHeader
  {
    Term term;
    // last_log_idx in vanilla raft but last_commit_idx here to preserve
    // verifiability
    Index last_commit_idx;
    // last_log_term in vanilla raft but last_commit_term here to preserve
    // verifiability
    Term last_commit_term;
  };

  struct RequestVoteResponse : RaftHeader
  {
    Term term;
    bool vote_granted;
  };

  struct RequestCtx
  {
    std::shared_ptr<enclave::RpcContext> ctx;
    std::shared_ptr<enclave::RpcHandler> frontend;
  };
#pragma pack(pop)

  class RequestMessage;

  class StateMachine
  {
  public:
    StateMachine() = default;
    virtual ~StateMachine() = default;

    virtual void receive_request(std::unique_ptr<RequestMessage> request) = 0;
    virtual void receive_message(OArray oa, kv::NodeId from) = 0;
    virtual void receive_message(
      OArray oa, AppendEntries ae, kv::NodeId from) = 0;
    virtual void add_node(
      kv::NodeId node_id, const std::vector<uint8_t>& cert) = 0;
    virtual bool is_primary() = 0;
    virtual kv::NodeId primary() = 0;
    virtual kv::Consensus::View view() = 0;
    virtual kv::Consensus::View get_view_for_version(kv::Version version) = 0;
    virtual kv::Version get_last_committed_version() = 0;
    virtual void attempt_to_open_network() = 0;
  };
}