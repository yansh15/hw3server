#ifndef SERVER_TCP_H
#define SERVER_TCP_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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
    ssize_t write(const std::string& header);
    ssize_t write(const std::string& header, const std::string& body);
    bool shutdown();
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
    unsigned long len = ::read(socketfd, buf, n);
    std::string ret(buf, len);
    delete[] buf;
    return ret;
}

ssize_t TcpSocket::write(const std::string& header) {
    uint32_t len = sizeof(uint16_t) + header.size();
    uint16_t headerLen = header.size();
    ::write(socketfd, (void*)&len, sizeof(uint32_t));
    ::write(socketfd, (void*)&headerLen, sizeof(uint16_t));
    return ::write(socketfd, header.data(), header.size()) + sizeof(uint16_t) + sizeof(uint32_t);
}

ssize_t TcpSocket::write(const std::string& header, const std::string& body) {
    uint32_t len = sizeof(uint16_t) + header.size() + body.size();
    uint16_t headerLen = header.size();
    ::write(socketfd, (void*)&len, sizeof(uint32_t));
    ::write(socketfd, (void*)&headerLen, sizeof(uint16_t));
    ssize_t ret1 = ::write(socketfd, header.data(), header.size());
    ssize_t ret2 = ::write(socketfd, body.data(), body.size());
    return ret1 + ret2 + sizeof(uint16_t) + sizeof(uint32_t);
}

bool TcpSocket::shutdown() {
    if (socketfd < 0)
        return true;
    return ::shutdown(socketfd, 2) != -1;
}

bool TcpSocket::close() {
    if (socketfd < 0)
        return true;
    return ::close(socketfd) != -1;
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
    socketfd = ::socket(AF_INET, SOCK_STREAM, 0);
    return socketfd >= 0;
}

bool TcpServer::bind() {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return ::bind(socketfd, (sockaddr *)&addr, sizeof(addr)) != -1;
}

bool TcpServer::listen() {
    return ::listen(socketfd, maxClientNum) != -1;
}

bool TcpServer::close() {
    if (socketfd < 0)
        return true;
    return ::close(socketfd) != -1;
}

TcpSocket *TcpServer::accept() {
    sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    int clientfd = ::accept(socketfd, (sockaddr*)&clientAddr, &len);
    if (clientfd < 0)
        return nullptr;
    char ip[20];
    inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, ip, sizeof(ip));
    TcpSocket *client = new TcpSocket(clientfd, ip, ntohs(clientAddr.sin_port));
    clients.push_back(client);
    return client;
}

#endif //SERVER_TCP_H
