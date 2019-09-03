// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#include "../logger.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <fstream>
#include <nlohmann/json.hpp>

TEST_CASE("Test custom log format")
{
  std::string test_log_file = "./test_json_logger.txt";
  remove(test_log_file.c_str());
  logger::config::loggers().emplace_back(
    std::make_unique<logger::JsonLogger>(test_log_file));
  logger::config::level() = logger::DBG;
  std::string log_msg_dbg = "log_msg_dbg";
  std::string log_msg_fail = "log_msg_fail";

  LOG_DEBUG_FMT(log_msg_dbg);
  LOG_TRACE_FMT(log_msg_fail);

  std::ifstream f(test_log_file);
  std::string line;
  size_t line_count = 0;
  while (std::getline(f, line))
  {
    line_count++;
    auto j = nlohmann::json::parse(line);
    auto host_ts = j.find("h_ts");
    REQUIRE(host_ts != j.end());
    REQUIRE(j["msg"] == log_msg_dbg + "\n");
    REQUIRE(j["file"] == __FILE__);
    auto line_number = j.find("number");
    REQUIRE(line_number != j.end());
    REQUIRE(j["level"] == "debug");
  }
  REQUIRE(line_count == 1);
}
