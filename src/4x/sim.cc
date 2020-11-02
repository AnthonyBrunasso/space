#pragma once

#include "4x/hex_map.cc"

namespace fourx {

void
SimPlayerJoin(const proto::PlayerJoin& player_join)
{
  printf(".......SimPlayerJoin.......\n");
}

void
SimMapCreate(const proto::MapCreate& map_create)
{
}

void
SimStart(const proto::SimStart& sim_start)
{
}

}  // namespace fourx
