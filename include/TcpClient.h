#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <vector>

class TcpClient {
  public:
    TcpClient(int n, int len);
    ~TcpClient();

    void init();
    void loop();

  private:
    int conn_;
    int msglen_;

    int epfd;
    std::vector<int> fds;
    void connect_n();

    std::vector<char> send_msg;
    int send_turn;
};
#endif