// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

enum ConsensusType
{
  Raft = 1,
  Pbft = 2
};

DECLARE_JSON_ENUM(
  ConsensusType,
  {{ConsensusType::Raft, "Raft"}, {ConsensusType::Pbft, "Pbft"}});