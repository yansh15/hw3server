#ifndef SERVER_USERINFO_H
#define SERVER_USERINFO_H

#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include "Tcp.h"

void serialize(std::ofstream& out, const std::string& str) {
    size_t size = str.size();
    out.write((const char*)&size, sizeof(size_t));
    out.write(str.data(), size);
}

void deserialize(std::ifstream& in, std::string& str) {
    size_t size = 0;
    in.read((char*)&size, sizeof(size_t));
    char *tmp = new char[size];
    in.read(tmp, size);
    str = std::string(tmp);
    delete tmp;
}

void serialize(std::ofstream& out, const int64_t& i) {
    out.write((const char*)&i, sizeof(int64_t));
}

void deserialize(std::ifstream& in, int64_t& i) {
    in.read((char*)&i, sizeof(int64_t));
}

struct MessageInfo {
    std::string username;
    std::string message;
    int64_t time;

    MessageInfo();
    MessageInfo(const std::string& u, const std::string& m, int64_t t);

    void serialize(std::ofstream& out);
    void deserialize(std::ifstream& in);
};

MessageInfo::MessageInfo() : username(), message(), time(0) {}

MessageInfo::MessageInfo(const std::string& u, const std::string& m, int64_t t) : username(u), message(m), time(t) {}

void MessageInfo::serialize(std::ofstream& out) {
    ::serialize(out, username);
    ::serialize(out, message);
    ::serialize(out, time);
}

void MessageInfo::deserialize(std::ifstream& in) {
    ::deserialize(in, username);
    ::deserialize(in, message);
    ::deserialize(in, time);
}

struct FileInfo {
    std::string username;
    int64_t size;
    std::string filename;
    std::string uuid;
    int64_t time;

    FileInfo();
    FileInfo(const std::string& u, int64_t s, const std::string& f, const std::string& uu, int64_t t);

    void serialize(std::ofstream& out);
    void deserialize(std::ifstream& in);
};

FileInfo::FileInfo() : username(), size(0), filename(), uuid(), time(0) {}

FileInfo::FileInfo(const std::string& u, int64_t s, const std::string& f, const std::string& uu, int64_t t) : username(u), size(s), filename(f), uuid(uu), time(t) {}

void FileInfo::serialize(std::ofstream& out) {
    ::serialize(out, username);
    ::serialize(out, size);
    ::serialize(out, filename);
    ::serialize(out, uuid);
    ::serialize(out, time);
}

void FileInfo::deserialize(std::ifstream& in) {
    ::deserialize(in, username);
    ::deserialize(in, size);
    ::deserialize(in, filename);
    ::deserialize(in, uuid);
    ::deserialize(in, time);
}

struct UserInfo {
    std::string username;
    std::string password;
    TcpSocket* client;
    std::vector<std::string> friends;
    std::queue<MessageInfo> messages;
    std::queue<FileInfo> files;

    UserInfo();

    void serialize(std::ofstream& out);
    void deserialize(std::ifstream& in);

    void login(TcpSocket*);
    void quit();
    bool isLogin();
};

UserInfo::UserInfo() : client(nullptr) {}

void UserInfo::serialize(std::ofstream& out) {
    ::serialize(out, username);
    ::serialize(out, password);
    int64_t size = friends.size();
    ::serialize(out, size);
    for (const auto& f : friends)
        ::serialize(out, f);
    size = messages.size();
    ::serialize(out, size);
    while (!messages.empty()) {
        messages.front().serialize(out);
        messages.pop();
    }
    size = files.size();
    ::serialize(out, size);
    while (!files.empty()) {
        files.front().serialize(out);
        files.pop();
    }
}

void UserInfo::deserialize(std::ifstream& in) {
    ::deserialize(in, username);
    ::deserialize(in, password);
    int64_t size;
    ::deserialize(in, size);
    std::string tmpString;
    for (int i = 0; i < size; ++i) {
        ::deserialize(in, tmpString);
        friends.emplace_back(tmpString);
    }
    ::deserialize(in, size);
    MessageInfo tmpMessage;
    for (int i = 0; i < size; ++i) {
        tmpMessage.deserialize(in);
        messages.emplace(tmpMessage);
    }
    ::deserialize(in, size);
    FileInfo tmpFile;
    for (int i = 0; i < size; ++i) {
        tmpFile.deserialize(in);
        files.emplace(tmpFile);
    }
}

void UserInfo::login(TcpSocket* c) {
    client = c;
}

void UserInfo::quit() {
    client = nullptr;
}

bool UserInfo::isLogin() {
    return client != nullptr;
}

#endif //SERVER_USERINFO_H
