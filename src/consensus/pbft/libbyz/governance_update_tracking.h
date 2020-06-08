// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "ds/dl_list.h"
#include "types.h"

class GovernanceRequestTracking
{
public:
  bool was_updated_in(Seqno seqno)
  {
    SeqnoWithMemberReq* current = seqnoWithMemberReqs.get_head();
    while (current != nullptr && current->seqno >= seqno)
    {
      if (current->seqno == seqno)
      {
        return true;
      }
      current = current->next;
    }
    return false;
  }

  void rollback(Seqno seqno)
  {
    Seqno last = last_seqno();
    mark_stable(seqno);
    LOG_INFO_FMT("rollingback last gov req, before:{}, after:{}, rollback_target:{}", last, last_seqno(), seqno);
  }

  void update(Seqno seqno)
  {
    LOG_INFO_FMT("Updating last gov req, before:{}, after:{}", last_seqno(), seqno);
    seqnoWithMemberReqs.insert(new SeqnoWithMemberReq{seqno, nullptr, nullptr});
  }

  void mark_stable(Seqno seqno)
  {
    while (seqnoWithMemberReqs.get_tail() != seqnoWithMemberReqs.get_head() &&
           seqnoWithMemberReqs.get_tail()->seqno < seqno)
    {
      delete seqnoWithMemberReqs.pop_tail();
    }
  }

  Seqno last_seqno()
  {
    if (seqnoWithMemberReqs.is_empty())
    {
      return 0;
    }
    return seqnoWithMemberReqs.get_head()->seqno;
  }

private:
  struct SeqnoWithMemberReq
  {
    Seqno seqno;
    SeqnoWithMemberReq* next;
    SeqnoWithMemberReq* prev;
  };

  snmalloc::DLList<SeqnoWithMemberReq> seqnoWithMemberReqs;
};