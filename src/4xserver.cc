#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "grpcpp/server.h"
#include "grpcpp/server_builder.h"

#include "proto/4xsim.grpc.pb.h"

ABSL_FLAG(std::string, address, "127.0.0.1:5327",
          "Address to run the server on.");

class SimulationServer : public SimulationService::Service
{
 public:
  grpc::Status
  Step(grpc::ServerContext* context,
       const SimulationStepRequest* request,
       SimulationStepResponse* response) override
  {
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
