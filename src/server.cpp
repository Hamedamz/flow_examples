#include <iostream>
#include <vector>
#include "flow/flow.h"
#include "flow/network.h"
#include "flow/IConnection.h"
#include "fdbrpc/FlowTransport.h"
#include "fdbrpc/fdbrpc.h"
#include "flow/TLSConfig.actor.g.h"
#include "flow/flow.h"
#include "fdbrpc/FlowTransport.h"

// Define the wrapped integer with serialization support
struct IntWrapper
{
  int value;

  template <class Ar>
  void serialize(Ar &ar)
  {
    serializer(ar, value);
  }
};

template <>
struct FileIdentifierFor<IntWrapper>
{
  static constexpr uint8_t value = 1;
};

// Define the server interface using the wrapped type
struct CountingServerInterface
{
  RequestStream<IntWrapper> addCount;
  RequestStream<IntWrapper> subtractCount;
  RequestStream<ReplyPromise<IntWrapper>> getCount;

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
    when(IntWrapper add = waitNext(csi.addCount.getFuture()))
    {
      count += add.value;
    }
    when(IntWrapper subtract = waitNext(csi.subtractCount.getFuture()))
    {
      count -= subtract.value;
    }
    when(ReplyPromise<IntWrapper> reply = waitNext(csi.getCount.getFuture()))
    {
      reply.send(IntWrapper{count});
    }
  }
}

// Server setup
ACTOR Future<Void> startServer()
{
  state CountingServerInterface csi;

  // Start the counting server actor and ensure it's waited upon
  wait(countingServer(csi));

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