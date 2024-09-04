#include <iostream>
#include <vector>
#include "flow/flow.h"
#include "flow/network.h"
#include "flow/IConnection.h"
#include "fdbrpc/FlowTransport.h"
#include "fdbrpc/fdbrpc.h"
#include "flow/TLSConfig.actor.g.h"

// Client-side function to interact with the server
ACTOR Future<Void> client(std::string address, uint16_t port)
{
  // Connect to the server
  state Reference<IConnection> conn = wait(TCP::connect(address, port));
  printf("Connected to server at %s:%d\n", address.c_str(), port);

  // Deserialize the CountingServerInterface received from the server
  state CountingServerInterface csi;
  wait(FlowTransport::transport().receiveUnreliable(conn, csi));

  // Send commands to the server
  csi.addCount.send(5);
  csi.subtractCount.send(2);

  // Request the current count
  Promise<int> finalCount;
  csi.getCount.send(finalCount);

  int value = wait(finalCount.getFuture());
  printf("Current count is: %d\n", value);

  return Void();
}

// Entry point for the client
int main(int argc, char **argv)
{
  // Connect to the server at 192.168.1.1 on port 8080
  client("10.0.0.1", 8080);
  return 0;
}