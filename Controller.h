#ifndef SERVER_CONTROLLER_H
#define SERVER_CONTROLLER_H

#include <ctime>
#include <fstream>
#include <mutex>
#include <vector>
#include "ReadRingBuffer.h"
#include "Tcp.h"
#include "UserInfo.h"
#include "JsonWritter.h"
#include "JsonReader.h"
#include "rapidjson/reader.h"
#include "rapidjson/document.h"

#define DEBUG

#ifdef DEBUG

#include <iostream>

#endif

class Controller {
public:
    Controller();
    ~Controller();

    static std::string createUUID();
    template<unsigned long SIZE>
    static bool havaEntireRequest(const ReadRingBuffer<SIZE>&);

    template<unsigned long SIZE>
    bool handleEntireRequest(ReadRingBuffer<SIZE>&, TcpSocket*);

    bool handleRegisterRequest(const std::string& uuid, const std::string& username, const std::string& password, TcpSocket*);

    bool handleLoginRequest(const std::string& uuid, const std::string& username, const std::string& password, TcpSocket*);

    bool handleQuitRequest(const std::string& uuid, TcpSocket*);

    bool handleSearchRequest(const std::string& uuid, TcpSocket*);

    bool handleAddRequest(const std::string& uuid, const std::vector<std::string>& users, TcpSocket*);

    bool handleSendMessageRequest(const std::string& uuid, MessageInfo& message, TcpSocket*);

    bool handleClientClose(TcpSocket*);
private:
    std::mutex mutex;
    std::map<std::string, UserInfo> globalUserInfo; // key: username
    std::map<std::string, FileInfo> globalFileInfo; // key: uuid
    std::map<TcpSocket*, std::string> globalClientInfo; // key: client, value: username

    void serialize(std::ofstream& out);
    void deserialize(std::ifstream& in);
};

Controller::Controller() {
    std::ifstream in("user.db", std::ios::binary);
    if (in)
        deserialize(in);
}

Controller::~Controller() {
    std::ofstream out("user.db", std::ios::binary);
    serialize(out);
}

std::string Controller::createUUID() {
    char buffer[37];
    const char *c = "89ab";
    char *p = buffer;
    for (int n = 0; n < 16; ++n) {
        int b = rand() % 255;
        switch (n) {
            case 6:
                sprintf(p, "4%x", b % 15);
                break;
            case 8:
                sprintf(p, "%c%x", c[rand() % strlen(c)], b % 15);
                break;
            default:
                sprintf(p, "%02x", b);
                break;
        }
        p += 2;
        switch (n) {
            case 3:
            case 5:
            case 7:
            case 9:
                *p++ = '-';
                break;
        }
    }
    return std::string(buffer);
}

template<unsigned long SIZE>
bool Controller::havaEntireRequest(const ReadRingBuffer<SIZE> &buffer) {
    uint16_t len = buffer.lookAheadUInt16LE();
    return buffer.getOccupancy() < len + 2 ? false : true;
}

template<unsigned long SIZE>
bool Controller::handleEntireRequest(ReadRingBuffer<SIZE> &buffer, TcpSocket *client) {
    std::unique_lock<std::mutex> lock(mutex);
    uint16_t len = buffer.getUInt16LE();
    JsonReader reader(buffer.getString(len));
    bool ret = false;
    switch (reader.getInt64("action")) {
        case REGISTEROP: {
            std::string uuid = reader.getString("uuid");
            std::string username = reader.getString("username");
            std::string password = reader.getString("password");
            ret = handleRegisterRequest(uuid, username, password, client);
            break;
        }
        case LOGINOP: {
            std::string uuid = reader.getString("uuid");
            std::string username = reader.getString("username");
            std::string password = reader.getString("password");
            ret = handleLoginRequest(uuid, username, password, client);
            break;
        }
        case QUITOP: {
            std::string uuid = reader.getString("uuid");
            ret = handleQuitRequest(uuid, client);
            break;
        }
        case SEARCHOP: {
            std::string uuid = reader.getString("uuid");
            ret = handleSearchRequest(uuid, client);
            break;
        }
        case ADDOP: {
            std::string uuid = reader.getString("uuid");
            std::vector<std::string> users = reader.getArray<std::string>("users");
            ret = handleAddRequest(uuid, users, client);
            break;
        }
        case SENDMESSAGEOP: {
            std::string uuid = reader.getString("uuid");
            MessageInfo message = reader.getClass<MessageInfo>("message");
            ret = handleSendMessageRequest(uuid, message, client);
            break;
        }
        default:
            break;
    }
    return ret;
}

