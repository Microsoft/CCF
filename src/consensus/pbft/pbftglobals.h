// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once
#include "libbyz/ITimer.h"
#include "libbyz/Statistics.h"
#include "node/nodetonode.h"

#include <signal.h>

namespace pbft
{
  char* AbstractPbftConfig::service_mem = 0;
};

std::vector<ITimer*> ITimer::timers;
Time ITimer::min_deadline = Long_max;
Time ITimer::_relative_current_time = 0;

long long clock_mhz = 1;

Statistics stats;