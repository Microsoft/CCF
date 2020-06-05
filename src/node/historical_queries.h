// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "consensus/ledger_enclave_types.h"
#include "kv/store.h"
#include "node/rpc/node_interface.h"

#include <deque>
#include <map>
#include <memory>

namespace ccf::historical
{
  class StateCache
  {
  protected:
    kv::Store& source_store;
    ringbuffer::WriterPtr to_host;

    // TODO: Add sensible caps on all these containers?
    std::set<consensus::Index> requested_indices;

    static constexpr size_t MAX_PENDING_FETCHES = 10;
    std::deque<consensus::Index> pending_fetches;

    using StorePtr = std::shared_ptr<kv::Store>;
    std::map<consensus::Index, StorePtr> untrusted_entries;

    // TODO: Make this an LRU, evict old entries
    std::map<consensus::Index, StorePtr> trusted_entries;

    using LedgerEntry = std::vector<uint8_t>;

    void request_entry_at(consensus::Index idx)
    {
      const auto ib = requested_indices.insert(idx);
      if (ib.second)
      {
        fetch_entry_at(idx);
      }
    }

    void fetch_entry_at(consensus::Index idx)
    {
      if (pending_fetches.size() < MAX_PENDING_FETCHES)
      {
        const auto it =
          std::find(pending_fetches.begin(), pending_fetches.end(), idx);
        if (it != pending_fetches.end())
        {
          // Already fetching this index
          return;
        }

        RINGBUFFER_WRITE_MESSAGE(
          consensus::ledger_get,
          to_host,
          idx,
          consensus::LedgerRequestPurpose::HistoricalQuery);
        pending_fetches.push_back(idx);
      }

      // Too many outstanding fetches, this one is silently ignored
      // TODO
    }

    void update_trusted_stores(consensus::Index idx, const LedgerEntry& entry)
    {
      StorePtr store = std::make_shared<kv::Store>();

      store->set_encryptor(source_store.get_encryptor());

      // TODO: Add a lazy clone option?
      store->clone_schema(source_store);
      store->set_strict_versions(false);

      const auto deserialise_result = store->deserialise_views(entry);

      switch (deserialise_result)
      {
        case kv::DeserialiseSuccess::FAILED:
        {
          // TODO: Host gave us junk, did they mean to? Do we fail silently?
          throw std::logic_error("Deserialise failed!");
          break;
        }
        case kv::DeserialiseSuccess::PASS:
        {
          const auto request_it = requested_indices.find(idx);
          if (request_it != requested_indices.end())
          {
            // We were looking for this entry! Store the produced store
            // TODO: Should we check if we already have this? If the host spams
            // us, do we just ignore everything but the first result they give
            // us?
            untrusted_entries[idx] = store;
          }

          // In either case, its not a signature - try the next transaction
          fetch_entry_at(idx + 1);
          break;
        }
        case kv::DeserialiseSuccess::PASS_SIGNATURE:
        {
          LOG_INFO_FMT("Found a signature transaction at {}", idx);
          // Hurrah! We can hopefully use this signature to move some old stores
          // from untrusted to trusted!

          // Get signatures table

          // Parse signatures entry

          // Check each of the transactions it covers

          // See if this includes any of ours

          // Check that the signature matches the entry we got!

          // Move signed stores from untrusted to trusted
          // TODO: Temp solution, blindly trust everything for now
          for (const auto& [k, v] : untrusted_entries)
          {
            trusted_entries[k] = v;
          }
          untrusted_entries.clear();
          break;
        }
        default:
        {
          throw std::logic_error("Unexpected deserialise result");
        }
      }
    }

  public:
    StateCache(kv::Store& store, const ringbuffer::WriterPtr& host_writer) :
      source_store(store),
      to_host(host_writer)
    {}

    StorePtr get_store_at(consensus::Index idx)
    {
      const auto trusted_it = trusted_entries.find(idx);
      if (trusted_it == trusted_entries.end())
      {
        // Otherwise, treat this as a hint and (potentially) start fetching it
        request_entry_at(idx);

        return nullptr;
      }

      return trusted_it->second;
    }

    bool handle_ledger_entry(consensus::Index idx, const LedgerEntry& data)
    {
      const auto it =
        std::find(pending_fetches.begin(), pending_fetches.end(), idx);
      if (it == pending_fetches.end())
      {
        // TODO
        // Unexpected entry - probably just ignore it?
        return false;
      }

      pending_fetches.erase(it);
      update_trusted_stores(idx, data);
      return true;
    }

    void handle_no_entry(consensus::Index idx)
    {
      const auto it =
        std::find(pending_fetches.begin(), pending_fetches.end(), idx);
      if (it == pending_fetches.end())
      {
        // TODO
        // We weren't even expecting this entry - surely just ignore it?
        return;
      }

      // The host failed or refused to gives us this. Currently we just forget
      // about it, we don't have a mechanism for remembering this failure and
      // reporting it to users.
      pending_fetches.erase(it);
    }
  };
}