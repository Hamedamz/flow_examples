#include <iostream>
#include <vector>
#include "flow/flow.h"
#include "flow/network.h"
#include "flow/IConnection.h"
#include "fdbrpc/FlowTransport.h"
#include "fdbrpc/fdbrpc.h"
#include "flow/TLSConfig.actor.g.h"

// Define the server interface
struct CountingServerInterface
{
  RequestStream<int> addCount;
  RequestStream<int> subtractCount;
  RequestStream<ReplyPromise<int>> getCount;

  template <class Ar>
  void serialize(Ar &ar)
  {
    serializer(ar, addCount, subtractCount, getCount);
  }
};

// Actor to handle requests
ACTOR Future<Void> countingServer(CountingServerInterface csi)
{
  state int count = 0;

  loop choose
  {
    when(int add = waitNext(csi.addCount.getFuture()))
    {
      count += add;
    }
    when(int subtract = waitNext(csi.subtractCount.getFuture()))
    {
      count -= subtract;
    }
    when(ReplyPromise<int> reply = waitNext(csi.getCount.getFuture()))
    {
      reply.send(count);
    }
  }
}

// Server setup
ACTOR Future<Void> startServer()
{
  state CountingServerInterface csi;

  // Start the counting server actor
  countingServer(csi);

  // Keep the server running
  wait(Never());
  return Void();
}

// Entry point
int main(int argc, char **argv)
{
  // Initialize the Flow network
  Error::init();
  FlowTransport::createInstance(false, 1, 1, nullptr);

  // Start the server
  Future<Void> serverFuture = startServer();

  // Run the network
  g_network->run();
  return 0;
}