bool Controller::handleRegisterRequest(const std::string &uuid, const std::string &username, const std::string &password, TcpSocket *client) {
#ifdef DEBUG
    fprintf(stderr, "register  username: %s, password: %s\n", username.c_str(), password.c_str());
#endif
    auto iter = globalUserInfo.find(username);
    if (iter != globalUserInfo.end()) {
        JsonWritter writter;
        writter.addMember("action", REGISTEROP);
        writter.addMember("uuid", uuid);
        writter.addMember("status", USERNAMEEXIST);
        client->write(writter.getString());
        return true;
    }
    auto clientIter = globalClientInfo.find(client);
    if (clientIter != globalClientInfo.end()) {
        globalUserInfo.find(clientIter->second)->second.quit();
        globalClientInfo.erase(clientIter);
    }
    UserInfo userInfo;
    userInfo.username = username;
    userInfo.password = password;
    userInfo.login(client);
    globalUserInfo.insert(std::make_pair(username, userInfo));
    globalClientInfo.insert(std::make_pair(client, username));
    JsonWritter writter;
    writter.addMember("action", REGISTEROP);
    writter.addMember("uuid", uuid);
    writter.addMember("status", SUCCESS);
    client->write(writter.getString());
    return true;
}

bool Controller::handleLoginRequest(const std::string &uuid, const std::string &username, const std::string &password, TcpSocket *client) {
#ifdef DEBUG
    fprintf(stderr, "login  username: %s, password: %s\n", username.c_str(), password.c_str());
#endif
    auto iter = globalUserInfo.find(username);
    if (iter == globalUserInfo.end()) {
        JsonWritter writter;
        writter.addMember("action", LOGINOP);
        writter.addMember("uuid", uuid);
        writter.addMember("status", USERNAMENOTEXIST);
        client->write(writter.getString());
        return true;
    }
    if (iter->second.password != password) {
        JsonWritter writter;
        writter.addMember("action", LOGINOP);
        writter.addMember("uuid", uuid);
        writter.addMember("status", PASSWORDWRONG);
        client->write(writter.getString());
        return true;
    }
    auto clientIter = globalClientInfo.find(client);
    if (clientIter != globalClientInfo.end()) {
        globalUserInfo.find(clientIter->second)->second.quit();
        globalClientInfo.erase(clientIter);
    }
    iter->second.login(client);
    globalClientInfo.insert(std::make_pair(client, username));
    JsonWritter writter;
    writter.addMember("action", LOGINOP);
    writter.addMember("uuid", uuid);
    writter.addMember("status", SUCCESS);
    JsonArrayWritter friends(writter.getAllocator());
    for (const auto& name : iter->second.friends) {
        friends.addElement(name);
    }
    writter.addArray("friends", friends);
    JsonArrayWritter messages(writter.getAllocator());
    while (!iter->second.messages.empty()) {
        messages.addClass(iter->second.messages.front());
        iter->second.messages.pop();
    }
    writter.addArray("messages", messages);
    JsonArrayWritter files(writter.getAllocator());
    while (!iter->second.files.empty()) {
        files.addClass(iter->second.files.front());
        iter->second.files.pop();
    }
    writter.addArray("files", files);
    client->write(writter.getString());
    return true;
}

bool Controller::handleQuitRequest(const std::string& uuid, TcpSocket *client) {
    auto clientIter = globalClientInfo.find(client);
#ifdef DEBUG
    fprintf(stderr, "quit  username: %s\n", clientIter->second.c_str());
#endif
    globalUserInfo.find(clientIter->second)->second.quit();
    globalClientInfo.erase(clientIter);
    JsonWritter writter;
    writter.addMember("action", QUITOP);
    writter.addMember("uuid", uuid);
    writter.addMember("status", SUCCESS);
    client->write(writter.getString());
    return true;
}

