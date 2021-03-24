// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "crypto/hash_provider.h"
#include "crypto/key_pair.h"
#include "crypto/symmetric_key.h"
#include "crypto/verifier.h"
#include "ds/logger.h"
#include "ds/serialized.h"
#include "ds/spin_lock.h"
#include "entities.h"
#include "node/entity_id.h"
#include "node_types.h"
#include "tls/key_exchange.h"

#include <iostream>
#include <map>
#include <mbedtls/ecdh.h>

namespace ccf
{
  using SeqNo = uint64_t;
  using GcmHdr = crypto::GcmHeader<sizeof(SeqNo)>;

  struct RecvNonce
  {
    uint8_t tid;
    uint64_t nonce : (sizeof(uint64_t) - sizeof(uint8_t)) * CHAR_BIT;

    RecvNonce(uint64_t nonce_, uint8_t tid_) : tid(tid_), nonce(nonce_) {}
    RecvNonce(const uint64_t header)
    {
      *this = *reinterpret_cast<const RecvNonce*>(&header);
    }

    uint64_t get_val() const
    {
      return *reinterpret_cast<const uint64_t*>(this);
    }
  };
  static_assert(
    sizeof(RecvNonce) == sizeof(SeqNo), "RecvNonce is the wrong size");

  static inline RecvNonce get_nonce(const GcmHdr& header)
  {
    return RecvNonce(header.get_iv_int());
  }

  enum ChannelStatus
  {
    INACTIVE = 0,
    INITIATED,
    WAITING_FOR_FINAL,
    ESTABLISHED
  };

  class Channel
  {
  private:
    struct OutgoingMsg
    {
      NodeMsgType type;
      std::vector<uint8_t> raw_plain; // To be integrity-protected
      std::vector<uint8_t> raw_cipher; // To be encrypted

      OutgoingMsg(
        NodeMsgType msg_type, CBuffer raw_plain_, CBuffer raw_cipher_) :
        type(msg_type),
        raw_plain(raw_plain_),
        raw_cipher(raw_cipher_)
      {}
    };

    NodeId self;
    const crypto::Pem& network_cert;
    crypto::KeyPairPtr node_kp;
    const crypto::Pem& node_cert;
    crypto::VerifierPtr peer_cv;
    crypto::Pem peer_cert;

    // Notifies the host to create a new outgoing connection
    ringbuffer::WriterPtr to_host;
    NodeId peer_id;
    std::string peer_hostname;
    std::string peer_service;
    bool outgoing;

    // Used for key exchange
    tls::KeyExchangeContext kex_ctx;
    ChannelStatus status = INACTIVE;
    std::vector<uint8_t> hkdf_salt;

    // Used for AES GCM authentication/encryption
    std::unique_ptr<crypto::KeyAesGcm> key;

    // Incremented for each tagged/encrypted message
    std::atomic<SeqNo> send_nonce{1};

    // Used to buffer at most one message sent on the channel before it is
    // established
    std::optional<OutgoingMsg> outgoing_msg;

    // Used to prevent replayed messages.
    // Set to the latest successfully received nonce.
    struct ChannelSeqno
    {
      SeqNo main_thread_seqno;
      SeqNo tid_seqno;
    };
    std::array<ChannelSeqno, threading::ThreadMessaging::max_num_threads>
      local_recv_nonce = {{}};

    bool verify_or_decrypt(
      const GcmHdr& header,
      CBuffer aad,
      CBuffer cipher = nullb,
      Buffer plain = {})
    {
      if (status != ESTABLISHED)
      {
        throw std::logic_error("Channel is not established for verifying");
      }

      RecvNonce recv_nonce(header.get_iv_int());
      auto tid = recv_nonce.tid;

      uint16_t current_tid = threading::get_current_thread_id();
      assert(
        current_tid == threading::ThreadMessaging::main_thread ||
        current_tid % threading::ThreadMessaging::thread_count == tid);

      SeqNo* local_nonce;
      if (current_tid == threading::ThreadMessaging::main_thread)
      {
        local_nonce = &local_recv_nonce[tid].main_thread_seqno;
      }
      else
      {
        local_nonce = &local_recv_nonce[tid].tid_seqno;
      }

      if (recv_nonce.nonce <= *local_nonce)
      {
        // If the nonce received has already been processed, return
        LOG_FAIL_FMT(
          "Invalid nonce, possible replay attack, from:{} received:{}, "
          "last_seen:{}, recv_nonce.tid:{}",
          peer_id,
          reinterpret_cast<uint64_t>(recv_nonce.nonce),
          *local_nonce,
          recv_nonce.tid);
        return false;
      }

      auto ret =
        key->decrypt(header.get_iv(), header.tag, cipher, aad, plain.p);
      if (ret)
      {
        // Set local recv nonce to received nonce only if verification is
        // successful
        *local_nonce = recv_nonce.nonce;
      }

      return ret;
    }

