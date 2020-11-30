// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "frontend.h"
#include "node/client_signatures.h"
#include "node/network_tables.h"

namespace ccf
{
  /** The CCF application must be an instance of UserRpcFrontend
   */
  class UserRpcFrontend : public RpcFrontend
  {
  protected:
    std::string invalid_caller_error_message() const override
    {
      return "Could not find matching user certificate";
    }

  public:
    UserRpcFrontend(kv::Store& tables, EndpointRegistry& h) :
      RpcFrontend(tables, h, Tables::USER_CLIENT_SIGNATURES)
    {}

    void open() override
    {
      RpcFrontend::open();
      endpoints.openapi_info.title = "CCF Application API";
    }

    // Forward these methods so that apps can write foo(...); rather than
    // endpoints.foo(...);
    template <typename... Ts>
    ccf::EndpointRegistry::Endpoint& install(Ts&&... ts)
    {
      return endpoints.install(std::forward<Ts>(ts)...);
    }

    template <typename... Ts>
    ccf::EndpointRegistry::Endpoint make_endpoint(Ts&&... ts)
    {
      return endpoints.make_endpoint(std::forward<Ts>(ts)...);
    }

    template <typename... Ts>
    ccf::EndpointRegistry::Endpoint make_read_only_endpoint(Ts&&... ts)
    {
      return endpoints.make_read_only_endpoint(std::forward<Ts>(ts)...);
    }

    template <typename... Ts>
    ccf::EndpointRegistry::Endpoint make_command_endpoint(Ts&&... ts)
    {
      return endpoints.make_command_endpoint(std::forward<Ts>(ts)...);
    }
  };

  class UserEndpointRegistry : public CommonEndpointRegistry
  {
  public:
    UserEndpointRegistry(kv::Store& store) :
      CommonEndpointRegistry(
        get_actor_prefix(ActorsType::users),
        store,
        Tables::USER_CERT_DERS,
        Tables::USER_DIGESTS)
    {}

    UserEndpointRegistry(NetworkTables& network) :
      CommonEndpointRegistry(
        get_actor_prefix(ActorsType::users),
        *network.tables,
        Tables::USER_CERT_DERS,
        Tables::USER_DIGESTS)
    {}

    std::shared_ptr<AuthnPolicy> get_cert_authn_policy() override
    {
      return user_cert_auth_policy;
    }

    std::shared_ptr<AuthnPolicy> get_sig_authn_policy() override
    {
      return user_signature_auth_policy;
    }
  };

  class SimpleUserRpcFrontend : public UserRpcFrontend
  {
  protected:
    UserEndpointRegistry common_handlers;

  public:
    SimpleUserRpcFrontend(kv::Store& tables) :
      UserRpcFrontend(tables, common_handlers),
      common_handlers(tables)
    {}
  };
}
