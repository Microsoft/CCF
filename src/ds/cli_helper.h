// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "ds/logger.h"
#include "ds/nonstd.h"
#include "tls/san.h"

#include <CLI11/CLI11.hpp>
#include <optional>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

namespace cli
{
  struct ParsedAddress
  {
    std::string hostname = {};
    std::string port = {};
  };

  CLI::Option* add_address_option(
    CLI::App& app,
    ParsedAddress& parsed,
    const std::string& option_name,
    const std::string& option_desc,
    const std::string& default_port = "0")
  {
    CLI::callback_t fun = [&parsed, option_name, default_port](
                            CLI::results_t results) {
      if (results.size() != 1)
      {
        throw CLI::ValidationError(option_name, "Address could not be parsed");
      }

      auto addr = results[0];
      auto found = addr.find_last_of(":");
      auto hostname = addr.substr(0, found);

      const auto port =
        found == std::string::npos ? default_port : addr.substr(found + 1);

      // Check if port is in valid range
      int port_int;
      try
      {
        port_int = std::stoi(port);
      }
      catch (const std::exception&)
      {
        throw CLI::ValidationError(option_name, "Port is not a number");
      }
      if (port_int < 0 || port_int > 65535)
      {
        throw CLI::ValidationError(
          option_name, "Port number is not in range 0-65535");
      }

      parsed.hostname = hostname;
      parsed.port = port;

      return true;
    };

    auto* option = app.add_option(option_name, fun, option_desc, true);
    option->type_name("HOST:PORT");

    return option;
  }

  struct ParsedMemberInfo
  {
    std::string cert_file;
    std::optional<std::string> enc_pubk_file;
    std::optional<std::string> member_data_file;
  };

  CLI::Option* add_member_info_option(
    CLI::App& app,
    std::vector<ParsedMemberInfo>& parsed,
    const std::string& option_name,
    const std::string& option_desc)
  {
    CLI::callback_t fun = [&option_name, &parsed](CLI::results_t res) {
      parsed.clear();
      for (const auto& r : res)
      {
        std::stringstream ss(r);
        std::string chunk;
        std::vector<std::string> chunks;

        while (std::getline(ss, chunk, ','))
        {
          chunks.emplace_back(chunk);
        }

        if (chunks.empty() || chunks.size() > 3)
        {
          throw CLI::ValidationError(
            option_name,
            "Member info is not in expected format: "
            "member_cert.pem[,member_enc_pubk.pem[,member_data.json]]");
        }

        ParsedMemberInfo member_info;
        member_info.cert_file = chunks.at(0);
        if (chunks.size() == 2)
        {
          member_info.enc_pubk_file = chunks.at(1);
        }
        else if (chunks.size() == 3)
        {
          // Only read encryption public key if there is something between two
          // commas
          if (!chunks.at(1).empty())
          {
            member_info.enc_pubk_file = chunks.at(1);
          }
          member_info.member_data_file = chunks.at(2);
        }

        // Validate that member info files exist, when specified
        auto validator = CLI::detail::ExistingFileValidator();
        auto err_str = validator(member_info.cert_file);
        if (!err_str.empty())
        {
          throw CLI::ValidationError(option_name, err_str);
        }

        if (member_info.enc_pubk_file.has_value())
        {
          err_str = validator(member_info.enc_pubk_file.value());
          if (!err_str.empty())
          {
            throw CLI::ValidationError(option_name, err_str);
          }
        }

        if (member_info.member_data_file.has_value())
        {
          err_str = validator(member_info.member_data_file.value());
          if (!err_str.empty())
          {
            throw CLI::ValidationError(option_name, err_str);
          }
        }

        parsed.emplace_back(member_info);
      }
      return true;
    };

    auto* option = app.add_option(option_name, fun, option_desc, true);
    option
      ->type_name("member_cert.pem[,member_enc_pubk.pem[,member_data.json]]")
      ->type_size(-1);

    return option;
  }

  static const std::string IP_ADDRESS_PREFIX("iPAddress:");
  static const std::string DNS_NAME_PREFIX("dNSName:");

  CLI::Option* add_subject_alternative_name_option(
    CLI::App& app,
    std::vector<tls::SubjectAltName>& parsed,
    const std::string& option_name,
    const std::string& option_desc)
  {
    CLI::callback_t fun = [&parsed, option_name](CLI::results_t results) {
      for (auto& result : results)
      {
        if (nonstd::starts_with(result, IP_ADDRESS_PREFIX))
        {
          parsed.push_back({result.substr(IP_ADDRESS_PREFIX.size()), true});
        }
        else if (nonstd::starts_with(result, DNS_NAME_PREFIX))
        {
          parsed.push_back({result.substr(DNS_NAME_PREFIX.size()), false});
        }
        else
        {
          throw CLI::ValidationError(
            option_name,
            fmt::format(
              "SAN could not be parsed: {}, must be (iPAddress|dNSName):VALUE",
              result));
        }
      }

      return true;
    };

    auto* option = app.add_option(option_name, fun, option_desc, true);
    option->type_name("(iPAddress|dNSName):VALUE")->type_size(-1);

    return option;
  }
}