#include "flow/flow.h"
#include "flow/TCPListener.h"
#include "flow/TCP.h"
#include "flow/network.h"
#include "flow/IConnection.h"
#include "fdbrpc/FlowTransport.h"
#include "fdbrpc/fdbrpc.h"
#include "flow/TLSConfig.actor.g.h"

struct CountingServerInterface
{
  PromiseStream<int> addCount;
  PromiseStream<int> subtractCount;
  PromiseStream<Promise<int>> getCount;

  template <class Ar>
  void serialize(Ar &ar)
  {
    serializer(ar, addCount, subtractCount, getCount);
  }
};

// Define the actor that handles counting requests
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
      when(Promise<int> reply = waitNext(csi.getCount.getFuture()))
      {
        reply.send(count);
      }
    }
  }
}

// Server setup to listen for connections and provide the CountingServerInterface
ACTOR Future<Void> startServer(uint16_t port)
{
  state CountingServerInterface csi;
  state Reference<IListener> listener = wait(TCPListener::create(port));
  printf("Server is listening on port %d...\n", port);

  // Start the counting server actor
  countingServer(csi);

  loop
  {
    // Accept a new connection from a client
    state Reference<IConnection> conn = wait(listener->accept());
    printf("New connection accepted.\n");

    // Serialize the CountingServerInterface to send it to the client
    wait(FlowTransport::transport().sendUnreliable(conn, csi));
  }
}

// Entry point for the server
int main(int argc, char **argv)
{
  // Run the server on port 8080
  startServer(8080);
  return 0;
}