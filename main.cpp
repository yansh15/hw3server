#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <set>

#include "Constant.h"
#include "Controller.h"
#include "Timer.h"

using namespace std;

int main() {
    fprintf(stderr, "Start server.\n");

    TcpServer tcpServer(INADDR_ANY, PORT, MAXCLIENTNUM);
    Controller controller;

    if (!tcpServer.open()) {
        fprintf(stderr, "Error: can't open tcp server.\n");
        exit(1);
    }
    fprintf(stderr, "Open server socket successfully.\n");

    if (!tcpServer.bind()) {
        fprintf(stderr, "Error: can't bind server socket with address.\n");
        exit(1);
    }
    fprintf(stderr, "Bind server socket with address successfully.\n");

    if (!tcpServer.listen()) {
        fprintf(stderr, "Error: can't listen the address.\n");
        exit(1);
    }
    fprintf(stderr, "Listen the port: %u successfully.\n", tcpServer.getPort());

    thread timer([&controller]() {
        Timer timer;
        timer.start(10000, [&controller]() {
            ofstream fout("user.db", ios::binary);
            controller.serialize(fout);
            fout.close();
        });
    });
    timer.detach();

    while (true) {
        TcpSocket *client = tcpServer.accept();
        if (client == nullptr) {
            fprintf(stderr, "Error: can't accept the connect request.\n");
            exit(1);
        }

        thread t([client, &controller]() {
            fprintf(stderr, "Connect to client %s:%u, client socket fd: %d\n", client->getIP(), client->getport(), client->getSocketFd());
            ReadRingBuffer<131072> buffer;
            while (true) {
                if (Controller::havaEntireRequest(buffer)) {
                    controller.handleEntireRequest(buffer, client);
                } else {
                    string data = client->read(buffer.getCapacity());
                    if (data.empty()) {
                        controller.handleClientClose(client);
                        fprintf(stderr, "Client disconnected.\n");
                        close(client->getSocketFd());
                        break;
                    }
                    buffer.putString(data);
                }
            }
        });
        t.detach();
    }

    return 0;
}
