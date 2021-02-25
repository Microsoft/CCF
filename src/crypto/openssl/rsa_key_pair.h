// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "crypto/rsa_key_pair.h"

#include "rsa_public_key.h"

#include <optional>
#include <vector>

namespace crypto
{
  class RSAKeyPair_OpenSSL : public RSAPublicKey_OpenSSL, public RSAKeyPair
  {
  public:
    RSAKeyPair_OpenSSL(
      size_t public_key_size = default_public_key_size,
      size_t public_exponent = default_public_exponent);
    RSAKeyPair_OpenSSL(EVP_PKEY* k);
    RSAKeyPair_OpenSSL(const RSAKeyPair&) = delete;
    RSAKeyPair_OpenSSL(const Pem& pem, CBuffer pw = nullb);
    virtual ~RSAKeyPair_OpenSSL() = default;

    virtual std::vector<uint8_t> unwrap(
      const std::vector<uint8_t>& input,
      std::optional<std::string> label = std::nullopt);

    virtual Pem public_key_pem() const;
  };
}
