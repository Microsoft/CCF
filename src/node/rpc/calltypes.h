// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once
#include <nlohmann/json.hpp>

namespace ccf
{
  struct GetSchema
  {
    struct In
    {
      std::string method = {};
    };

    struct Out
    {
      nlohmann::json params_schema = {};
      nlohmann::json result_schema = {};
    };
  };

  struct GetCommit
  {
    struct Out
    {
      uint64_t term;
      int64_t commit;
    };
  };

  struct GetTxHist
  {
    struct Out
    {
      int low = {};
      int high = {};
      size_t overflow = {};
      size_t underflow = {};
      nlohmann::json histogram = {};
    };
  };
}