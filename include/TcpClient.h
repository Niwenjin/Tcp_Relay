#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <vector>
#include "timer.h"

class TcpClient {
  public:
    TcpClient(int n, int len);
    ~TcpClient();

    void conn();
    void loop();

  private:
    int conn_;
    int msglen_;

    int epfd;
    std::vector<int> fds;

    std::vector<char> send_msg;
    Timer timer;
};
#endif