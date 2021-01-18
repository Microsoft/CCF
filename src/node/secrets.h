// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once
#include "entities.h"
#include "kv/map.h"

#include <msgpack/msgpack.hpp>
#include <vector>

namespace ccf
{
  struct EncryptedLedgerSecret
  {
    // Version at which the ledger secret is applicable from (recovery
    // only)
    std::optional<kv::Version> version;

    // Encrypted secret for each backup
    std::vector<uint8_t> encrypted_secret = {};

    MSGPACK_DEFINE(version, encrypted_secret);
  };

  DECLARE_JSON_TYPE(EncryptedLedgerSecret)
  DECLARE_JSON_REQUIRED_FIELDS(EncryptedLedgerSecret, version, encrypted_secret)

  struct EncryptedLedgerSecrets
  {
    std::vector<uint8_t> primary_public_encryption_key = {};

    std::vector<EncryptedLedgerSecret> encrypted_secrets = {};

    MSGPACK_DEFINE(primary_public_encryption_key, encrypted_secrets);
  };

  DECLARE_JSON_TYPE(EncryptedLedgerSecrets)
  DECLARE_JSON_REQUIRED_FIELDS(
    EncryptedLedgerSecrets, primary_public_encryption_key, encrypted_secrets)

  // This map is used to communicate encrypted ledger secrets from the primary
  // to the backups during recovery (past secrets) and re-keying (new secret)
  using Secrets = kv::Map<NodeId, EncryptedLedgerSecrets>;
}
