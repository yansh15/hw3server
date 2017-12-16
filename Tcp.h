#ifndef SERVER_TCP_H
#define SERVER_TCP_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace SocketAPI {
    int cSocket(int domain, int type, int protocol) {
        return socket(domain, type, protocol);
    }
    bool cBind(int fd, const sockaddr * addr, socklen_t len) {
        return bind(fd, addr, len) != -1;
    }
    bool cListen(int fd, int n) {
        return listen(fd, n) != -1;
    }
    bool cClose(int fd) {
        return close(fd) != -1;
    }
    int cAccept (int fd, sockaddr *addr, socklen_t *len) {
        return accept(fd, addr, len);
    }
    ssize_t cRead (int fd, void *buf, size_t nbytes) {
        return read(fd, buf, nbytes);
    }
    ssize_t cWrite (int fd, const void *buf, size_t n) {
        return write(fd, buf, n);
    }
}

class TcpSocket {
public:
    TcpSocket() = delete;
    TcpSocket(int fd, char* i, uint16_t p);
    ~TcpSocket();

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    TcpSocket(TcpSocket&&) noexcept;
    TcpSocket& operator=(TcpSocket&&) noexcept;

    const char* getIP() const;
    void setIP(const char*);
    const uint16_t getport() const;
    void setPort(uint16_t);
    int getSocketFd() const;

    std::string read(unsigned long);
    ssize_t write(const std::string&);
    bool close();
private:
    int socketfd;
    char ip[20];
    uint16_t port;
};

TcpSocket::TcpSocket(int fd, char *i, uint16_t p) : socketfd(fd), port(p) {
    strcpy(ip, i);
}

TcpSocket::~TcpSocket() {
    close();
}

TcpSocket::TcpSocket(TcpSocket&& r) noexcept : socketfd(r.socketfd), port(r.port) {
    strcpy(ip, r.ip);
    r.socketfd = -1;
}

TcpSocket& TcpSocket::operator=(TcpSocket&& r) noexcept {
    close();
    socketfd = r.socketfd;
    strcpy(ip, r.ip);
    port = r.port;
    r.socketfd = -1;
    return *this;
}

const char* TcpSocket::getIP() const {
    return ip;
}

void TcpSocket::setIP(const char* i) {
    strcpy(ip, i);
}

const uint16_t TcpSocket::getport() const {
    return port;
}

void TcpSocket::setPort(uint16_t p) {
    port = p;
}

int TcpSocket::getSocketFd() const {
    return socketfd;
}

std::string TcpSocket::read(unsigned long n) {
    char *buf = new char[n];
    unsigned long len = SocketAPI::cRead(socketfd, buf, n);
    return std::string(buf, len);
}

ssize_t TcpSocket::write(const std::string& str) {
    uint16_t len = str.size();
    SocketAPI::cWrite(socketfd, (void*)&len, sizeof(uint16_t));
    return SocketAPI::cWrite(socketfd, str.data(), str.size()) + sizeof(uint16_t);
}

bool TcpSocket::close() {
    if (socketfd < 0)
        return true;
    return SocketAPI::cClose(socketfd);
}

class TcpServer {
public:
    TcpServer();
    TcpServer(uint32_t host, uint16_t port, int maxClientNum);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer(TcpServer&&) = delete;

    TcpServer& operator=(const TcpServer&) = delete;
    TcpServer& operator=(TcpServer&&) = delete;

    void setPort(uint16_t port);
    uint16_t getPort();
    void setHost(uint32_t host);
    uint32_t getHost();
    void setMaxClientNum(int maxClientNum);
    int getMaxClientNum();
    bool open();
    bool bind();
    bool listen();
    bool close();
    TcpSocket* accept();
private:
    int socketfd;
    uint16_t port;
    u_int32_t host;
    int maxClientNum;
    std::vector<TcpSocket*> clients;
};

TcpServer::TcpServer() : socketfd(-1), port(0), host(0), maxClientNum(0) {}

TcpServer::TcpServer(uint32_t host, uint16_t port, int maxClientNum) : socketfd(-1), port(port), host(host), maxClientNum(maxClientNum) {}

TcpServer::~TcpServer() {
    for (auto& client : clients)
        delete client;
    close();
}

void TcpServer::setPort(uint16_t port) {
    this->port = port;
}

uint16_t TcpServer::getPort() {
    return port;
}

void TcpServer::setHost(uint32_t host) {
    this->host = host;
}

uint32_t TcpServer::getHost() {
    return host;
}

void TcpServer::setMaxClientNum(int maxClientNum) {
    this->maxClientNum = maxClientNum;
}

int TcpServer::getMaxClientNum() {
    return maxClientNum;
}

bool TcpServer::open() {
    socketfd = SocketAPI::cSocket(AF_INET, SOCK_STREAM, 0);
    return socketfd >= 0;
}

bool TcpServer::bind() {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return SocketAPI::cBind(socketfd, (sockaddr *)&addr, sizeof(addr));
}

bool TcpServer::listen() {
    return SocketAPI::cListen(socketfd, maxClientNum);
}

bool TcpServer::close() {
    if (socketfd < 0)
        return true;
    return SocketAPI::cClose(socketfd);
}

TcpSocket *TcpServer::accept() {
    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int clientfd = SocketAPI::cAccept(socketfd, (sockaddr*)&clientAddr, &len);
    if (clientfd < 0)
        return nullptr;
    char ip[20];
    inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, ip, sizeof(ip));
    TcpSocket *client = new TcpSocket(clientfd, ip, ntohs(clientAddr.sin_port));
    clients.push_back(client);
    return client;
}

#endif //SERVER_TCP_H
