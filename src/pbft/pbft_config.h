// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#include "libbyz/libbyz.h"
#include "libbyz/pbft_assert.h"
#include "pbft/pbft_deps.h"

namespace pbft
{
  class AbstractPbftConfig
  {
  public:
    static char* service_mem;
    virtual ~AbstractPbftConfig() = default;
    virtual void set_service_mem(char* sm) = 0;
    virtual ExecCommand get_exec_command() = 0;
    virtual size_t message_size() = 0;
    virtual void fill_request(
      uint8_t* buffer,
      size_t total_req_size,
      const std::vector<uint8_t>& data,
      size_t jsonrpc_id,
      uint64_t actor) = 0;
  };

  char* AbstractPbftConfig::service_mem = 0;

  class PbftConfigCcf : public AbstractPbftConfig
  {
  public:
    PbftConfigCcf(std::shared_ptr<enclave::RpcMap> rpc_map_) : rpc_map(rpc_map_)
    {}

    ~PbftConfigCcf() = default;

    void set_service_mem(char* sm) override
    {
      service_mem = sm;
    }

    ExecCommand get_exec_command() override
    {
      return exec_command;
    }

    size_t message_size() override
    {
      return sizeof(ccf_req);
    }

    void fill_request(
      uint8_t* buffer,
      size_t total_req_size,
      const std::vector<uint8_t>& data,
      size_t jsonrpc_id,
      uint64_t actor) override
    {
      serialized::write(buffer, total_req_size, jsonrpc_id);
      serialized::write(buffer, total_req_size, actor);
      serialized::write(buffer, total_req_size, data.data(), data.size());
    }

  private:
    std::shared_ptr<enclave::RpcMap> rpc_map;

    struct ccf_req
    {
      size_t jsonrpc_id;
      ccf::ActorsType actor;

      uint8_t* get_data()
      {
        return (uint8_t*)((uintptr_t)this + sizeof(ccf_req));
      }

      size_t get_size(size_t total_size)
      {
        if (total_size < sizeof(ccf_req))
        {
          return 0;
        }
        return total_size - sizeof(ccf_req);
      }
    };

    ExecCommand exec_command = [this](
                                 Byz_req* inb,
                                 Byz_rep* outb,
                                 _Byz_buffer* non_det,
                                 int client,
                                 bool ro,
                                 Seqno total_requests_executed) {
      LOG_INFO << "<<<< START exec_command() >>>>" << std::endl;

      // TODO: Do the unpacking of the request the CCF way
      auto request = new (inb->contents) ccf_req;

      LOG_INFO_FMT("received request with jsonrpc id: {}", request->jsonrpc_id);
      LOG_INFO_FMT("received request with actor: {}", request->actor);

      auto handler = this->rpc_map->find(request->actor);
      if (!handler.has_value())
        throw std::logic_error("No frontend in pbft exec_command");

      auto user_frontend = handler.value();

      // TODO: Also pass the transaction object and rpc_ctx used earlier on to
      // verify the caller/signature
      auto rep = user_frontend->process_pbft(
        {request->get_data(),
         request->get_data() + request->get_size(inb->size)});

      LOG_INFO << "After process_pbft, size of reply " << rep.size()
               << std::endl;


      outb->size = rep.size();
      auto outb_ptr = (uint8_t*)outb->contents;
      size_t outb_size = (size_t)outb->size;

      serialized::write(outb_ptr, outb_size, rep.data(), rep.size());

      LOG_INFO << "<<<< END exec_command() >>>>" << std::endl;

      return 0;
    };
  };
}