
// #include <foundationdb/fdb_c.h>

#include "hashmap.h"

// #include "flow/actorcompiler.h" // has to be last include

#include <vector>
#include <iostream>
#include <cstdio>
#include <map>
#include <fstream>
#include <string>
#include <unordered_map>
#include <thread>

// using namespace std;
using std::vector, std::pair, std::string;

ACTOR Future<Void> serveHashmapServerInterface() {
    try {
        platformInit();
        g_network = newNet2(TLSConfig(), false, true);
        FlowTransport::createInstance(0, 0, WLTOKEN_COUNT_IN_TUTORIAL);
        auto serverAddress = NetworkAddress::parse("0.0.0.0:7650");
        auto listenError = FlowTransport::transport().bind(serverAddress, serverAddress);
        if (listenError.isError()) {
            listenError.get();
        }
        state HashmapServerInterface hsi;
        state std::unordered_map<string,string> hashmap;
        hsi.getInterface.makeWellKnownEndpoint(WLTOKEN_ECHO_SERVER,TaskPriority::DefaultEndpoint);
        loop {
            choose {
                when(HashmapInterfaceRequest x = waitNext(hsi.getInterface.getFuture())) {
                    x.reply.send(hsi);
                }
                when(HashmapOperation x = waitNext(hsi.put.getFuture())) {
                    hashmap[x.data.key] = x.data.value;
                    x.reply.send(x.data);
                }
                when(HashmapOperation x = waitNext(hsi.get.getFuture())) {
                    HashmapData result;
                    result.key = x.data.key;
                    result.value = hashmap[x.data.key];
                    x.reply.send(result);
                }
                when(HashmapOperation x = waitNext(hsi.deleteKey.getFuture())) {
                    auto result = hashmap.find(x.data.key);
                    if(result != hashmap.end()) {
                        hashmap.erase(result->first);
                    }
                    x.reply.send(x.data);
                }
            }
        }
    }  catch(Error &e) {
        std::cerr<<e.name()<<"\n"<<e.what()<<"\n";
		throw;
    }
}

int main() {
    auto f = stopAfter(serveHashmapServerInterface());
    g_network->run();
}