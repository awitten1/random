

#include "server.h"

#include <string>
#include <grpcpp/grpcpp.h>

void run_server() {
  std::string server_address("unix-abstract:grpc%00abstract");
  csi::IdentityImpl identity_service;
  csi::NodeImpl node_service;
  // grpc::EnableDefaultHealthCheckService(true);
  // grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&identity_service);
  builder.RegisterService(&node_service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << " ... ";
  server->Wait();
}


int main() {
  return 0;
}