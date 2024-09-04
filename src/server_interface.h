#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H

#include "fdbrpc/fdbrpc.h"
#include "flow/flow.h"

// This is the server interface that the client interacts with
struct CountingServerInterface
{
  // Request streams for the server operations
  RequestStream<int> addCount;
  RequestStream<int> subtractCount;
  RequestStream<ReplyPromise<int>> getCount;

  // Serialization
  template <class Ar>
  void serialize(Ar &ar)
  {
    serializer(ar, addCount, subtractCount, getCount);
  }
};

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

#endif // SERVER_INTERFACE_H
