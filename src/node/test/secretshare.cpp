// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "../secretshare.h"

#include <doctest/doctest.h>
#include <iomanip>

TEST_CASE("Simple test")
{
  size_t n = 5;
  size_t k = 3;
  ccf::SecretSharing::SecretToSplit data_to_split;

  INFO("Data to split must be have fixed length");
  {
    auto random =
      tls::create_entropy()->random(ccf::SecretSharing::SECRET_TO_SHARE_LENGTH);
    std::copy_n(
      random.begin(),
      ccf::SecretSharing::SECRET_TO_SHARE_LENGTH,
      data_to_split.begin());
  }

  INFO("Split and combine shares");
  {
    auto shares = ccf::SecretSharing::split(data_to_split, n, k);
    REQUIRE(shares.size() == n);

    for (auto const& s : shares)
    {
      for (auto const& c : s)
      {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)c;
      }
      std::cout << std::dec << std::endl;
    }

    auto restored = ccf::SecretSharing::combine(shares, k);
    REQUIRE(data_to_split == restored);

    std::cout << "Restored ";
    for (auto const& d : data_to_split)
    {
      std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)d;
    }
    std::cout << std::dec << std::endl;
  }
}

TEST_CASE("Edge cases")
{
  size_t n = 3;
  size_t k = 2;
  ccf::SecretSharing::SecretToSplit data_to_split;

  INFO("n = 0 and n too large");
  {
    REQUIRE_THROWS_AS(
      ccf::SecretSharing::split(data_to_split, 0, 2), std::logic_error);
    REQUIRE_THROWS_AS(
      ccf::SecretSharing::split(
        data_to_split, ccf::SecretSharing::MAX_NUMBER_SHARES + 1, k),
      std::logic_error);
  }

  INFO("k = 0 and k too large");
  {
    REQUIRE_THROWS_AS(
      ccf::SecretSharing::split(data_to_split, n, 0), std::logic_error);
    REQUIRE_THROWS_AS(
      ccf::SecretSharing::split(data_to_split, n, n + 1), std::logic_error);
  }
}