  public:
    static constexpr size_t protocol_version = 1;

    Channel(
      ringbuffer::AbstractWriterFactory& writer_factory,
      const crypto::Pem& network_cert_,
      crypto::KeyPairPtr node_kp_,
      const crypto::Pem& node_cert_,
      const NodeId& self_,
      const NodeId& peer_id_,
      const std::string& peer_hostname_,
      const std::string& peer_service_) :
      self(self_),
      network_cert(network_cert_),
      node_kp(node_kp_),
      node_cert(node_cert_),
      to_host(writer_factory.create_writer_to_outside()),
      peer_id(peer_id_),
      peer_hostname(peer_hostname_),
      peer_service(peer_service_),
      outgoing(true)
    {
      RINGBUFFER_WRITE_MESSAGE(
        ccf::add_node, to_host, peer_id.value(), peer_hostname, peer_service);
      auto e = crypto::create_entropy();
      hkdf_salt = e->random(32);
    }

    Channel(
      ringbuffer::AbstractWriterFactory& writer_factory,
      const crypto::Pem& network_cert_,
      crypto::KeyPairPtr node_kp_,
      const crypto::Pem& node_cert_,
      const NodeId& self_,
      const NodeId& peer_id_) :
      self(self_),
      network_cert(network_cert_),
      node_kp(node_kp_),
      node_cert(node_cert_),
      to_host(writer_factory.create_writer_to_outside()),
      peer_id(peer_id_),
      outgoing(false)
    {
      auto e = crypto::create_entropy();
      hkdf_salt = e->random(32);
    }

    ~Channel()
    {
      if (outgoing)
      {
        RINGBUFFER_WRITE_MESSAGE(ccf::remove_node, to_host, peer_id.value());
      }
    }

    void set_status(ChannelStatus status_)
    {
      status = status_;
    }

    ChannelStatus get_status()
    {
      return status;
    }

    bool is_outgoing() const
    {
      return outgoing;
    }

    void set_outgoing(
      const std::string& peer_hostname_, const std::string& peer_service_)
    {
      peer_hostname = peer_hostname_;
      peer_service = peer_service_;

      if (!outgoing)
      {
        RINGBUFFER_WRITE_MESSAGE(
          ccf::add_node, to_host, peer_id.value(), peer_hostname, peer_service);
      }
      outgoing = true;
    }

    void reset_outgoing()
    {
      if (outgoing)
      {
        RINGBUFFER_WRITE_MESSAGE(ccf::remove_node, to_host, peer_id.value());
      }
      outgoing = false;
    }

    std::vector<uint8_t> sign_key_share(
      const std::vector<uint8_t>& ks,
      bool with_salt = false,
      const std::vector<uint8_t>* extra = nullptr)
    {
      std::vector<uint8_t> t = ks;

      if (extra)
      {
        t.insert(t.end(), extra->begin(), extra->end());
      }

      auto signature = node_kp->sign(t);

      // Serialise channel key share, signature, and certificate and
      // length-prefix them
      auto space =
        ks.size() + signature.size() + node_cert.size() + 4 * sizeof(size_t);
      if (with_salt)
      {
        space += hkdf_salt.size() + sizeof(size_t);
      }
      std::vector<uint8_t> serialised_signed_key_share(space);
      auto data_ = serialised_signed_key_share.data();
      serialized::write(data_, space, protocol_version);
      serialized::write(data_, space, ks.size());
      serialized::write(data_, space, ks.data(), ks.size());
      serialized::write(data_, space, signature.size());
      serialized::write(data_, space, signature.data(), signature.size());
      serialized::write(data_, space, node_cert.size());
      serialized::write(data_, space, node_cert.data(), node_cert.size());
      if (with_salt)
      {
        serialized::write(data_, space, hkdf_salt.size());
        serialized::write(data_, space, hkdf_salt.data(), hkdf_salt.size());
      }

      return serialised_signed_key_share;
    }

