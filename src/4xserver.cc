#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "platform/type.cc"

#include "4x/server.cc"


ABSL_FLAG(std::string, address, "127.0.0.1:3878",
          "Address to run the server on.");


int
main(int argc, char** argv)
{
  absl::ParseCommandLine(argc, argv);
  std::string server_address = absl::GetFlag(FLAGS_address);
  fourx::RunServer(absl::GetFlag(FLAGS_address));
  return 0;
}
