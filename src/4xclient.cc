#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "grpcpp/channel.h"
#include "grpcpp/create_channel.h"

#include "4xsim.grpc.pb.h"

#include <memory>

ABSL_FLAG(std::string, address, "127.0.0.1:3878",
          "Server address to connect to.");
  
int
main(int argc, char** argv)
{
  absl::ParseCommandLine(argc, argv);

  std::string server_address = absl::GetFlag(FLAGS_address);
  std::shared_ptr<grpc::Channel> client_channel =
      grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  std::unique_ptr<SimulationService::Stub> client_stub =
      SimulationService::NewStub(client_channel);

  SimulationStepRequest request;
  request.set_foo("test");
  SimulationStepResponse response;
  grpc::ClientContext context;
  grpc::Status status = client_stub->Step(&context, request, &response);
  if (!status.ok()) {
    printf("RPC failed with status %s\n", status.error_message().c_str());
    return 1;
  }

  printf("Server responded with %s\n", response.DebugString().c_str());

  return 0;
}