    std::vector<uint8_t> get_signed_key_share(bool with_salt)
    {
      return sign_key_share(kex_ctx.get_own_key_share(), with_salt);
    }

    CBuffer extract_buffer(const uint8_t*& data, size_t& size) const
    {
      if (size == 0)
      {
        return {};
      }

      auto sz = serialized::read<size_t>(data, size);
      CBuffer r(data, sz);

      if (r.n > size)
      {
        LOG_FAIL_FMT(
          "Buffer header wants {} bytes, but only {} remain", r.n, size);
        r.n = 0;
      }
      else
      {
        data += r.n;
        size -= r.n;
      }

      return r;
    }

    bool verify_peer_certificate(CBuffer pc)
    {
      if (pc.n != 0)
      {
        peer_cert = crypto::Pem(pc);
        peer_cv = crypto::make_verifier(peer_cert);

        if (!peer_cv->verify_certificate({&network_cert}))
        {
          LOG_FAIL_FMT("Peer certificate verification failed");
          reset();
          return false;
        }

        LOG_DEBUG_FMT(
          "New peer certificate: {}\n{}",
          peer_cv->serial_number(),
          peer_cert.str());
      }

      return true;
    }

    bool verify_peer_signature(CBuffer msg, CBuffer sig)
    {
      LOG_DEBUG_FMT(
        "Verifying peer signature with peer certificate serial {}",
        peer_cv ? peer_cv->serial_number() : "no peer_cv!");

      if (!peer_cv || !peer_cv->verify(msg, sig))
      {
        LOG_FAIL_FMT(
          "Node channel peer signature verification failed for {} with "
          "certificate serial {}",
          peer_id,
          peer_cv->serial_number());
        return false;
      }

      return true;
    }

    // Protocol overview:
    //
    // initiate()
    // > key_exchange_init message
    // consume_initiator_key_share() [by responder]
    // < key_exchange_response message
    // consume_responder_key_share() [by initiator]
    // > key_exchange_final message
    // check_peer_key_share_signature() [by responder]
    // both reach status == ESTABLISHED

    bool consume_responder_key_share(const std::vector<uint8_t>& data)
    {
      return consume_responder_key_share(data.data(), data.size());
    }

    bool consume_responder_key_share(const uint8_t* data, size_t size)
    {
      LOG_DEBUG_FMT("status == {}", status);

      if (status != INITIATED && status != ESTABLISHED)
      {
        return false;
      }

      size_t peer_version = serialized::read<size_t>(data, size);
      CBuffer ks = extract_buffer(data, size);
      CBuffer sig = extract_buffer(data, size);
      CBuffer pc = extract_buffer(data, size);

      LOG_DEBUG_FMT(
        "From responder {}: version={} ks={} sig={} pc={}",
        peer_id,
        peer_version,
        ds::to_hex(ks),
        ds::to_hex(sig),
        ds::to_hex(pc));

      if (size != 0)
      {
        LOG_FAIL_FMT("{} exccess bytes remaining", size);
        return false;
      }

      if (peer_version != protocol_version || ks.n == 0 || sig.n == 0)
      {
        return false;
      }

      if (!verify_peer_certificate(pc))
      {
        return false;
      }

      // We are the initiator and expect a signature over both key shares
      std::vector<uint8_t> t = {ks.p, ks.p + ks.n};
      auto oks = kex_ctx.get_own_key_share();
      t.insert(t.end(), oks.begin(), oks.end());

      if (!verify_peer_signature(t, sig))
      {
        return false;
      }

      kex_ctx.load_peer_key_share(ks);

      // Sign the peer's key share
      auto signature = node_kp->sign(ks);

      // Serialise signature with length-prefix
      auto space = signature.size() + 1 * sizeof(size_t);
      std::vector<uint8_t> serialised_signature(space);
      auto data_ = serialised_signature.data();
      serialized::write(data_, space, signature.size());
      serialized::write(data_, space, signature.data(), signature.size());

      to_host->write(
        node_outbound,
        peer_id.value(),
        NodeMsgType::channel_msg,
        self.value(),
        ChannelMsg::key_exchange_final,
        serialised_signature);

      LOG_DEBUG_FMT(
        "key_exchange_final -> {}: ks={} serialised_signed_key_share={}",
        peer_id,
        ds::to_hex(ks),
        ds::to_hex(serialised_signature));

      establish(true);

      return true;
    }

