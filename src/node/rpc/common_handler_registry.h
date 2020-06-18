// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "consts.h"
#include "handler_registry.h"
#include "json_handler.h"
#include "metrics.h"

namespace ccf
{
  /*
   * Extends the basic HandlerRegistry with methods which should be present
   * on all frontends
   */
  class CommonHandlerRegistry : public HandlerRegistry
  {
  private:
    metrics::Metrics metrics;

    Nodes* nodes = nullptr;

  protected:
    kv::Store* tables = nullptr;

  public:
    CommonHandlerRegistry(
      kv::Store& store, const std::string& certs_table_name = "") :
      HandlerRegistry(store, certs_table_name),
      nodes(store.get<Nodes>(Tables::NODES)),
      tables(&store)
    {}

    void init_handlers(kv::Store& t) override
    {
      HandlerRegistry::init_handlers(t);

      auto get_commit = [this](kv::Tx& tx, nlohmann::json&& params) {
        if (consensus != nullptr)
        {
          auto [view, seqno] = consensus->get_committed_txid();
          return make_success(GetCommit::Out{view, seqno});
        }

        return make_error(
          HTTP_STATUS_INTERNAL_SERVER_ERROR,
          "Failed to get commit info from Consensus");
      };

      auto get_tx_status = [this](kv::Tx& tx, nlohmann::json&& params) {
        const auto in = params.get<GetTxStatus::In>();

        if (consensus != nullptr)
        {
          const auto tx_view = consensus->get_view(in.seqno);
          const auto committed_seqno = consensus->get_committed_seqno();
          const auto committed_view = consensus->get_view(committed_seqno);

          GetTxStatus::Out out;
          out.status = ccf::get_tx_status(
            in.view, in.seqno, tx_view, committed_view, committed_seqno);
          return make_success(out);
        }

        return make_error(
          HTTP_STATUS_INTERNAL_SERVER_ERROR, "Consensus is not yet configured");
      };

      auto get_metrics = [this](kv::Tx& tx, nlohmann::json&& params) {
        auto result = metrics.get_metrics();
        return make_success(result);
      };

      auto make_signature = [this](kv::Tx& tx, nlohmann::json&& params) {
        if (consensus != nullptr)
        {
          if (consensus->type() == ConsensusType::RAFT)
          {
            if (history != nullptr)
            {
              history->emit_signature();
              return make_success(true);
            }
          }
          else if (consensus->type() == ConsensusType::PBFT)
          {
            consensus->emit_signature();
            return make_success(true);
          }
        }

        return make_error(
          HTTP_STATUS_INTERNAL_SERVER_ERROR, "Failed to trigger signature");
      };

      auto who =
        [this](kv::Tx& tx, CallerId caller_id, nlohmann::json&& params) {
          if (certs == nullptr)
          {
            return make_error(
              HTTP_STATUS_NOT_FOUND,
              fmt::format(
                "This frontend does not support {}", GeneralProcs::WHO));
          }

          if (!params.is_null())
          {
            const WhoIs::In in = params;
            auto certs_view = tx.get_view(*certs);
            auto caller_id_opt = certs_view->get(in.cert);

            if (!caller_id_opt.has_value())
            {
              return make_error(
                HTTP_STATUS_BAD_REQUEST, "Certificate not recognised");
            }

            caller_id = caller_id_opt.value();
          }

          return make_success(WhoAmI::Out{caller_id});
        };

      auto get_primary_info = [this](kv::Tx& tx, nlohmann::json&& params) {
        if ((nodes != nullptr) && (consensus != nullptr))
        {
          NodeId primary_id = consensus->primary();
          auto current_view = consensus->get_view();

          auto nodes_view = tx.get_view(*nodes);
          auto info = nodes_view->get(primary_id);

          if (info)
          {
            GetPrimaryInfo::Out out;
            out.primary_id = primary_id;
            out.primary_host = info->pubhost;
            out.primary_port = info->rpcport;
            out.current_view = current_view;
            return make_success(out);
          }
        }

        return make_error(
          HTTP_STATUS_INTERNAL_SERVER_ERROR, "Primary unknown.");
      };

      auto get_network_info = [this](kv::Tx& tx, nlohmann::json&& params) {
        GetNetworkInfo::Out out;
        if (consensus != nullptr)
        {
          out.primary_id = consensus->primary();
        }

        auto nodes_view = tx.get_view(*nodes);
        nodes_view->foreach([&out](const NodeId& nid, const NodeInfo& ni) {
          if (ni.status == ccf::NodeStatus::TRUSTED)
          {
            out.nodes.push_back({nid, ni.pubhost, ni.rpcport});
          }
          return true;
        });

        return make_success(out);
      };

      auto get_nodes_by_rpc_address = [this](kv::Tx& tx, nlohmann::json&& params) {
        const auto in = params.get<GetNodesByRPCAddress::In>();

        GetNodesByRPCAddress::Out out;
        auto nodes_view = tx.get_view(*nodes);
        nodes_view->foreach([&in, &out](const NodeId& nid, const NodeInfo& ni) {
          if (ni.rpchost == in.host && ni.rpcport == in.port)
          {
            if (ni.status != ccf::NodeStatus::RETIRED || in.retired)
            {
              out.nodes.push_back({nid, ni.status});
            }
          }
          return true;
        });

        return make_success(out);
      };

      auto list_methods_fn = [this](kv::Tx& tx, nlohmann::json&& params) {
        ListMethods::Out out;

        list_methods(tx, out);

        std::sort(out.methods.begin(), out.methods.end());

        return make_success(out);
      };

      auto get_schema = [this](kv::Tx& tx, nlohmann::json&& params) {
        const auto in = params.get<GetSchema::In>();

        const auto it = handlers.find(in.method);
        if (it == handlers.end())
        {
          return make_error(
            HTTP_STATUS_BAD_REQUEST,
            fmt::format("Method {} not recognised", in.method));
        }

        const GetSchema::Out out{it->second.params_schema,
                                 it->second.result_schema};

        return make_success(out);
      };

      auto get_receipt = [this](kv::Tx& tx, nlohmann::json&& params) {
        const auto in = params.get<GetReceipt::In>();

        if (history != nullptr)
        {
          try
          {
            auto p = history->get_receipt(in.commit);
            const GetReceipt::Out out{p};

            return make_success(out);
          }
          catch (const std::exception& e)
          {
            return make_error(
              HTTP_STATUS_INTERNAL_SERVER_ERROR,
              fmt::format(
                "Unable to produce receipt for commit {} : {}",
                in.commit,
                e.what()));
          }
        }

        return make_error(
          HTTP_STATUS_INTERNAL_SERVER_ERROR, "Unable to produce receipt");
      };

      auto verify_receipt = [this](kv::Tx& tx, nlohmann::json&& params) {
        const auto in = params.get<VerifyReceipt::In>();

        if (history != nullptr)
        {
          try
          {
            bool v = history->verify_receipt(in.receipt);
            const VerifyReceipt::Out out{v};

            return make_success(out);
          }
          catch (const std::exception& e)
          {
            return make_error(
              HTTP_STATUS_INTERNAL_SERVER_ERROR,
              fmt::format("Unable to verify receipt: {}", e.what()));
          }
        }

        return make_error(
          HTTP_STATUS_INTERNAL_SERVER_ERROR, "Unable to verify receipt");
      };

      install(GeneralProcs::GET_COMMIT, json_adapter(get_commit), Read)
        .set_execute_locally(true)
        .set_auto_schema<void, GetCommit::Out>();
      install(GeneralProcs::GET_TX_STATUS, json_adapter(get_tx_status), Read)
        .set_auto_schema<GetTxStatus>()
        .set_http_get_only();
      install(GeneralProcs::GET_METRICS, json_adapter(get_metrics), Read)
        .set_auto_schema<void, GetMetrics::Out>()
        .set_execute_locally(true)
        .set_http_get_only();
      install(GeneralProcs::MK_SIGN, json_adapter(make_signature), Write)
        .set_auto_schema<void, bool>();
      install(GeneralProcs::WHO, json_adapter(who), Read)
        .set_auto_schema<WhoIs::In, WhoAmI::Out>()
        .set_http_get_only();
      install(
        GeneralProcs::GET_PRIMARY_INFO, json_adapter(get_primary_info), Read)
        .set_auto_schema<void, GetPrimaryInfo::Out>()
        .set_http_get_only();
      install(
        GeneralProcs::GET_NETWORK_INFO, json_adapter(get_network_info), Read)
        .set_auto_schema<void, GetNetworkInfo::Out>()
        .set_http_get_only();
      install(
        GeneralProcs::GET_NODES_BY_RPC_ADDRESS, json_adapter(get_nodes_by_rpc_address), Read)
        .set_auto_schema<GetNodesByRPCAddress::In, GetNodesByRPCAddress::Out>()
        .set_http_get_only();
      install(
        GeneralProcs::API_LIST_METHODS, json_adapter(list_methods_fn), Read)
        .set_auto_schema<void, ListMethods::Out>()
        .set_http_get_only();
      install(GeneralProcs::API_GET_SCHEMA, json_adapter(get_schema), Read)
        .set_auto_schema<GetSchema>()
        .set_http_get_only();
      install(GeneralProcs::GET_RECEIPT, json_adapter(get_receipt), Read)
        .set_auto_schema<GetReceipt>()
        .set_http_get_only();
      install(GeneralProcs::VERIFY_RECEIPT, json_adapter(verify_receipt), Read)
        .set_auto_schema<VerifyReceipt>();
    }

    void tick(
      std::chrono::milliseconds elapsed,
      kv::Consensus::Statistics stats) override
    {
      metrics.track_tx_rates(elapsed, stats);

      HandlerRegistry::tick(elapsed, stats);
    }
  };
}