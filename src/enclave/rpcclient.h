// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "tlsframedendpoint.h"
#include "enclavetypes.h"

namespace enclave
{
  class RPCClient : public FramedTLSEndpoint
  {
    using HandleDataCallback =
      std::function<std::pair<bool, std::vector<uint8_t>>(
        const std::vector<uint8_t>& data)>;

  private:
    HandleDataCallback handle_data_cb;
    AbstractRPCResponder& rpcresponder;

    // Initiating RPC context in case the client requires sending the peer's
    // response directly back to the client
    RpcContext rpc_ctx;

  public:
    RPCClient(
      size_t session_id,
      ringbuffer::AbstractWriterFactory& writer_factory,
      std::unique_ptr<tls::Context> ctx,
      AbstractRPCResponder& rpcresponder_,
      RpcContext& rpc_ctx_) :
      FramedTLSEndpoint(session_id, writer_factory, move(ctx)),
      rpcresponder(rpcresponder_),
      rpc_ctx(rpc_ctx_)
    {}

    void connect(
      const std::string& hostname,
      const std::string& service,
      const HandleDataCallback f)
    {
      RINGBUFFER_WRITE_MESSAGE(
        tls::tls_connect, to_host, session_id, hostname, service);
      handle_data_cb = f;
    }

    bool handle_data(const std::vector<uint8_t>& data) override
    {
      auto res = handle_data_cb(data);
      if (res.first)
      {
        // Send reply to the initiating session
        LOG_FAIL << "Join network: returning reply to session "
                 << rpc_ctx.session_id << std::endl;
        rpcresponder.reply_async(rpc_ctx.session_id, res.second);
      }

      close();
      return true;
    }
  };
}
