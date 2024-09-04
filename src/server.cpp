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

// Define a class to handle requests
class CountingServer : public NetworkMessageReceiver
{
public:
  CountingServer() : count(0) {}

  void receive(RequestStream<int>::RequestT req)
  {
    if (req.isFor(csi.addCount.getEndpoint()))
    {
      count += req;
    }
    else if (req.isFor(csi.subtractCount.getEndpoint()))
    {
      count -= req;
    }
  }

  void receive(RequestStream<ReplyPromise<int>>::RequestT req)
  {
    if (req.isFor(csi.getCount.getEndpoint()))
    {
      req.reply.send(count);
    }
  }

  void setupEndpoints()
  {
    FlowTransport::transport().addEndpoint(csi.addCount.getEndpoint(), this, TaskPriority::DefaultOnRunLoop);
    FlowTransport::transport().addEndpoint(csi.subtractCount.getEndpoint(), this, TaskPriority::DefaultOnRunLoop);
    FlowTransport::transport().addEndpoint(csi.getCount.getEndpoint(), this, TaskPriority::DefaultOnRunLoop);
  }

  CountingServerInterface csi;

private:
  int count;
};

// Server setup to listen for connections and provide the CountingServerInterface
ACTOR Future<Void> startServer()
{
  state CountingServer server;
  server.setupEndpoints();

  printf("Server is running and waiting for clients...\n");

  // Keep the server running
  wait(Never());
  return Void();
}

// Entry point for the server
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