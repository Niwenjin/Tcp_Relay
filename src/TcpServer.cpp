#include "TcpServer.h"

TcpServer::TcpServer(int n) {
    mainreactor = new MainReactor(n);
}

TcpServer::TcpServer() {
    mainreactor = new MainReactor(1);
}

TcpServer::~TcpServer() {
    delete mainreactor;
}

void TcpServer::run() {
    mainreactor->run();
}