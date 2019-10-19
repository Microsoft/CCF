// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "Message.h"
#include "Node.h"
#include "Replica.h"
#include "Request.h"
#include "consensus/ledgerenclave.h"
#include "host/ledger.h"
#include "network_mock.h"

#include <cstdio>
#include <doctest/doctest.h>

static constexpr size_t TOTAL_REQUESTS = 30;

class ExecutionMock
{
public:
  ExecutionMock(size_t init_counter_) : command_counter(init_counter_) {}
  size_t command_counter;
  struct fake_req
  {
    uint8_t rt;
    int64_t ctx;
  };

  ExecCommand exec_command = [](
                               Byz_req* inb,
                               Byz_rep* outb,
                               _Byz_buffer* non_det,
                               int client,
                               bool ro,
                               Seqno total_requests_executed,
                               ByzInfo& info) {
    auto request = reinterpret_cast<fake_req*>(inb->contents);
    info.ctx = request->ctx;
    info.merkle_root.fill(0);
    info.merkle_root.data()[0] = request->rt;
    return 0;
  };
};

NodeInfo get_node_info()
{
  std::vector<PrincipalInfo> principal_info;

  PrincipalInfo pi = {
    0,
    (short)(3000),
    "ip",
    "96031a6cbe405894f1c0295881bd3946f0215f95fc40b7f1f0cc89b821c58504",
    "8691c3438859c142a26b5f251b96f39a463799430315d34ce8a4db0d2638f751",
    "name-1",
    true};
  principal_info.emplace_back(pi);

  GeneralInfo gi = {
    2, 0, 0, true, "generic", 1800000, 5000, 100, 9999250000, principal_info};

  NodeInfo node_info = {
    gi.principal_info[0],
    "0045c65ec31179652c57ae97f50de77e177a939dce74e39d7db51740663afb69",
    gi};

  return node_info;
}

void init_replica(
  std::vector<char>& service_mem,
  std::unique_ptr<consensus::LedgerEnclave> ledger)
{
  auto node_info = get_node_info();

  replica = new Replica(
    node_info,
    service_mem.data(),
    service_mem.size(),
    Create_Mock_Network(),
    std::move(ledger));
  replica->init_state();
  for (auto& pi : node_info.general_info.principal_info)
  {
    if (pi.id != node_info.own_info.id)
    {
      replica->add_principal(pi);
    }
  }
}

