#pragma once

#include "4x/hex_map.cc"

namespace fourx {

void
SimPlayerJoin(const proto::PlayerJoin& player_join)
{
  printf("SimPlayerJoin: %s\n", player_join.DebugString().c_str());
}

void
SimMapCreate(const proto::MapCreate& map_create)
{
}

void
SimStart(const proto::SimStart& sim_start)
{
}

void
SimExecute(const proto::SimulationStepRequest& request)
{
  switch (request.step_case()) {
    case proto::SimulationStepRequest::kPlayerJoin: {
      SimPlayerJoin(request.player_join());
    } break;
    case proto::SimulationStepRequest::kMapCreate: {
      SimMapCreate(request.map_create());
    } break;
    case proto::SimulationStepRequest::kSimStart: {
      SimStart(request.sim_start());
    } break;
    default: break;
  }
}

}  // namespace fourx
