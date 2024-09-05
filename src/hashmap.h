#include "fmt/format.h"
#include "flow/flow.h"
#include "flow/Platform.h"
#include "flow/DeterministicRandom.h"
#include "flow/flow.h"
#include "flow/network.h"
#include "flow/IConnection.h"
#include "fdbrpc/FlowTransport.h"
#include "fdbrpc/fdbrpc.h"
#include "flow/TLSConfig.actor.g.h"
#include "flow/TLSConfig.actor.h"
#include <functional>
#include <unordered_map>
#include <memory>
#include <iostream>

#include "flow/actorcompiler.h"

#include <string>
using std::string;

struct HashmapData
{
  std::string key;
  std::string value;
  constexpr static FileIdentifier file_identifier = 10624019;
  template <class Ar>
  void serialize(Ar &ar)
  {
    serializer(ar, key, value);
  }
};
struct HashmapOperation
{
  constexpr static FileIdentifier file_identifier = 12004156;
  HashmapData data;
  ReplyPromise<HashmapData> reply;
  template <class Ar>
  void serialize(Ar &ar)
  {
    serializer(ar, data, reply);
  }
  HashmapOperation() {}
  HashmapOperation(HashmapData data) : data(data) {};
};
struct HashmapServerInterface
{
  /* data */
  constexpr static FileIdentifier file_identifier = 3152015;
  RequestStream<struct HashmapInterfaceRequest> getInterface;
  RequestStream<HashmapOperation> put;
  RequestStream<HashmapOperation> get;
  RequestStream<HashmapOperation> deleteKey;
  template <class Ar>
  void serialize(Ar &ar)
  {
    serializer(ar, put, get, deleteKey);
    // serializer(ar);
  }
};

struct HashmapInterfaceRequest
{
  constexpr static FileIdentifier file_identifier = 12004156;
  ReplyPromise<HashmapServerInterface> reply;
  template <class Ar>
  void serialize(Ar &ar)
  {
    serializer(ar, reply);
  }
};

enum TutorialWellKnownEndpoints
{
  WLTOKEN_ECHO_SERVER = WLTOKEN_FIRST_AVAILABLE,
  WLTOKEN_COUNT_IN_TUTORIAL
};

uint64_t tokenCounter = 1;