    bool consume_initiator_key_share(
      const std::vector<uint8_t>& data, bool priority = false)
    {
      return consume_initiator_key_share(data.data(), data.size(), priority);
    }

    bool consume_initiator_key_share(
      const uint8_t* data, size_t size, bool priority = false)
    {
      LOG_DEBUG_FMT("status == {}", status);

      if (status == INITIATED)
      {
        // Both nodes tried to initiate the channel, the one with priority wins.
        if (!priority)
          return true;
      }
      else if (status == WAITING_FOR_FINAL)
      {
        return false;
      }

      size_t peer_version = serialized::read<size_t>(data, size);
      CBuffer ks = extract_buffer(data, size);
      CBuffer sig = extract_buffer(data, size);
      CBuffer pc = extract_buffer(data, size);
      CBuffer salt = extract_buffer(data, size);

      LOG_DEBUG_FMT(
        "From initiator {}: version={} ks={} sig={} pc={} salt={}",
        peer_id,
        peer_version,
        ds::to_hex(ks),
        ds::to_hex(sig),
        ds::to_hex(pc),
        ds::to_hex(salt));

      if (size != 0)
      {
        LOG_FAIL_FMT("{} exccess bytes remaining", size);
        return false;
      }

      hkdf_salt = {salt.p, salt.p + salt.n};

      if (peer_version != protocol_version || ks.n == 0 || sig.n == 0)
      {
        return false;
      }

      if (!verify_peer_certificate(pc) || !verify_peer_signature(ks, sig))
      {
        return false;
      }

      if (status == ESTABLISHED)
      {
        // key_ctx does not hold a key share; we need a new one.
        kex_ctx.reset();
      }

      kex_ctx.load_peer_key_share(ks);
      status = WAITING_FOR_FINAL;

      // We are the responder and we return a signature over both public key
      // shares back to the initiator

      auto oks = kex_ctx.get_own_key_share();
      std::vector<uint8_t> pks = {ks.p, ks.p + ks.n};
      auto serialised_signed_share = sign_key_share(oks, false, &pks);

      to_host->write(
        node_outbound,
        peer_id.value(),
        NodeMsgType::channel_msg,
        self.value(),
        ChannelMsg::key_exchange_response,
        serialised_signed_share);

      LOG_DEBUG_FMT(
        "key_exchange_response -> {}: oks={} serialised_signed_share={}",
        peer_id,
        ds::to_hex(oks),
        ds::to_hex(serialised_signed_share));

      return true;
    }

    bool check_peer_key_share_signature(const std::vector<uint8_t>& data)
    {
      return check_peer_key_share_signature(data.data(), data.size());
    }

    bool check_peer_key_share_signature(const uint8_t* data, size_t size)
    {
      LOG_DEBUG_FMT("status == {}", status);

      if (status != WAITING_FOR_FINAL)
      {
        return false;
      }

      auto oks = kex_ctx.get_own_key_share();

      CBuffer sig = extract_buffer(data, size);

      if (!verify_peer_signature(oks, sig))
        return false;

      establish(false);

      return true;
    }

