#pragma once

#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"

#include "4xsim.grpc.pb.h"

#include <unordered_map>

namespace fourx {

using fourx::proto::SimulationStepRequest;
using fourx::proto::SimulationStepResponse;
using fourx::proto::SimulationSyncRequest;
using fourx::proto::SimulationSyncResponse;

using fourx::proto::PlayerJoin;

constexpr s32 kInvalidPlayer = 0;

struct ServerPlayer {
  s32 id;
  std::string name;
  u32 sequence_number = 0;
};

class ServerState {
 public:
  void
  PushStep(const SimulationStepRequest& step)
  {
    steps_.push_back(step);
  }

  ServerPlayer*
  FindPlayerByName(const std::string& name)
  {
    for (auto& p : players_) {
      if (p.second.name == name) {
        return &p.second;
      }
    }
    return nullptr;
  }

  ServerPlayer*
  FindPlayer(s32 id)
  {
    auto found = players_.find(id);
    if (found == players_.end()) return nullptr;
    return &found->second;
  }

  s32
  AddPlayer(const std::string& name)
  {
    ServerPlayer server_player;
    server_player.name = name;
    server_player.id = auto_increment_id_++;
    server_player.sequence_number = 0;
    PlayerJoin join_request;
    join_request.set_name(name);
    join_request.set_id(server_player.id);
    SimulationStepRequest step_request;
    step_request.set_player_id(kInvalidPlayer);
    *step_request.mutable_player_join() = join_request;
    steps_.push_back(step_request);
    players_[server_player.id] = server_player;
    return server_player.id;
  }

  std::vector<SimulationStepRequest>
  SyncPlayer(s32 player_id)
  {
    std::vector<SimulationStepRequest> steps;
    ServerPlayer& player = players_[player_id];
    if (player.sequence_number >= steps.size() - 1) {
      return steps;
    }
    for (int i = player.sequence_number; i < steps_.size(); ++i) {
      const SimulationStepRequest& step = steps_[i];
      // This step has already been processed on the client.
      if (step.player_id() == player_id ||
          step.player_id() == kInvalidPlayer) {
        steps.push_back(step);
      }
    }
    player.sequence_number = steps_.size();
    return steps;
  }

 private:
  std::unordered_map<u32, ServerPlayer> players_;
  std::vector<SimulationStepRequest> steps_;
  s32 auto_increment_id_ = 1;
};


template <typename T>
void
LogRequest(const T* request)
{
  printf("[REQUEST] %s:\n%s\n",
         request->GetTypeName().c_str(),
         request->DebugString().c_str());
}

// Synchronous sim server. The player will send RPCs with their action the
// server will store those for all players and send them to connected players
// when asked via the Sync call.
class SimulationServer : public fourx::proto::SimulationService::Service
{
 public:
  grpc::Status
  Step(grpc::ServerContext* context, const SimulationStepRequest* request,
       SimulationStepResponse* response) override
  {
    LogRequest(request);
    if (request->has_player_join()) {
      PlayerJoin join = request->player_join();
      if (join.name().empty()) {
        return grpc::Status(grpc::INVALID_ARGUMENT,
                            "Player name must be non empty.");
      }
      ServerPlayer* splayer = state_.FindPlayerByName(join.name());
      if (splayer) {
        return grpc::Status(grpc::INVALID_ARGUMENT,
                            "Player exists with same name.");
      }
      s32 id = state_.AddPlayer(join.name());
      response->mutable_player_join_response()->set_player_id(id);
    } else {
      state_.PushStep(*request);
    }
    return grpc::Status::OK;
  }

  grpc::Status
  Sync(grpc::ServerContext* context, const SimulationSyncRequest* request,
       SimulationSyncResponse* response) override
  {
    if (request->player_id() == kInvalidPlayer) {
      return grpc::Status(grpc::INVALID_ARGUMENT, "Invalid player id.");
    }
    if (!state_.FindPlayer(request->player_id())) {
      return grpc::Status(grpc::INVALID_ARGUMENT, "Invalid player id.");
    }
    std::vector<SimulationStepRequest> steps =
      state_.SyncPlayer(request->player_id());
    for (const auto& step : steps) {
      *response->add_steps() = step;
    }
    return grpc::Status::OK;
  }

 private:
  ServerState state_;
};

void
RunServer(const std::string& address)
{
  SimulationServer service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  printf("Server listening on %s\n", address.c_str());
  server->Wait();
}

}  // namespace fourx
