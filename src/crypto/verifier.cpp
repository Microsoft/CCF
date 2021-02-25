// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.

#include "verifier.h"

#include "crypto/mbedtls/verifier.h"
#include "crypto/openssl/verifier.h"

namespace crypto
{
  using VerifierPtr = std::shared_ptr<Verifier>;
  using VerifierUniquePtr = std::unique_ptr<Verifier>;

  /**
   * Construct Verifier from a certificate in DER or PEM format
   *
   * @param cert Sequence of bytes containing the certificate
   */
  VerifierUniquePtr make_unique_verifier(const std::vector<uint8_t>& cert)
  {
#ifdef CRYPTO_PROVIDER_IS_MBEDTLS
    return std::make_unique<Verifier_mbedTLS>(cert);
#else
    return std::make_unique<Verifier_OpenSSL>(cert);
#endif
  }

  VerifierPtr make_verifier(const std::vector<uint8_t>& cert)
  {
#ifdef CRYPTO_PROVIDER_IS_MBEDTLS
    return std::make_shared<Verifier_mbedTLS>(cert);
#else
    return std::make_shared<Verifier_OpenSSL>(cert);
#endif
  }

  VerifierUniquePtr make_unique_verifier(const Pem& pem)
  {
    return make_unique_verifier(pem.raw());
  }

  VerifierPtr make_verifier(const Pem& pem)
  {
    return make_verifier(pem.raw());
  }

  crypto::Pem cert_der_to_pem(const std::vector<uint8_t>& der)
  {
    return make_verifier(der)->cert_pem();
  }

  std::vector<uint8_t> cert_pem_to_der(const std::string& pem_string)
  {
    return make_verifier(Pem(pem_string).raw())->cert_der();
  }

  Pem public_key_pem_from_cert(const Pem& cert)
  {
    return make_unique_verifier(cert)->public_key_pem();
  }

  void check_is_cert(const CBuffer& der)
  {
    make_unique_verifier((std::vector<uint8_t>)der); // throws on error
  }
}
