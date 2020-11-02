#pragma once

#include <queue>
#include <thread>

#include "grpcpp/channel.h"
#include "grpcpp/create_channel.h"

#include "4xsim.pb.h"

namespace fourx {

template <class T>
class Queue {
 public:
  void
  Push(const T& request)
  {
    std::lock_guard<std::mutex> lock(mutex_);
    requests_.push(request);
  }

  void
  Pop()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    requests_.pop();
  }

  T
  Front()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return requests_.front();
  }

  size_t
  Size() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return requests_.size();
  }

 private:
  std::queue<T> requests_;
  mutable std::mutex mutex_;
};

static std::thread* kClientThread;

struct Connection {
  Queue<proto::SimulationStepRequest> step_queue;
  Queue<proto::SimulationSyncRequest> sync_queue;
  Queue<proto::SimulationStepResponse> step_response_queue;
  Queue<proto::SimulationSyncResponse> sync_response_queue;
  std::shared_ptr<grpc::Channel> channel;
  std::unique_ptr<fourx::proto::SimulationService::Stub> stub;
  std::atomic<bool> run = false;
};

static Connection kConnection;

void
StartClient(const std::string& address)
{
  printf("Starting client connection %s\n", address.c_str());
  std::shared_ptr<grpc::Channel> client_channel =
      grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
  std::unique_ptr<fourx::proto::SimulationService::Stub> client_stub =
      fourx::proto::SimulationService::NewStub(client_channel);
  kConnection.run = true;
  while (kConnection.run) {
    if (kConnection.step_queue.Size() > 0) {
      proto::SimulationStepRequest request = kConnection.step_queue.Front();
      grpc::ClientContext context;
      proto::SimulationStepResponse response;
      grpc::Status status =
          client_stub->Step(&context, request, &response);
      kConnection.step_queue.Pop();
      if (!status.ok()) {
        continue;
      }
      kConnection.step_response_queue.Push(response);
    }

    if (kConnection.sync_queue.Size() > 0) {
      proto::SimulationSyncRequest request = kConnection.sync_queue.Front();
      grpc::ClientContext context;
      proto::SimulationSyncResponse response;
      grpc::Status status =
          client_stub->Sync(&context, request, &response);
      kConnection.sync_queue.Pop();
      if (!status.ok()) {
        printf("[RPC FAILURE] %s\n", status.error_message().c_str());
        continue;
      }
      kConnection.sync_response_queue.Push(response);
    }
  }
}

void
ClientStartConnection(const std::string& address)
{
  kClientThread = new std::thread(StartClient, address);
  while (!kConnection.run);
}

void
ClientStopConnection()
{
  kConnection.run = false;
  kClientThread->join();
}

void
ClientPushStepRequest(const proto::SimulationStepRequest& request)
{
  kConnection.step_queue.Push(request);
}

void
ClientPushSyncRequest(const proto::SimulationSyncRequest& request)
{
  kConnection.sync_queue.Push(request);
}

bool
ClientPopStepResponse(proto::SimulationStepResponse* response)
{
  if (!kConnection.step_response_queue.Size()) return false;
  *response = kConnection.step_response_queue.Front();
  kConnection.step_response_queue.Pop();
  return true;
}

bool
ClientPopSyncResponse(proto::SimulationSyncResponse* response)
{
  if (!kConnection.sync_response_queue.Size()) return false;
  *response = kConnection.sync_response_queue.Front();
  kConnection.sync_response_queue.Pop();
  return true;
}

}
