#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "grpcpp/channel.h"
#include "grpcpp/create_channel.h"

#include "4xsim.grpc.pb.h"

#include <memory>

ABSL_FLAG(std::string, address, "127.0.0.1:3878",
          "Server address to connect to.");

ABSL_FLAG(bool, join_player, false,
          "Sends PlayerJoinRequest with 'player_name' param");

ABSL_FLAG(std::string, player_name, "", "");

void
JoinPlayer(fourx::proto::SimulationService::Stub* stub)
{
  const std::string& name = absl::GetFlag(FLAGS_player_name);
  if (name.empty()) {
    printf("Must provide --player_name\n");
    return;
  }
  fourx::proto::PlayerJoinRequest request;
  request.set_name(name);
  fourx::proto::PlayerJoinResponse response;
  grpc::ClientContext context;
  grpc::Status status = stub->PlayerJoin(&context, request, &response);
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
  if (absl::GetFlag(FLAGS_join_player)) {
    JoinPlayer(client_stub.get());
  }
  return 0;
}
