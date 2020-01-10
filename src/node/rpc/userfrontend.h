// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "frontend.h"
#include "node/clientsignatures.h"

namespace ccf
{
  class UserRpcFrontend : public RpcFrontend, public CommonHandlerRegistry
  {
  protected:
    std::string invalid_caller_error_message() const override
    {
      return "Could not find matching user certificate";
    }

    Users* users;

  public:
    UserRpcFrontend(Store& tables_) :
      RpcFrontend(
        tables_,
        *this,
        tables_.get<ClientSignatures>(Tables::USER_CLIENT_SIGNATURES)),
      CommonHandlerRegistry(Tables::USER_CERTS),
      users(tables_.get<Users>(Tables::USERS))
    {}

    std::vector<uint8_t> get_cert_to_forward(
      const enclave::RpcContext& ctx) override
    {
      // Caller cert can be looked up on receiver - so don't forward it
      return {};
    }

    bool lookup_forwarded_caller_cert(
      enclave::RpcContext& ctx, Store::Tx& tx) override
    {
      // Lookup the calling user's certificate from the forwarded caller id
      auto users_view = tx.get_view(*users);
      auto caller = users_view->get(ctx.session.fwd->caller_id);
      if (!caller.has_value())
      {
        return false;
      }

      ctx.session.caller_cert = caller.value().cert;
      return true;
    }
  };
}
