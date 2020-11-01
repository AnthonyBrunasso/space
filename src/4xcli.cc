#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "grpcpp/channel.h"
#include "grpcpp/create_channel.h"

#include "4xsim.grpc.pb.h"

#include <memory>

ABSL_FLAG(std::string, address, "127.0.0.1:3878",
          "Server address to connect to.");

ABSL_FLAG(bool, player_join, false,
          "Sends PlayerJoin simulation step with 'player_name' param");

ABSL_FLAG(std::string, player_name, "", "");

ABSL_FLAG(bool, sync, false,
          "Sends SimulationSyncRequest with 'player_id' param");

ABSL_FLAG(int, player_id, 0, "Player id to send sync / step request");

using fourx::proto::SimulationStepRequest;
using fourx::proto::SimulationStepResponse;
using fourx::proto::SimulationSyncRequest;
using fourx::proto::SimulationSyncResponse;

using fourx::proto::PlayerJoin;

void
Sync(fourx::proto::SimulationService::Stub* stub)
{
  int player_id = absl::GetFlag(FLAGS_player_id);
  if (!player_id) {
    printf("Must provide --player_id\n");
    return;
  }
  SimulationSyncRequest sync;
  sync.set_player_id(player_id);
  grpc::ClientContext context;
  SimulationSyncResponse response;
  grpc::Status status = stub->Sync(&context, sync, &response);
  if (!status.ok()) {
    printf("FAILURE: %s\n", status.error_message().c_str());
    return;
  }
  printf("Server responded with %s\n", response.DebugString().c_str());

}

void
JoinPlayer(fourx::proto::SimulationService::Stub* stub)
{
  const std::string& name = absl::GetFlag(FLAGS_player_name);
  if (name.empty()) {
    printf("Must provide --player_name\n");
    return;
  }
  PlayerJoin join;
  join.set_name(name);
  SimulationStepRequest step_request;
  *step_request.mutable_player_join() = join;
  grpc::ClientContext context;
  SimulationStepResponse response;
  grpc::Status status = stub->Step(&context, step_request, &response);
  if (!status.ok()) {
    printf("FAILURE: %s\n", status.error_message().c_str());
    return;
  }
  printf("Server responded with %s\n", response.DebugString().c_str());
}

int
main(int argc, char** argv)
{
  absl::ParseCommandLine(argc, argv);
  std::string server_address = absl::GetFlag(FLAGS_address);
  std::shared_ptr<grpc::Channel> client_channel =
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  std::unique_ptr<fourx::proto::SimulationService::Stub> client_stub =
      fourx::proto::SimulationService::NewStub(client_channel);
  if (absl::GetFlag(FLAGS_player_join)) {
    JoinPlayer(client_stub.get());
  }
  return 0;
}
