// Copyright (c) Microsoft Corporation.
// Copyright (c) 1999 Miguel Castro, Barbara Liskov.
// Copyright (c) 2000, 2001 Miguel Castro, Rodrigo Rodrigues, Barbara Liskov.
// Licensed under the MIT license.

#include "Principal.h"

#include "Node.h"
#include "Reply.h"
#include "crypt.h"
#include "epbft_drng.h"

#include <stdlib.h>
#include <strings.h>

Principal::Principal(
  int i, Addr a, bool is_rep, const std::vector<uint8_t>& cert_)
{
  id = i;
  addr = a;
  last_fetch = 0;
  replica = is_rep;

  ssize = tls::PbftSignatureSize;
  if (!cert_.empty())
  {
    verifier = tls::make_verifier(cert_);
    cert = cert_;
  }

  for (int j = 0; j < 4; j++)
  {
    kin[j] = 0;
    kout[j] = 0;
  }

  tstamp = 0;
  my_tstamp = zero_time();
  has_received_network_open_msg = false;
}

bool Principal::verify_signature(
  const char* src, unsigned src_len, const uint8_t* sig, bool allow_self)
{
  // Principal never verifies its own authenticator.
  if ((id == pbft::GlobalState::get_node().id()) && !allow_self)
  {
    return false;
  }

  INCR_OP(num_sig_ver);
  START_CC(sig_ver_cycles);

  bool ret = verifier->verify((uint8_t*)src, src_len, sig, sig_size());

  STOP_CC(sig_ver_cycles);
  return ret;
}

void random_nonce(unsigned* n)
{
  epbft::IntelDRNG drng;
  drng.rng(0, (unsigned char*)n, Nonce_size);
}
