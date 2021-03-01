// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.

#include "rsa_key_pair.h"

#include "openssl_wrappers.h"

namespace crypto
{
  using namespace OpenSSL;

  RSAKeyPair_OpenSSL::RSAKeyPair_OpenSSL(
    size_t public_key_size, size_t public_exponent)
  {
    RSA* rsa = NULL;
    BIGNUM* big_exp = NULL;
    OpenSSL::CHECKNULL(big_exp = BN_new());
    OpenSSL::CHECK1(BN_set_word(big_exp, public_exponent));
    OpenSSL::CHECKNULL(rsa = RSA_new());
    OpenSSL::CHECK1(RSA_generate_key_ex(rsa, public_key_size, big_exp, NULL));
    OpenSSL::CHECKNULL(key = EVP_PKEY_new());
    OpenSSL::CHECK1(EVP_PKEY_set1_RSA(key, rsa));
    BN_free(big_exp);
    RSA_free(rsa);
  }

  RSAKeyPair_OpenSSL::RSAKeyPair_OpenSSL(EVP_PKEY* k) :
    RSAPublicKey_OpenSSL(std::move(k))
  {}

  RSAKeyPair_OpenSSL::RSAKeyPair_OpenSSL(const Pem& pem, CBuffer pw)
  {
    Unique_BIO mem(pem.data(), -1);
    key = PEM_read_bio_PrivateKey(mem, NULL, NULL, (void*)pw.p);
    if (!key)
    {
      throw std::runtime_error("could not parse PEM");
    }
  }

  size_t RSAKeyPair_OpenSSL::key_size() const
  {
    return RSAPublicKey_OpenSSL::key_size();
  }

  std::vector<uint8_t> RSAKeyPair_OpenSSL::rsa_oaep_unwrap(
    const std::vector<uint8_t>& input,
    std::optional<std::vector<std::uint8_t>> label)
  {
    const unsigned char* label_ = NULL;
    size_t label_size = 0;
    if (label.has_value())
    {
      label_ = label->data();
      label_size = label->size();
    }

    Unique_EVP_PKEY_CTX ctx(key);
    OpenSSL::CHECK1(EVP_PKEY_decrypt_init(ctx));
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);
    EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256());
    EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256());

    if (label_)
    {
      unsigned char* openssl_label = (unsigned char*)OPENSSL_malloc(label_size);
      std::copy(label_, label_ + label_size, openssl_label);
      EVP_PKEY_CTX_set0_rsa_oaep_label(ctx, openssl_label, label_size);
    }
    else
    {
      EVP_PKEY_CTX_set0_rsa_oaep_label(ctx, NULL, 0);
    }

    size_t olen;
    OpenSSL::CHECK1(
      EVP_PKEY_decrypt(ctx, NULL, &olen, input.data(), input.size()));

    std::vector<uint8_t> output(olen);
    OpenSSL::CHECK1(
      EVP_PKEY_decrypt(ctx, output.data(), &olen, input.data(), input.size()));

    output.resize(olen);
    return output;
  }

  Pem RSAKeyPair_OpenSSL::public_key_pem() const
  {
    return PublicKey_OpenSSL::public_key_pem();
  }
}
