
// #include <foundationdb/fdb_c.h>

#include "hashmap.h"

// #include "flow/actorcompiler.h" // has to be last include

#include <vector>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <map>
#include <string>
#include <unordered_map>

using std::vector, std::pair, std::string;

NetworkAddress serverAddress;

ACTOR Future<Void> put(HashmapServerInterface hsi, string key, string value)
{
  HashmapData data = HashmapData{key, value};
  HashmapOperation op(data);
  HashmapData result = wait(hsi.put.getReply(op));
  return Void();
}

ACTOR Future<string> get(HashmapServerInterface hsi, string key)
{
  HashmapData data = HashmapData{key, ""};
  HashmapOperation op(data);
  hsi.get.send(op);
  HashmapData result = wait(hsi.get.getReply(op));
  return result.value;
}

ACTOR Future<Void> deleteKey(HashmapServerInterface hsi, string key)
{
  HashmapData data = HashmapData{key, ""};
  HashmapOperation op(data);
  HashmapData result = wait(hsi.deleteKey.getReply(op));
  return Void();
}

ACTOR Future<Void> run_client()
{
  try
  {
    platformInit();
    g_network = newNet2(TLSConfig(), false, true);
    FlowTransport::createInstance(1, 0, WLTOKEN_COUNT_IN_TUTORIAL);
    serverAddress = NetworkAddress::parse("172.17.0.1:7650");
    state HashmapServerInterface hsi;
    hsi.getInterface = RequestStream<HashmapInterfaceRequest>(Endpoint::wellKnown({serverAddress}, WLTOKEN_ECHO_SERVER));
    HashmapServerInterface s = wait(hsi.getInterface.getReply(HashmapInterfaceRequest()));
    hsi = s;

    state std::string op;
    state std::string key;
    state std::string value;
    while (std::cin >> op >> key)
    {
      if (op == "put")
      {
        std::cin >> value;
      }
      if (op == "put")
      {
        wait(put(hsi, key, value));
      }
      else if (op == "delete")
      {
        wait(deleteKey(hsi, key));
      }
      else if (op == "get")
      {
        std::string result = wait(get(hsi, key));
        std::cout << result << std::endl;
      }
    }
  }
  catch (Error &e)
  {
    std::cerr << e.name() << "\n"
              << e.what() << "\n";
    throw;
  }
  return Void();
}

int main()
{
  auto f = stopAfter(run_client());
  g_network->run();
}