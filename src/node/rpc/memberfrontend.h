// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once
#include "frontend.h"
#include "luainterp/txscriptrunner.h"
#include "node/genesisgen.h"
#include "node/members.h"
#include "node/nodes.h"
#include "node/quoteverification.h"
#include "tls/keypair.h"

#include <exception>
#include <initializer_list>
#include <map>
#include <memory>
#include <set>
#include <sstream>

namespace ccf
{
  struct SetUserData
  {
    UserId user_id;
    nlohmann::json user_data = nullptr;
  };
  DECLARE_JSON_TYPE_WITH_OPTIONAL_FIELDS(SetUserData)
  DECLARE_JSON_REQUIRED_FIELDS(SetUserData, user_id)
  DECLARE_JSON_OPTIONAL_FIELDS(SetUserData, user_data)

  class MemberHandlers : public CommonHandlerRegistry
  {
  private:
    Script get_script(Store::Tx& tx, std::string name)
    {
      const auto s = tx.get_view(network.gov_scripts)->get(name);
      if (!s)
      {
        throw std::logic_error(
          fmt::format("Could not find gov script: {}", name));
      }
      return *s;
    }

    void set_app_scripts(
      Store::Tx& tx, std::map<std::string, std::string> scripts)
    {
      auto tx_scripts = tx.get_view(network.app_scripts);

      // First, remove all existing handlers
      tx_scripts->foreach(
        [&tx_scripts](const std::string& name, const Script& script) {
          tx_scripts->remove(name);
          return true;
        });

      for (auto& rs : scripts)
      {
        tx_scripts->put(rs.first, lua::compile(rs.second));
      }
    }

    void set_js_scripts(
      Store::Tx& tx, std::map<std::string, std::string> scripts)
    {
      auto tx_scripts = tx.get_view(network.app_scripts);

      // First, remove all existing handlers
      tx_scripts->foreach(
        [&tx_scripts](const std::string& name, const Script& script) {
          tx_scripts->remove(name);
          return true;
        });

      for (auto& rs : scripts)
      {
        tx_scripts->put(rs.first, {rs.second});
      }
    }

