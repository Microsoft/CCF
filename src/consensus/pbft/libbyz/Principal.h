// Copyright (c) Microsoft Corporation.
// Copyright (c) 1999 Miguel Castro, Barbara Liskov.
// Copyright (c) 2000, 2001 Miguel Castro, Rodrigo Rodrigues, Barbara Liskov.
// Licensed under the MIT license.

#pragma once

#include "Time.h"
#include "network.h"
#include "tls/keypair.h"
#include "types.h"

#include <string.h>
#include <sys/time.h>

class Reply;

// Sizes in bytes.
const size_t UMAC_size = 16;
const size_t UNonce_size = sizeof(long long);
const size_t MAC_size = UMAC_size + UNonce_size;

const int Nonce_size = 16;
const int Nonce_size_u = Nonce_size / sizeof(unsigned);
const int Key_size = 16;
const int Key_size_u = Key_size / sizeof(unsigned);

const int Tag_size = 16;

class Principal : public IPrincipal
{
public:
  Principal(int i, Addr a, bool replica, const std::string& pub_key_sig);
  // Requires: "pkey" points to a null-terminated ascii encoding of
  // an integer in base-16 or is null (in which case no public-key is
  // associated with the principal.)
  // Effects: Creates a new Principal object.

  virtual ~Principal() = default;
  // Effects: Deallocates all the storage associated with principal.

  int pid() const;
  // Effects: Returns the principal identifier.

  const Addr* address() const;
  // Effects: Returns a pointer to the principal's address.

  bool is_replica() const;
  // Effects: Returns true iff this is a replica

  const std::string& get_pub_key_sig() const;

  size_t sig_size() const;
  // Effects: Returns the size of signatures generated by this principal.

  bool verify_signature(
    const char* src,
    unsigned src_len,
    const uint8_t* sig,
    bool allow_self = false);
  // Requires: "sig" is at least sig_size() bytes.
  // Effects: Checks a signature "sig" (from this principal) for
  // "src_len" bytes starting at "src". If "allow_self" is false, it
  // always returns false if "this->id == pbft::GlobalState::get_node().id()";
  // otherwise, returns true if signature is valid.

  Request_id last_fetch_rid() const;
  void set_last_fetch_rid(Request_id r);
  // Effects: Gets and sets the last request identifier in a fetch
  // message from this principal.

  bool received_network_open_msg() const;
  void set_received_network_open_msg();
  // Effects: Gets and sets if we have seen a network open message

  void set_certificate(const std::string& cert);
  bool has_certificate_configured();

private:
  int id;
  Addr addr;
  bool replica;
  tls::VerifierPtr verifier;
  std::string public_key_pem;
  size_t ssize; // signature size
  unsigned
    kin[Key_size_u]; // session key for incoming messages from this principal
  unsigned
    kout[Key_size_u]; // session key for outgoing messages to this principal
  ULong tstamp; // last timestamp in a new-key message from this principal
  Time my_tstamp; // my time when message was accepted

  Request_id
    last_fetch; // Last request_id in a fetch message from this principal
  bool has_received_network_open_msg;
};

inline bool Principal::has_certificate_configured()
{
  return !public_key_pem.empty();
}

inline void Principal::set_certificate(const std::string& cert)
{
  std::vector<uint8_t> v(cert.begin(), cert.end());
  verifier = tls::make_verifier(v);
  public_key_pem = cert;
  LOG_TRACE_FMT("Certificate for node {} set", id);
}

inline bool Principal::received_network_open_msg() const
{
  return has_received_network_open_msg;
}

inline void Principal::set_received_network_open_msg()
{
  has_received_network_open_msg = true;
}

inline const Addr* Principal::address() const
{
  return &addr;
}

inline int Principal::pid() const
{
  return id;
}

inline bool Principal::is_replica() const
{
  return replica;
}

inline size_t Principal::sig_size() const
{
  return ssize;
}

inline const std::string& Principal::get_pub_key_sig() const
{
  return public_key_pem;
}

inline Request_id Principal::last_fetch_rid() const
{
  return last_fetch;
}

inline void Principal::set_last_fetch_rid(Request_id r)
{
  last_fetch = r;
}

void random_nonce(unsigned* n);
// Requires: k is an array of at least Nonce_size bytes.
// Effects: Places a new random nonce with size Nonce_size bytes in n.
