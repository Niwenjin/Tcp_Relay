#include "TcpClient.h"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./client <connNum> <msgLen>" << std::endl;
        return 1;
    }
    int connNum = atoi(argv[1]);
    int msgLen = atoi(argv[2]);

    TcpClient client(connNum, msgLen);
    client.init();
    client.loop();

    return 0;
}