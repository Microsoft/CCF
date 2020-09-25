// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
/* Definition of the call-in and call-out interfaces
 */
#pragma once

#include "consensus/consensus_types.h"
#include "consensus_type.h"
#include "ds/buffer.h"
#include "ds/logger.h"
#include "ds/oversized.h"
#include "ds/ring_buffer_types.h"
#include "kv/kv_types.h"
#include "node/members.h"
#include "node/node_info_network.h"
#include "start_type.h"
#include "tls/san.h"
#include "tls/tls.h"

#include <chrono>

struct EnclaveConfig
{
  ringbuffer::Circuit* circuit = nullptr;
  oversized::WriterConfig writer_config = {};

#ifdef DEBUG_CONFIG
  struct DebugConfig
  {
    size_t memory_reserve_startup;
  };
  DebugConfig debug_config = {};
#endif
};

struct CCFConfig
{
  consensus::Config consensus_config = {};
  ccf::NodeInfoNetwork node_info_network = {};
  std::string domain;
  size_t snapshot_tx_interval;

  struct SignatureIntervals
  {
    size_t sig_tx_interval;
    size_t sig_ms_interval;
    MSGPACK_DEFINE(sig_tx_interval, sig_ms_interval);
  };
  SignatureIntervals signature_intervals = {};

  struct Genesis
  {
    std::vector<ccf::MemberPubInfo> members_info;
    std::string gov_script;
    size_t recovery_threshold;
    MSGPACK_DEFINE(members_info, gov_script, recovery_threshold);
  };
  Genesis genesis = {};

  struct Joining
  {
    std::string target_host;
    std::string target_port;
    std::vector<uint8_t> network_cert;
    size_t join_timer;
    std::vector<uint8_t> snapshot;
    MSGPACK_DEFINE(
      target_host, target_port, network_cert, join_timer, snapshot);
  };
  Joining joining = {};

  std::string subject_name;
  std::vector<tls::SubjectAltName> subject_alternative_names;

  MSGPACK_DEFINE(
    consensus_config,
    node_info_network,
    domain,
    snapshot_tx_interval,
    signature_intervals,
    genesis,
    joining,
    subject_name,
    subject_alternative_names);
};

/// General administrative messages
enum AdminMessage : ringbuffer::Message
{
  /// Log message. Enclave -> Host
  DEFINE_RINGBUFFER_MSG_TYPE(log_msg),

  /// Fatal error message. Enclave -> Host
  DEFINE_RINGBUFFER_MSG_TYPE(fatal_error_msg),

  /// Stop processing messages. Host -> Enclave
  DEFINE_RINGBUFFER_MSG_TYPE(stop),

  /// Stopped processing messages. Enclave -> Host
  DEFINE_RINGBUFFER_MSG_TYPE(stopped),

  /// Periodically update based on current time. Host -> Enclave
  DEFINE_RINGBUFFER_MSG_TYPE(tick),

  /// Notify the host of work done since last message. Enclave -> Host
  DEFINE_RINGBUFFER_MSG_TYPE(work_stats)
};

DECLARE_RINGBUFFER_MESSAGE_PAYLOAD(
  AdminMessage::log_msg,
  std::chrono::milliseconds,
  std::string,
  size_t,
  logger::Level,
  uint16_t,
  std::string);
DECLARE_RINGBUFFER_MESSAGE_PAYLOAD(AdminMessage::fatal_error_msg, std::string);
DECLARE_RINGBUFFER_MESSAGE_NO_PAYLOAD(AdminMessage::stop);
DECLARE_RINGBUFFER_MESSAGE_NO_PAYLOAD(AdminMessage::stopped);
DECLARE_RINGBUFFER_MESSAGE_PAYLOAD(AdminMessage::tick, size_t);
DECLARE_RINGBUFFER_MESSAGE_PAYLOAD(AdminMessage::work_stats, std::string);
