// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "ds/json.h"

namespace ccf
{
  enum class TxStatus
  {
    Unknown,
    Pending,
    Committed,
    Invalid,
  };

  DECLARE_JSON_ENUM(
    TxStatus,
    {{TxStatus::Unknown, "UNKNOWN"},
     {TxStatus::Pending, "PENDING"},
     {TxStatus::Committed, "COMMITTED"},
     {TxStatus::Invalid, "INVALID"}});

  constexpr size_t VIEW_UNKNOWN = 0;

  static TxStatus get_tx_status(
    size_t target_view,
    size_t target_seqno,
    size_t local_view,
    size_t committed_view,
    size_t committed_seqno)
  {
    const bool is_committed = committed_seqno >= target_seqno;
    const bool views_match = local_view == target_view;
    const bool view_known = local_view != VIEW_UNKNOWN;

    if (is_committed && !view_known)
    {
      throw std::logic_error(fmt::format(
        "Should know local view for seqnos up to {}, but have no view for {}",
        committed_seqno,
        target_seqno));
    }

    if (local_view > committed_view)
    {
      throw std::logic_error(fmt::format(
        "Should not believe {} occurred in view {}, ahead of the current "
        "committed view {}",
        target_view,
        local_view,
        committed_view));
    }

    if (is_committed)
    {
      // The requested seqno has been committed, so we know for certain whether
      // the requested tx id is committed or not
      if (views_match)
      {
        return TxStatus::Committed;
      }
      else
      {
        return TxStatus::Invalid;
      }
    }
    else if (local_view == target_view)
    {
      // This node knows about the requested tx id, but it is not globally
      // committed
      return TxStatus::Pending;
    }
    else if (committed_view > target_view)
    {
      // This node has seen the seqno in a different view, and committed
      // further, so the requested tx id is impossible
      return TxStatus::Invalid;
    }
    else
    {
      // Otherwise, we cannot state anything about this tx id. The most common
      // reason is that the local_view is unknown (this transaction has never
      // existed, or has not reached this node yet). It is also possible that
      // this node believes locally that this tx id is impossible, but does not
      // have a global commit to back this up - it will eventually receive
      // either a global commit confirming this belief, or an election and
      // global commit where this tx id is valid
      return TxStatus::Unknown;
    }
  }
}