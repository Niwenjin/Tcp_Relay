#include "TcpServer.h"
#include <iostream>

using std::cerr;
using std::endl;
using std::stoi;

int main(int argc, char *argv[]) {
    TcpServer *server;
    if (argc == 1)
        server = new TcpServer();
    else if (argc == 2)
        server = new TcpServer(stoi(argv[1]));
    else {
        cerr << "Usage: " << argv[0] << "threads" << endl;
        return 1;
    }
    server->run();
    return 0;
}