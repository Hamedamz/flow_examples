#include <iostream>
#include <vector>
#include "flow/flow.h"
#include "flow/network.h"
#include "flow/IConnection.h"
#include "fdbrpc/FlowTransport.h"
#include "fdbrpc/fdbrpc.h"
#include "flow/TLSConfig.actor.g.h"
#include "flow/genericactors.actor.h"
#include "flow/actorcompiler.h" // This must be the last include

// Include the server interface definition
#include "server_interface.h" // Ensure that the CountingServerInterface is declared here

// Client actor function
ACTOR Future<Void> client(NetworkAddress serverAddress)
{
  state CountingServerInterface csi;

  // Initialize the server interface
  csi.addCount = RequestStream<IntWrapper>(Endpoint({serverAddress}, UID(1, 0)));
  csi.subtractCount = RequestStream<IntWrapper>(Endpoint({serverAddress}, UID(2, 0)));
  csi.getCount = RequestStream<ReplyPromise<IntWrapper>>(Endpoint({serverAddress}, UID(3, 0)));

  // Example of sending requests to the server
  wait(csi.addCount.send(IntWrapper{10}));
  wait(csi.subtractCount.send(IntWrapper{5}));

  ReplyPromise<IntWrapper> reply;
  IntWrapper currentCount = wait(csi.getCount.getReply(reply));

  TraceEvent("ClientCurrentCount").detail("Count", currentCount.value);

  return Void();
}

int main(int argc, char **argv)
{
  // Initialize the Flow network (client mode)
  FlowTransport::createInstance(true, 1, 1, nullptr);

  // Parse the server address
  NetworkAddress serverAddress = NetworkAddress::parse("10.10.1.1", 8080);

  // Run the client actor
  Future<Void> clientFuture = client(serverAddress);

  // Run the network
  g_network->run();

  return 0;
}