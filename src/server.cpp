#include <iostream>
#include <vector>
#include "flow/flow.h"
#include "flow/network.h"
#include "flow/IConnection.h"
#include "fdbrpc/FlowTransport.h"
#include "fdbrpc/fdbrpc.h"
#include "flow/TLSConfig.actor.g.h"

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

ACTOR Future<Void> countingServer(CountingServerInterface csi)
{
  state int count = 0;

  loop
  {
    choose
    {
      when(int addValue = waitNext(csi.addCount.getFuture()))
      {
        count += addValue;
      }
      when(int subtractValue = waitNext(csi.subtractCount.getFuture()))
      {
        count -= subtractValue;
      }
      when(ReplyPromise<int> reply = waitNext(csi.getCount.getFuture()))
      {
        reply.send(count);
      }
    }
  }
}

ACTOR Future<Void> startServer()
{
  CountingServerInterface csi;

  // Start the counting server actor
  countingServer(csi);

  // Register the CountingServerInterface for clients
  FlowTransport::transport().addEndpoint(csi.addCount.getEndpoint());
  FlowTransport::transport().addEndpoint(csi.subtractCount.getEndpoint());
  FlowTransport::transport().addEndpoint(csi.getCount.getEndpoint());

  printf("Server is running and waiting for clients...\n");

  // Keep the server running
  wait(Never());
  return Void();
}

int main(int argc, char **argv)
{
  // Initialize the Flow network
  Error::init();
  FlowTransport::createInstance(false, 1);

  // Start the server
  startServer();

  // Run the network
  g_network->run();
  return 0;
}