// Copyright (c) Microsoft Corporation.
// Copyright (c) 1999 Miguel Castro, Barbara Liskov.
// Copyright (c) 2000, 2001 Miguel Castro, Rodrigo Rodrigues, Barbara Liskov.
// Licensed under the MIT license.
#pragma once

#include "Message.h"

//
// Network open messages have the following format:
//
#pragma pack(push)
#pragma pack(1)
struct Network_open_rep : public Message_rep
{
  int id; // id of the replica that generated the message.
  char padding[5];
};
#pragma pack(pop)

class Network_open : public Message
{
public:
  Network_open(int id);

  int id() const;

  static bool convert(Message* m1, Network_open*& m2);

private:
  Network_open_rep& rep() const;
};
