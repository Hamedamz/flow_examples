#include <iostream>
#include <vector>
#include "flow/flow.h"
#include "flow/network.h"
#include "flow/IConnection.h"
#include "fdbrpc/FlowTransport.h"
#include "fdbrpc/fdbrpc.h"
#include "flow/TLSConfig.actor.g.h"

ACTOR Future<Void> client(NetworkAddress serverAddress)
{
  CountingServerInterface csi;

  // Set up the server endpoints on the client side
  csi.addCount = RequestStream<int>(Endpoint({serverAddress}, UID(1, 0)));
  csi.subtractCount = RequestStream<int>(Endpoint({serverAddress}, UID(2, 0)));
  csi.getCount = RequestStream<ReplyPromise<int>>(Endpoint({serverAddress}, UID(3, 0)));

  // Send commands to the server
  csi.addCount.send(5);
  csi.subtractCount.send(2);

  // Request the current count
  ReplyPromise<int> finalCount;
  csi.getCount.send(finalCount);

  int value = wait(finalCount.getFuture());
  printf("Current count is: %d\n", value);

  return Void();
}

int main(int argc, char **argv)
{
  Error::init();
  FlowTransport::createInstance(false, 1);

  NetworkAddress serverAddress("10.10.1.1", 8080);

  client(serverAddress);

  g_network->run();
  return 0;
}