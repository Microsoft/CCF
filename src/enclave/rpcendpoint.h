// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "http.h"
#include "node/rpc/jsonrpc.h"
#include "rpcmap.h"
#include "tlsframedendpoint.h"

namespace enclave
{
#ifdef HTTP
  using ServerEndpoint = HTTPEndpoint<http::ResponseHeaderEmitter>;
#else
  using ServerEndpoint = FramedTLSEndpoint;
#endif

  class RPCEndpoint : public ServerEndpoint
  {
  private:
    std::shared_ptr<RPCMap> rpc_map;
    std::shared_ptr<RpcHandler> handler;
    ccf::ActorsType actor;
    size_t session_id;
    CBuffer caller;

  public:
    RPCEndpoint(
      std::shared_ptr<RPCMap> rpc_map_,
      size_t session_id,
      ringbuffer::AbstractWriterFactory& writer_factory,
      std::unique_ptr<tls::Context> ctx) :
      ServerEndpoint(session_id, writer_factory, move(ctx)),
      rpc_map(rpc_map_),
      session_id(session_id)
    {}

    std::optional<std::string> get_method(const nlohmann::json& j)
    {
      if (j.find(jsonrpc::SIG) != j.end())
      {
        return j.at(jsonrpc::REQ).at(jsonrpc::METHOD).get<std::string>();
      }
      else
      {
        return j.at(jsonrpc::METHOD).get<std::string>();
      }
    }

    std::string get_actor(const std::string& method)
    {
      return method.substr(0, method.find_last_of('/'));
    }

    bool handle_data(const std::vector<uint8_t>& data)
    {
      LOG_TRACE_FMT("Entered handle_data {} {}", data.size(), data.empty());

      std::optional<jsonrpc::Pack> pack;
      auto [success, rpc] = jsonrpc::unpack_rpc(data, pack);

      if (!success)
      {
        send(jsonrpc::pack(rpc, pack.value()));
        return true;
      }
      LOG_TRACE_FMT("Deserialised");

      auto method = get_method(rpc);
      if (!method.has_value())
      {
        send(jsonrpc::pack(
          jsonrpc::error_response(
            rpc.value(jsonrpc::ID, 0),
            jsonrpc::StandardErrorCodes::METHOD_NOT_FOUND,
            "No method specified"),
          pack.value()));
        return true;
      }
      LOG_TRACE_FMT("Got method");

      std::string actor_prefix = get_actor(method.value());

      auto actor = rpc_map->resolve(actor_prefix);
      if (actor == ccf::ActorsType::unknown)
      {
        send(jsonrpc::pack(
          jsonrpc::error_response(
            rpc.value(jsonrpc::ID, 0),
            jsonrpc::StandardErrorCodes::METHOD_NOT_FOUND,
            fmt::format("No such prefix: {}", actor_prefix)),
          pack.value()));
        return true;
      }

      auto search = rpc_map->find(actor);
      if (!search.has_value())
        return false;

      RPCContext rpc_ctx(session_id, peer_cert(), actor);
      rpc_ctx.pack = pack;
      auto rep = search.value()->process(rpc_ctx, rpc, data);

      if (rpc_ctx.is_pending)
      {
        // If the RPC has been forwarded, hold the connection.
        return true;
      }
      else
      {
        // Otherwise, reply to the client synchronously.
        send(rep);
      }

      return true;
    }
  };
}
