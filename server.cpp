// #include "TcpServer.h"
#include "RelayServer.h"
#include <iostream>

using std::cerr;
using std::endl;
using std::stoi;

int main(int argc, char *argv[]) {
    RelayServer *server;
    if (argc == 1)
        server = new RelayServer();
    else if (argc == 2)
        server = new RelayServer(stoi(argv[1]));
    else {
        cerr << "Usage: " << argv[0] << "threads" << endl;
        return 1;
    }
    server->loop();
    delete server;
    return 0;
}