    void establish(bool initiator)
    {
      auto shared_secret = kex_ctx.compute_shared_secret();
      std::string label;
      if (initiator)
      {
        label = self.value() + peer_id.value();
      }
      else
      {
        label = peer_id.value() + self.value();
      }
      std::vector<uint8_t> info = {label.data(), label.data() + label.size()};
      LOG_DEBUG_FMT("salt={}", ds::to_hex(hkdf_salt));
      auto key_bytes = crypto::hkdf(
        crypto::MDType::SHA256, 32, shared_secret, hkdf_salt, info);
      key = crypto::make_key_aes_gcm(key_bytes);
      kex_ctx.free_ctx();

      status = ESTABLISHED;

      if (outgoing_msg.has_value())
      {
        send(
          outgoing_msg->type,
          outgoing_msg->raw_plain,
          outgoing_msg->raw_cipher);
        outgoing_msg.reset();
      }

      auto node_cv = make_verifier(node_cert);
      LOG_INFO_FMT("Node channel with {} is now established.", peer_id);

      LOG_DEBUG_FMT(
        "Node certificate serial numbers: node={} peer={}",
        node_cv->serial_number(),
        peer_cv->serial_number());
    }

    void initiate()
    {
      if (status == WAITING_FOR_FINAL || status == ESTABLISHED)
        return;

      status = INITIATED;

      to_host->write(
        node_outbound,
        peer_id.value(),
        NodeMsgType::channel_msg,
        self.value(),
        ChannelMsg::key_exchange_init,
        get_signed_key_share(true));

      auto sn = make_verifier(node_cert)->serial_number();
      LOG_DEBUG_FMT("key_exchange_init -> {} node serial: {}", peer_id, sn);
    }

    bool send(NodeMsgType type, CBuffer aad, CBuffer plain = nullb)
    {
      if (status != ESTABLISHED)
      {
        initiate();
        outgoing_msg = OutgoingMsg(type, aad, plain);
        return false;
      }

      RecvNonce nonce(
        send_nonce.fetch_add(1), threading::get_current_thread_id());

      GcmHdr gcm_hdr;
      gcm_hdr.set_iv_seq(nonce.get_val());

      std::vector<uint8_t> cipher(plain.n);
      key->encrypt(gcm_hdr.get_iv(), plain, aad, cipher.data(), gcm_hdr.tag);

      to_host->write(
        node_outbound,
        peer_id.value(),
        type,
        self.value(),
        serializer::ByteRange{aad.p, aad.n},
        gcm_hdr,
        cipher);

      LOG_DEBUG_FMT("-> {}: encrypted msg of type {}", peer_id, type);

      return true;
    }

    bool recv_authenticated(CBuffer aad, const uint8_t*& data, size_t& size)
    {
      // Receive authenticated message, modifying data to point to the start of
      // the non-authenticated plaintext payload
      if (status != ESTABLISHED)
      {
        LOG_FAIL_FMT(
          "Node channel with {} cannot receive authenticated message: not "
          "yet established",
          peer_id);
        return false;
      }

      const auto& hdr = serialized::overlay<GcmHdr>(data, size);
      if (!verify_or_decrypt(hdr, aad))
      {
        LOG_FAIL_FMT("Failed to verify node message from {}", peer_id);
        return false;
      }

      return true;
    }

    bool recv_authenticated_with_load(const uint8_t*& data, size_t& size)
    {
      // Receive authenticated message, modifying data to point to the start of
      // the non-authenticated plaintex payload. data contains payload first,
      // then GCM header

      if (status != ESTABLISHED)
      {
        LOG_FAIL_FMT(
          "node channel with {} cannot receive authenticated with payload "
          "message: not yet established",
          peer_id);
        return false;
      }

      const uint8_t* data_ = data;
      size_t size_ = size;

      serialized::skip(data_, size_, (size_ - sizeof(GcmHdr)));
      const auto& hdr = serialized::overlay<GcmHdr>(data_, size_);
      size -= sizeof(GcmHdr);

      if (!verify_or_decrypt(hdr, {data, size}))
      {
        LOG_FAIL_FMT("Failed to verify node message from {}", peer_id);
        return false;
      }

      return true;
    }