    //! Table of functions that proposal scripts can propose to invoke
    const std::unordered_map<
      std::string,
      std::function<bool(Store::Tx&, const nlohmann::json&)>>
      hardcoded_funcs = {
        // set the lua application script
        {"set_lua_app",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           const std::string app = args;
           set_app_scripts(tx, lua::Interpreter().invoke<nlohmann::json>(app));

           return true;
         }},
        // set the js application script
        {"set_js_app",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           const std::string app = args;
           set_js_scripts(tx, lua::Interpreter().invoke<nlohmann::json>(app));
           return true;
         }},
        // add a new member
        {"new_member",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           const auto parsed = args.get<MemberPubInfo>();
           GenesisGenerator g(this->network, tx);
           auto new_member_id =
             g.add_member(parsed.cert, parsed.keyshare, MemberStatus::ACCEPTED);

           auto [ma_view, sig_view] =
             tx.get_view(this->network.member_acks, this->network.signatures);

           ma_view->put(new_member_id, MemberAck(sig_view->get(0)->root));

           return true;
         }},
        // add a new user
        {"new_user",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           const Cert pem_cert = args;

           GenesisGenerator g(this->network, tx);
           g.add_user(pem_cert);

           return true;
         }},
        {"set_user_data",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           const auto parsed = args.get<SetUserData>();
           auto users_view = tx.get_view(this->network.users);
           auto user_info = users_view->get(parsed.user_id);
           if (!user_info.has_value())
           {
             throw std::logic_error(
               fmt::format("{} is not a valid user ID", parsed.user_id));
           }

           user_info->user_data = parsed.user_data;
           users_view->put(parsed.user_id, user_info.value());
           return true;
         }},
        // accept a node
        {"trust_node",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           const auto id = args.get<NodeId>();
           auto nodes = tx.get_view(this->network.nodes);
           auto node_info = nodes->get(id);
           if (!node_info.has_value())
           {
             throw std::logic_error(fmt::format("Node {} does not exist", id));
           }
           if (node_info->status == NodeStatus::RETIRED)
           {
             throw std::logic_error(
               fmt::format("Node {} is already retired", id));
           }
           node_info->status = NodeStatus::TRUSTED;
           nodes->put(id, node_info.value());
           LOG_INFO_FMT("Node {} is now {}", id, node_info->status);
           return true;
         }},
        // retire a node
        {"retire_node",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           const auto id = args.get<NodeId>();
           auto nodes = tx.get_view(this->network.nodes);
           auto node_info = nodes->get(id);
           if (!node_info.has_value())
           {
             throw std::logic_error(fmt::format("Node {} does not exist", id));
           }
           if (node_info->status == NodeStatus::RETIRED)
           {
             throw std::logic_error(
               fmt::format("Node {} is already retired", id));
           }
           node_info->status = NodeStatus::RETIRED;
           nodes->put(id, node_info.value());
           LOG_INFO_FMT("Node {} is now {}", id, node_info->status);
           return true;
         }},
        // accept new code
        {"new_code",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           const auto id = args.get<CodeDigest>();
           auto code_ids = tx.get_view(this->network.code_ids);
           auto existing_code_id = code_ids->get(id);
           if (existing_code_id)
           {
             throw std::logic_error(fmt::format(
               "Code signature already exists with digest: {:02x}",
               fmt::join(id, "")));
           }
           code_ids->put(id, CodeStatus::ACCEPTED);
           return true;
         }},
        {"accept_recovery",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           if (node.is_part_of_public_network())
           {
             return node.finish_recovery(tx, args);
           }
           else
           {
             return false;
           }
         }},
        {"open_network",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           return node.open_network(tx);
         }},
        {"rekey_ledger",
         [this](Store::Tx& tx, const nlohmann::json& args) {
           return node.rekey_ledger(tx);
         }},
      };

    bool complete_proposal(Store::Tx& tx, const ObjectId id)
    {
      auto proposals = tx.get_view(this->network.proposals);
      auto proposal = proposals->get(id);
      if (!proposal.has_value())
      {
        throw std::logic_error(fmt::format("No such proposal: {}", id));
      }

      if (proposal->state != ProposalState::OPEN)
      {
        throw std::logic_error(fmt::format(
          "Cannot complete non-open proposal - current state is {}",
          proposal->state));
      }

      // run proposal script
      const auto proposed_calls = tsr.run<nlohmann::json>(
        tx,
        {proposal->script,
         {}, // can't write
         WlIds::MEMBER_CAN_READ,
         get_script(tx, GovScriptIds::ENV_PROPOSAL)},
        // vvv arguments to script vvv
        proposal->parameter);

      nlohmann::json votes;
      // Collect all member votes
      for (const auto& vote : proposal->votes)
      {
        // valid voter
        if (!check_member_active(tx, vote.first))
        {
          continue;
        }

        // does the voter agree?
        votes[std::to_string(vote.first)] = tsr.run<bool>(
          tx,
          {vote.second,
           {}, // can't write
           WlIds::MEMBER_CAN_READ,
           {}},
          proposed_calls);
      }

      const auto pass = tsr.run<int>(
        tx,
        {get_script(tx, GovScriptIds::PASS),
         {}, // can't write
         WlIds::MEMBER_CAN_READ,
         {}},
        // vvv arguments to script vvv
        proposed_calls,
        votes);

      switch (pass)
      {
        case CompletionResult::PASSED:
        {
          // vote passed, go on to update the state
          break;
        }
        case CompletionResult::PENDING:
        {
          // vote is pending, return false but do not update state
          return false;
        }
        case CompletionResult::REJECTED:
        {
          // vote unsuccessful, update the proposal's state
          proposal->state = ProposalState::REJECTED;
          proposals->put(id, proposal.value());
          return false;
        }
        default:
        {
          throw std::logic_error(fmt::format(
            "Invalid completion result ({}) for proposal {}", pass, id));
        }
      };

      // execute proposed calls
      ProposedCalls pc = proposed_calls;
      for (const auto& call : pc)
      {
        // proposing a hardcoded C++ function?
        const auto f = hardcoded_funcs.find(call.func);
        if (f != hardcoded_funcs.end())
        {
          if (!f->second(tx, call.args))
          {
            return false;
          }
          continue;
        }

        // proposing a script function?
        const auto s = tx.get_view(network.gov_scripts)->get(call.func);
        if (!s.has_value())
        {
          continue;
        }
        tsr.run<void>(
          tx,
          {s.value(),
           WlIds::MEMBER_CAN_PROPOSE, // can write!
           {},
           {}},
          call.args);
      }

      // if the vote was successful, update the proposal's state
      proposal->state = ProposalState::ACCEPTED;
      proposals->put(id, proposal.value());

      return true;
    }

    bool check_member_active(Store::Tx& tx, MemberId id)
    {
      return check_member_status(tx, id, {MemberStatus::ACTIVE});
    }

    bool check_member_accepted(Store::Tx& tx, MemberId id)
    {
      return check_member_status(
        tx, id, {MemberStatus::ACTIVE, MemberStatus::ACCEPTED});
    }

    bool check_member_status(
      Store::Tx& tx, MemberId id, std::initializer_list<MemberStatus> allowed)
    {
      auto member = tx.get_view(this->network.members)->get(id);
      if (!member)
      {
        return false;
      }
      for (const auto s : allowed)
      {
        if (member->status == s)
        {
          return true;
        }
      }
      return false;
    }

    void record_voting_history(
      Store::Tx& tx, CallerId caller_id, const SignedReq& signed_request)
    {
      auto governance_history = tx.get_view(network.governance_history);
      governance_history->put(caller_id, {signed_request});
    }

    NetworkTables& network;
    AbstractNodeState& node;
    const lua::TxScriptRunner tsr;

    static constexpr auto SIZE_NONCE = 16;

  public:
    MemberHandlers(NetworkTables& network, AbstractNodeState& node) :
      CommonHandlerRegistry(*network.tables, Tables::MEMBER_CERTS),
      network(network),
      node(node),
      tsr(network)
    {}

    void init_handlers(Store& tables_) override
    {
      CommonHandlerRegistry::init_handlers(tables_);

      auto read = [this](RequestArgs& args) {
        if (!check_member_status(
              args.tx,
              args.caller_id,
              {MemberStatus::ACTIVE, MemberStatus::ACCEPTED}))
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INSUFFICIENT_RIGHTS);
          return;
        }

        const auto in = args.rpc_ctx->get_params().get<KVRead::In>();

        const ccf::Script read_script(R"xxx(
        local tables, table_name, key = ...
        return tables[table_name]:get(key) or {}
        )xxx");

        auto value = tsr.run<nlohmann::json>(
          args.tx,
          {read_script, {}, WlIds::MEMBER_CAN_READ, {}},
          in.table,
          in.key);
        if (value.empty())
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::StandardErrorCodes::INVALID_PARAMS,
            fmt::format(
              "Key {} does not exist in table {}", in.key.dump(), in.table));
          return;
        }

        args.rpc_ctx->set_response_result(std::move(value));
        return;
      };
      install_with_auto_schema<KVRead>(MemberProcs::READ, read, Read);

      auto query = [this](RequestArgs& args) {
        if (!check_member_accepted(args.tx, args.caller_id))
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INSUFFICIENT_RIGHTS);
          return;
        }

        const auto script = args.rpc_ctx->get_params().get<ccf::Script>();
        args.rpc_ctx->set_response_result(tsr.run<nlohmann::json>(
          args.tx, {script, {}, WlIds::MEMBER_CAN_READ, {}}));
        return;
      };
      install_with_auto_schema<Script, nlohmann::json>(
        MemberProcs::QUERY, query, Read);

      auto propose = [this](RequestArgs& args) {
        if (!check_member_active(args.tx, args.caller_id))
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INSUFFICIENT_RIGHTS);
          return;
        }

        const auto in = args.rpc_ctx->get_params().get<Propose::In>();
        const auto proposal_id = get_next_id(
          args.tx.get_view(this->network.values), ValueIds::NEXT_PROPOSAL_ID);
        Proposal proposal(in.script, in.parameter, args.caller_id);

        auto proposals = args.tx.get_view(this->network.proposals);
        proposal.votes[args.caller_id] = in.ballot;
        proposals->put(proposal_id, proposal);
        const bool completed = complete_proposal(args.tx, proposal_id);

        record_voting_history(
          args.tx, args.caller_id, args.rpc_ctx->get_signed_request().value());

        args.rpc_ctx->set_response_result(
          Propose::Out({proposal_id, completed}));
        return;
      };
      install_with_auto_schema<Propose>(
        MemberProcs::PROPOSE, propose, Write, true);

      auto withdraw = [this](RequestArgs& args) {
        if (!check_member_status(
              args.tx, args.caller_id, {MemberStatus::ACTIVE}))
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INSUFFICIENT_RIGHTS);
          return;
        }

        const auto proposal_action =
          args.rpc_ctx->get_params().get<ProposalAction>();
        const auto proposal_id = proposal_action.id;
        auto proposals = args.tx.get_view(this->network.proposals);
        auto proposal = proposals->get(proposal_id);

        if (!proposal)
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::StandardErrorCodes::INVALID_PARAMS,
            fmt::format("Proposal {} does not exist", proposal_id));
          return;
        }

        if (proposal->proposer != args.caller_id)
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INVALID_CALLER_ID,
            fmt::format(
              "Proposal {} can only be withdrawn by proposer {}, not caller {}",
              proposal_id,
              proposal->proposer,
              args.caller_id));
          return;
        }

        if (proposal->state != ProposalState::OPEN)
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::StandardErrorCodes::INVALID_PARAMS,
            fmt::format(
              "Proposal {} is currently in state {} - only {} proposals can be "
              "withdrawn",
              proposal_id,
              proposal->state,
              ProposalState::OPEN));
          return;
        }

        proposal->state = ProposalState::WITHDRAWN;
        proposals->put(proposal_id, proposal.value());
        record_voting_history(
          args.tx, args.caller_id, args.rpc_ctx->get_signed_request().value());

        args.rpc_ctx->set_response_result(true);
        return;
      };
      install_with_auto_schema<ProposalAction, bool>(
        MemberProcs::WITHDRAW, withdraw, Write, true);

      auto vote = [this](RequestArgs& args) {
        if (!check_member_active(args.tx, args.caller_id))
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INSUFFICIENT_RIGHTS);
          return;
        }

        const auto vote = args.rpc_ctx->get_params().get<Vote>();
        auto proposals = args.tx.get_view(this->network.proposals);
        auto proposal = proposals->get(vote.id);
        if (!proposal)
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::StandardErrorCodes::INVALID_PARAMS,
            fmt::format("Proposal {} does not exist", vote.id));
          return;
        }

        if (proposal->state != ProposalState::OPEN)
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::StandardErrorCodes::INVALID_PARAMS,
            fmt::format(
              "Proposal {} is currently in state {} - only {} proposals can "
              "receive votes",
              vote.id,
              proposal->state,
              ProposalState::OPEN));
          return;
        }

        proposal->votes[args.caller_id] = vote.ballot;
        proposals->put(vote.id, proposal.value());

        record_voting_history(
          args.tx, args.caller_id, args.rpc_ctx->get_signed_request().value());

        args.rpc_ctx->set_response_result(complete_proposal(args.tx, vote.id));
        return;
      };
      install_with_auto_schema<Vote, bool>(
        MemberProcs::VOTE, vote, Write, true);

      auto complete = [this](RequestArgs& args) {
        if (!check_member_active(args.tx, args.caller_id))
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INSUFFICIENT_RIGHTS);
          return;
        }

        const auto proposal_action =
          args.rpc_ctx->get_params().get<ProposalAction>();
        const auto proposal_id = proposal_action.id;

        record_voting_history(
          args.tx, args.caller_id, args.rpc_ctx->get_signed_request().value());

        args.rpc_ctx->set_response_result(
          complete_proposal(args.tx, proposal_id));
        return;
      };
      install_with_auto_schema<ProposalAction, bool>(
        MemberProcs::COMPLETE, complete, Write, true);

      //! A member acknowledges state
      auto ack = [this](RequestArgs& args) {
        const auto signed_request = args.rpc_ctx->get_signed_request();

        auto [ma_view, sig_view] =
          args.tx.get_view(this->network.member_acks, this->network.signatures);
        const auto ma = ma_view->get(args.caller_id);
        if (!ma)
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INVALID_CALLER_ID,
            fmt::format("No ACK record exists for caller {}", args.caller_id));
          return;
        }

        if (
          ma->state_digest !=
          args.rpc_ctx->get_params().get<StateDigest>().state_digest)
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::StandardErrorCodes::INVALID_PARAMS,
            "Submitted state digest is not valid");
          return;
        }

        ma_view->put(
          args.caller_id,
          MemberAck(sig_view->get(0)->root, signed_request.value()));

        auto members = args.tx.get_view(this->network.members);
        auto member = members->get(args.caller_id);
        if (member->status == MemberStatus::ACCEPTED)
        {
          member->status = MemberStatus::ACTIVE;
        }
        members->put(args.caller_id, member.value());
        args.rpc_ctx->set_response_result(true);
        return;
      };

      install_with_auto_schema<StateDigest, bool>(
        MemberProcs::ACK, ack, Write, true);

      //! A member asks for a fresher state digest
      auto update_state_digest = [this](RequestArgs& args) {
        auto [ma_view, sig_view] =
          args.tx.get_view(this->network.member_acks, this->network.signatures);
        auto ma = ma_view->get(args.caller_id);
        if (!ma)
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INVALID_CALLER_ID,
            fmt::format("No ACK record exists for caller {}", args.caller_id));
          return;
        }

        auto root = sig_view->get(0)->root;
        ma->state_digest = std::vector<uint8_t>(root.h.begin(), root.h.end());
        ma_view->put(args.caller_id, ma.value());

        args.rpc_ctx->set_response_result(ma->state_digest);
        return;
      };
      install_with_auto_schema<void, StateDigest>(
        MemberProcs::UPDATE_ACK_STATE_DIGEST, update_state_digest, Write);

      auto get_encrypted_recovery_share = [this](RequestArgs& args) {
        // This check should depend on whether new shares are emitted when a new
        // member is added (status = Accepted) or when the new member acks
        // (status = Active). For now, the member should just be accepted.
        if (!check_member_accepted(args.tx, args.caller_id))
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INSUFFICIENT_RIGHTS);
          return;
        }

        std::optional<EncryptedShare> enc_s;
        auto current_keyshare = args.tx.get_view(this->network.shares)->get(0);
        for (auto const& s : current_keyshare->encrypted_shares)
        {
          LOG_FAIL_FMT("We've got a share for member {}", s.first);
          if (s.first == args.caller_id)
          {
            LOG_FAIL_FMT("A share for me!");
            enc_s = s.second;
          }
        }

        if (!enc_s.has_value())
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::RECOVERY_SHARE_NOT_FOUND);
        }

        args.rpc_ctx->set_response_result(enc_s.value());
        return;
      };
      install_with_auto_schema<void, EncryptedShare>(
        MemberProcs::GET_ENCRYPTED_RECOVERY_SHARE,
        get_encrypted_recovery_share,
        Read);

      auto submit_recovery_share = [this](RequestArgs& args) {
        // Only active members can submit their shares for recovery
        if (!check_member_active(args.tx, args.caller_id))
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::CCFErrorCodes::INSUFFICIENT_RIGHTS);
          return;
        }

        // For now, we don't check if recovery has yet been approved. To be
        // resilient to elections, we should store that the service is waiting
        // for shares post recovery vote.

        throw std::logic_error("Not implemented");
      };
      install_with_auto_schema<std::vector<uint8_t>, bool>(
        MemberProcs::SUBMIT_RECOVERY_SHARE, submit_recovery_share, Write);

      auto create = [this](RequestArgs& args) {
        LOG_DEBUG_FMT("Processing create RPC");
        const auto in =
          args.rpc_ctx->get_params().get<CreateNetworkNodeToNode::In>();

        GenesisGenerator g(this->network, args.tx);

        // This endpoint can only be called once, directly from the starting
        // node for the genesis transaction to initialise the service
        if (g.is_service_created())
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::StandardErrorCodes::INTERNAL_ERROR,
            "Service is already created");
          return;
        }

        g.init_values();
        for (auto& [cert, k_encryption_key] : in.members_info)
        {
          g.add_member(cert, k_encryption_key);
        }

        node.split_ledger_secrets(args.tx);

        size_t self = g.add_node({in.node_info_network,
                                  in.node_cert,
                                  in.quote,
                                  in.public_encryption_key,
                                  NodeStatus::TRUSTED});

        LOG_INFO_FMT("Create node id: {}", self);
        if (self != 0)
        {
          args.rpc_ctx->set_response_error(
            jsonrpc::StandardErrorCodes::INTERNAL_ERROR,
            "Starting node ID is not 0");
          return;
        }

