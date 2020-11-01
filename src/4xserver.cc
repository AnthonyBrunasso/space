#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"

#include "4xsim.grpc.pb.h"


ABSL_FLAG(std::string, address, "127.0.0.1:3878",
          "Address to run the server on.");

std::vector<std::string> kPlayers;

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
  PlayerJoin(grpc::ServerContext* context,
             const fourx::proto::PlayerJoinRequest* request,
             fourx::proto::PlayerJoinResponse* response) override
  {
    LogRequest(request);
    for (const auto& player_name : kPlayers) {
      if (request->name() == player_name) {
        return grpc::Status(
            grpc::INVALID_ARGUMENT,
            "Player with same name already exists in game.");
      }
    }
    kPlayers.push_back(request->name());
    for (const auto& player_name : kPlayers) {
      response->add_players()->set_name(player_name);
    }
    return grpc::Status::OK;
  }
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