bool Controller::handleSearchRequest(const std::string& uuid, TcpSocket* client) {
    auto clientIter = globalClientInfo.find(client);
#ifdef DEBUG
    fprintf(stderr, "search  username: %s\n", clientIter->second.c_str());
#endif
    JsonWritter writter;
    writter.addMember("action", SEARCHOP);
    writter.addMember("uuid", uuid);
    writter.addMember("status", SUCCESS);
    JsonArrayWritter array(writter.getAllocator());
    for (const auto& item : globalUserInfo) {
        JsonObjectWritter object(writter.getAllocator());
        object.addMember("username", item.first);
        array.addObject(object);
    }
    writter.addArray("users", array);
    client->write(writter.getString());
    return true;
}

bool Controller::handleAddRequest(const std::string &uuid, const std::vector<std::string> &users, TcpSocket *client) {
    auto clientIter = globalClientInfo.find(client);
#ifdef DEBUG
    fprintf(stderr, "add username: %s,", clientIter->second.c_str());
    for (const auto& item : users)
        fprintf(stderr, " username: %s", item.c_str());
    fprintf(stderr, "\n");
#endif
    auto subject = globalUserInfo.find(clientIter->second);
    for (const auto &username : users) {
        auto object = globalUserInfo.find(username);
        subject->second.friends.push_back(object->first);
        object->second.friends.push_back(subject->first);
        if (object->second.isLogin()) {
            JsonWritter objectWritter;
            objectWritter.addMember("action", ADDOP);
            objectWritter.addMember("uuid", "message");
            objectWritter.addMember("status", SUCCESS);
            objectWritter.addMember("username", subject->first);
            object->second.client->write(objectWritter.getString());
        }
    }
    JsonWritter subjectWritter;
    subjectWritter.addMember("action", ADDOP);
    subjectWritter.addMember("uuid", uuid);
    subjectWritter.addMember("status", SUCCESS);
    client->write(subjectWritter.getString());
    return true;
}

bool Controller::handleSendMessageRequest(const std::string &uuid, MessageInfo& message, TcpSocket *client) {
    auto clientIter = globalClientInfo.find(client);
#ifdef DEBUG
    fprintf(stderr, "send message  username: %s, username: %s, message: %s\n", clientIter->second.c_str(), message.username.c_str(), message.message.c_str());
#endif
    auto subject = globalUserInfo.find(clientIter->second);
    auto object = globalUserInfo.find(message.username);
    JsonWritter subjectWritter;
    subjectWritter.addMember("action", SENDMESSAGEOP);
    subjectWritter.addMember("uuid", uuid);
    subjectWritter.addMember("status", SUCCESS);
    client->write(subjectWritter.getString());
    message.username = subject->first;
    if (object->second.isLogin()) {
        JsonWritter objectWritter;
        objectWritter.addMember("action", SENDMESSAGEOP);
        objectWritter.addMember("uuid", "message");
        objectWritter.addMember("status", SUCCESS);
        objectWritter.addClass("message", message);
        object->second.client->write(objectWritter.getString());
    } else {
        object->second.messages.push(message);
    }
    return true;
}

bool Controller::handleClientClose(TcpSocket *client) {
    std::unique_lock<std::mutex> lock(mutex);
    auto clientIter = globalClientInfo.find(client);
    if (clientIter != globalClientInfo.end()) {
        globalUserInfo.find(clientIter->second)->second.quit();
        globalClientInfo.erase(clientIter);
    }
    return false;
}

void Controller::serialize(std::ofstream& out) {
    int64_t size = globalUserInfo.size();
    ::serialize(out, size);
    for (auto& user : globalUserInfo) {
        ::serialize(out, user.first);
        user.second.serialize(out);
    }
}

void Controller::deserialize(std::ifstream& in) {
    int64_t size = 0;
    ::deserialize(in, size);
    for (int i = 0; i < size; ++i) {
        std::string tmpS;
        UserInfo tmpU;
        ::deserialize(in, tmpS);
        tmpU.deserialize(in);
        globalUserInfo.emplace(tmpS, tmpU);
    }
}

#endif //SERVER_CONTROLLER_H
