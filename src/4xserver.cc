#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"

#include "4xsim.grpc.pb.h"

#include "platform/type.cc"

#include <unordered_map>


ABSL_FLAG(std::string, address, "127.0.0.1:3878",
          "Address to run the server on.");

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
    for (int i = player.sequence_number; i < steps.size(); ++i) {
      const SimulationStepRequest& step = steps_[i];
      // This step has already been processed on the client.
      if (step.player_id() != player_id) continue;
      steps.push_back(step);
    }
    player.sequence_number = players_.size() - 1;
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

class SimulationServer : public fourx::proto::SimulationService::Service
{
 public:
  grpc::Status
  Step(grpc::ServerContext* context,
       const SimulationStepRequest* request,
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
    }
    return grpc::Status::OK;
  }

  grpc::Status
  Sync(grpc::ServerContext* context,
       const SimulationSyncRequest* request,
       SimulationSyncResponse* response) override
  {
    LogRequest(request);
    if (request->player_id() == kInvalidPlayer) {
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

int
main(int argc, char** argv)
{
  absl::ParseCommandLine(argc, argv);
  std::string server_address = absl::GetFlag(FLAGS_address);
  SimulationServer service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  printf("Server listening on %s\n", server_address.c_str());
  server->Wait();
  return 0;
}