    std::optional<std::vector<uint8_t>> recv_encrypted(
      CBuffer aad, const uint8_t* data, size_t size)
    {
      // Receive encrypted message, returning the decrypted payload
      if (status != ESTABLISHED)
      {
        LOG_FAIL_FMT(
          "Node channel with {} cannot receive encrypted message: not yet "
          "established",
          peer_id);
        return std::nullopt;
      }

      const auto& hdr = serialized::overlay<GcmHdr>(data, size);
      std::vector<uint8_t> plain(size);
      if (!verify_or_decrypt(hdr, aad, {data, size}, plain))
      {
        LOG_FAIL_FMT("Failed to decrypt node message from {}", peer_id);
        return std::nullopt;
      }

      return plain;
    }

    void reset()
    {
      LOG_INFO_FMT("Resetting channel with {}", peer_id);

      status = INACTIVE;

      kex_ctx.reset();
      key.reset();
      peer_cert = {};
      peer_cv.reset();

      auto e = crypto::create_entropy();
      hkdf_salt = e->random(32);
    }

    void restart()
    {
      LOG_INFO_FMT("Restarting node channel with {}", peer_id);

      to_host->write(
        node_outbound,
        peer_id.value(),
        NodeMsgType::channel_msg,
        self.value(),
        ChannelMsg::key_exchange_restart);

      LOG_DEBUG_FMT("key_exchange_restart -> {}", peer_id);
    }
  };

  class ChannelManager
  {
  private:
    std::unordered_map<NodeId, std::shared_ptr<Channel>> channels;
    ringbuffer::AbstractWriterFactory& writer_factory;
    const crypto::Pem& network_cert;
    crypto::KeyPairPtr node_kp;
    const crypto::Pem& node_cert;
    NodeId self;
    SpinLock lock;

  public:
    ChannelManager(
      ringbuffer::AbstractWriterFactory& writer_factory_,
      const crypto::Pem& network_cert_,
      crypto::KeyPairPtr node_kp_,
      const crypto::Pem& node_cert_,
      const NodeId& self_) :
      writer_factory(writer_factory_),
      network_cert(network_cert_),
      node_kp(node_kp_),
      node_cert(node_cert_),
      self(self_)
    {}

    void create_channel(
      const NodeId& peer_id,
      const std::string& hostname,
      const std::string& service)
    {
      std::lock_guard<SpinLock> guard(lock);
      auto search = channels.find(peer_id);
      if (search == channels.end())
      {
        LOG_DEBUG_FMT(
          "Creating new outbound channel to {} ({}:{})",
          peer_id,
          hostname,
          service);
        auto channel = std::make_shared<Channel>(
          writer_factory,
          network_cert,
          node_kp,
          node_cert,
          self,
          peer_id,
          hostname,
          service);
        channels.emplace_hint(search, peer_id, std::move(channel));
      }
      else if (!search->second->is_outgoing())
      {
        // Channel with peer already exists but is incoming. Create host
        // outgoing connection.
        LOG_DEBUG_FMT("Setting existing channel to {} as outgoing", peer_id);
        search->second->set_outgoing(hostname, service);
        return;
      }
    }

    void restart()
    {
      for (auto c : channels)
      {
        c.second->restart();
      }
    }

    void destroy_channel(const NodeId& peer_id)
    {
      std::lock_guard<SpinLock> guard(lock);
      auto search = channels.find(peer_id);
      if (search == channels.end())
      {
        LOG_FAIL_FMT(
          "Cannot destroy node channel with {}: channel does not exist",
          peer_id);
        return;
      }

      channels.erase(peer_id);
    }

    void destroy_all_channels()
    {
      std::lock_guard<SpinLock> guard(lock);
      channels.clear();
    }

    void close_all_outgoing()
    {
      std::lock_guard<SpinLock> guard(lock);
      for (auto& c : channels)
      {
        if (c.second->is_outgoing())
        {
          c.second->reset_outgoing();
        }
      }
    }

    std::shared_ptr<Channel> get(const NodeId& peer_id)
    {
      std::lock_guard<SpinLock> guard(lock);
      auto search = channels.find(peer_id);
      if (search != channels.end())
      {
        return search->second;
      }

      // Creating temporary channel that is not outgoing (at least for now)
      channels.try_emplace(
        peer_id,
        std::make_shared<Channel>(
          writer_factory, network_cert, node_kp, node_cert, self, peer_id));
      return channels.at(peer_id);
    }
  };
}