#ifdef GET_QUOTE
        CodeDigest node_code_id;
        std::copy_n(
          std::begin(in.code_digest),
          CODE_DIGEST_BYTES,
          std::begin(node_code_id));
        g.trust_code_id(node_code_id);
#endif

        for (const auto& wl : default_whitelists)
        {
          g.set_whitelist(wl.first, wl.second);
        }

        g.set_gov_scripts(
          lua::Interpreter().invoke<nlohmann::json>(in.gov_script));

        g.create_service(in.network_cert);

        args.rpc_ctx->set_response_result(true);
        LOG_INFO_FMT("Created service");
        return;
      };
      install(MemberProcs::CREATE, create, Write);
    }
  };

  class MemberRpcFrontend : public RpcFrontend
  {
  protected:
    std::string invalid_caller_error_message() const override
    {
      return "Could not find matching member certificate";
    }

    MemberHandlers member_handlers;
    Members* members;

  public:
    MemberRpcFrontend(NetworkTables& network, AbstractNodeState& node) :
      RpcFrontend(
        *network.tables, member_handlers, &network.member_client_signatures),
      member_handlers(network, node),
      members(&network.members)
    {}

    std::vector<uint8_t> get_cert_to_forward(
      std::shared_ptr<enclave::RpcContext> ctx) override
    {
      // Caller cert can be looked up on receiver - so don't forward it
      return {};
    }

    bool lookup_forwarded_caller_cert(
      std::shared_ptr<enclave::RpcContext> ctx, Store::Tx& tx) override
    {
      // Lookup the caller member's certificate from the forwarded caller id
      auto members_view = tx.get_view(*members);
      auto caller = members_view->get(ctx->session->fwd->caller_id);
      if (!caller.has_value())
      {
        return false;
      }

      ctx->session->caller_cert = caller.value().cert;
      return true;
    }
  };
} // namespace ccf