TEST_CASE("Test Ledger Replay")
{
  size_t circuit_size_shift = 22;
  ringbuffer::Circuit init_circuit(1 << circuit_size_shift);
  auto wf = ringbuffer::WriterFactory(init_circuit);
  auto initial_ledger_io = std::make_unique<consensus::LedgerEnclave>(wf);

  int mem_size = 400 * 8192;
  std::vector<char> service_mem(mem_size, 0);
  ExecutionMock exec_mock(0);

  // initiate replica with ledger enclave
  ringbuffer::Circuit replay_circuit(1 << circuit_size_shift);
  auto wf_rplay = ringbuffer::WriterFactory(replay_circuit);

  auto replay_ledger_io = std::make_unique<consensus::LedgerEnclave>(wf_rplay);

  init_replica(service_mem, std::move(replay_ledger_io));
  replica->register_exec(exec_mock.exec_command);

  INFO("Create dummy pre-prepares and write them to ledger file");
  {
    LedgerWriter ledger_writer(std::move(initial_ledger_io));

    Req_queue rqueue;
    for (size_t i = 1; i < TOTAL_REQUESTS; i++)
    {
      Byz_req req;
      Byz_alloc_request(&req, sizeof(ExecutionMock::fake_req));

      auto fr = reinterpret_cast<ExecutionMock::fake_req*>(req.contents);
      fr->rt = i;
      fr->ctx = i;

      Request* request = (Request*)req.opaque;
      request->request_id() = i;
      request->authenticate(req.size, false);
      request->mark_verified();
      request->trim();

      rqueue.append(request);
      size_t num_requests = 1;
      auto pp = std::make_unique<Pre_prepare>(1, i, rqueue, num_requests);

      // imitate exec command
      ByzInfo info;
      info.ctx = fr->ctx;
      info.merkle_root.fill(0);
      info.merkle_root.data()[0] = fr->rt;

      pp->set_merkle_root_and_ctx(info.merkle_root, info.ctx);

      ledger_writer.write_pre_prepare(pp.get());
    }
    // remove the requests that were not processed, only written to the ledger
    replica->big_reqs()->clear();
  }

  INFO("Read the ledger file and replay it out of order and in order");
  {
    std::vector<uint8_t> initial_ledger;
    std::vector<size_t> positions;
    size_t total_len = 0;
    size_t num_msgs = 0;
    // all entries will have the same size in this test
    size_t entry_size;
    init_circuit.read_from_inside().read(
      -1, [&](ringbuffer::Message m, const uint8_t* data, size_t size) {
        switch (m)
        {
          case consensus::ledger_append:
          {
            positions.push_back(total_len);
            size_t framed_entry_size = size + sizeof(uint32_t);
            total_len += framed_entry_size;
            initial_ledger.reserve(total_len);
            uint32_t s = (uint32_t)size;
            entry_size = size + sizeof(uint32_t);
            initial_ledger.insert(
              end(initial_ledger),
              (uint8_t*)&s,
              (uint8_t*)&s + sizeof(uint32_t));
            initial_ledger.insert(end(initial_ledger), data, data + size);
          }
          break;
          default:
            REQUIRE(false);
        }
        ++num_msgs;
      });
    REQUIRE(num_msgs == (TOTAL_REQUESTS - 1));
    // check that nothing gets executed out of order

    // read from entries 5 till 9
    auto first = initial_ledger.begin() + positions.at(4);
    auto last = initial_ledger.end();
    std::vector<uint8_t> vec_5_9(first, last);
    CHECK(!replica->apply_ledger_data(vec_5_9));

    // read entries 1 till 3
    first = initial_ledger.begin();
    last = initial_ledger.begin() + positions.at(3);
    std::vector<uint8_t> vec_1_3(first, last);
    CHECK(replica->apply_ledger_data(vec_1_3));

    // try 2 till 3 and expect replay to not execute
    first = initial_ledger.begin() + positions.at(1);
    last = initial_ledger.begin() + positions.at(3);
    std::vector<uint8_t> vec_2_3(first, last);
    CHECK(!replica->apply_ledger_data(vec_2_3));

    // execute the rest of the pre-prepares in batches of 3
    for (size_t i = 3; i < TOTAL_REQUESTS; i += 4)
    {
      auto until = i + 4;
      first = initial_ledger.begin() + positions.at(i);
      if (until >= positions.size())
      {
        last = initial_ledger.end();
      }
      else
      {
        last = initial_ledger.begin() + positions.at(until);
      }
      std::vector<uint8_t> vec(first, last);
      CHECK(replica->apply_ledger_data(vec));
    }

    std::vector<uint8_t> replay_ledger;
    std::vector<size_t> positions_replay;
    size_t total_len_replay = 0;
    replay_circuit.read_from_inside().read(
      -1, [&](ringbuffer::Message m, const uint8_t* data, size_t size) {
        switch (m)
        {
          case consensus::ledger_append:
          {
            positions_replay.push_back(total_len_replay);
            size_t framed_entry_size = size + sizeof(uint32_t);
            total_len_replay += framed_entry_size;
            replay_ledger.reserve(total_len_replay);

            uint32_t s = (uint32_t)size;
            entry_size = size + sizeof(uint32_t);
            replay_ledger.insert(
              end(replay_ledger),
              (uint8_t*)&s,
              (uint8_t*)&s + sizeof(uint32_t));
            replay_ledger.insert(end(replay_ledger), data, data + size);
          }
          break;
          case consensus::ledger_truncate:
          {
            auto last_idx = serialized::read<size_t>(data, size);
            total_len_replay = positions_replay.at(last_idx);
            positions_replay.resize(last_idx);

            replay_ledger.resize(total_len_replay);

            break;
          }
          default:
            REQUIRE(false);
        }
      });

    CHECK(std::equal(
      initial_ledger.begin(), initial_ledger.end(), replay_ledger.begin()));
  }
}