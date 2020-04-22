// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "enclave/tls_endpoint.h"
#include "http_proc.h"

#include <algorithm>
#include <cctype>
#include <endian.h>
#include <map>
#include <queue>
#include <string>

namespace ws
{
  enum ParserState
  {
    INIT,
    READ_SLEN,
    READ_LLEN,
    READ_BODY
  };

  class Parser
  {
  public:
    virtual size_t consume(std::vector<uint8_t>&) = 0;
  };

  class ResponseParser : public Parser
  {
  private:
    http::ResponseProcessor& proc;

  public:
    ResponseParser(http::ResponseProcessor& proc_) : proc(proc_) {}

    size_t consume(std::vector<uint8_t>& data) override
    {
      throw std::logic_error("Not implemented");
    }
  };

  class RequestParser : public Parser
  {
  private:
    http::RequestProcessor& proc;
    uint64_t size = 0;
    ParserState state = INIT;

  public:
    RequestParser(http::RequestProcessor& proc_) : proc(proc_) {}

    size_t consume(std::vector<uint8_t>& data) override
    {
      switch (state)
      {
        case INIT:
        {
          assert(data.size() == 2);

          bool fin = data[0] & 0x80;
          if (!fin)
          {
            LOG_FAIL_FMT("Fragment messages aren't supported.");
            return 0;
          }
          else
          {
            if (data[0] == 0x82)
            {
              if (data[1] & 0x80)
              {
                LOG_FAIL_FMT("Masked messages aren't supported.");
                return 0;
              }
              size = data[1] & 0x7f;
              switch (size)
              {
                case 0x7f:
                {
                  state = READ_LLEN;
                  return 8;
                }
                case 0x7e:
                {
                  state = READ_SLEN;
                  return 2;
                }
                default:
                {
                  state = READ_BODY;
                  return size;
                }
              }
            }
            else
            {
              LOG_FAIL_FMT("Only binary messages are supported.");
              return 0;
            }
          }
        }
        case READ_SLEN:
        {
          assert(data.size() == 2);

          size = be16toh(*(uint16_t*)data.data());
          state = READ_BODY;
          return size;
        }
        case READ_LLEN:
        {
          assert(data.size() == 8);

          size = be64toh(*(uint64_t*)data.data());
          state = READ_BODY;
          return size;
        }
        case READ_BODY:
        {
          assert(data.size() == size);
          proc.handle_request(
            http_method::HTTP_POST,
            "/users/LOG_record",
            {},
            {{"Content-type", "application/json"}},
            std::move(data));
          state = INIT;
          return 2;
        }
        default:
        {
          throw std::logic_error("Unknown state");
        }
      }
    }
  };
}
