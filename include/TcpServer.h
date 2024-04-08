#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "mainreactor.h"

class TcpServer {
  public:
    TcpServer(int n);
    TcpServer();
    ~TcpServer();
    void run();

  private:
    MainReactor *mainreactor;
};
#endif