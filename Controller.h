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

    bool handleSendFileRequest(const std::string& uuid, FileInfo& file, TcpSocket*);

    bool handleSendFileDataStartRequest(const std::string& uuid, const std::string& fileuuid, const int64_t size, TcpSocket*);

    bool handleSendFileDataRequest(const std::string& uuid, const std::string& fileuuid, const int64_t size, const std::string& filedata, TcpSocket*);

    bool handleSendFileDataEndRequest(const std::string& uuid, TcpSocket*);

    bool handleReceiveFileDataStartRequest(const std::string& uuid, const std::string& fileuuid, TcpSocket*);

    bool handleReceiveFileDataRequest(const std::string& uuid, TcpSocket*);

    bool handleReceiveFileDataEndRequest(const std::string& uuid, TcpSocket*);

    bool handleClientClose(TcpSocket*);

    void serialize(std::ofstream& out);

    void deserialize(std::ifstream& in);
private:
    struct FileClientInfo {
        std::string fileuuid;
        bool isUpload;
        FileClientInfo(std::string f, bool i) : fileuuid(f), isUpload(i) {}
    };

    std::mutex mutex;
    std::map<std::string, UserInfo> globalUserInfo; // key: username
    std::map<std::string, FileInfo> globalFileInfo; // key: uuid
    std::map<TcpSocket*, std::string> globalUserClientInfo; // key: client, value: username
    std::map<TcpSocket*, FileClientInfo> globalFileClientInfo; // key: client, value: fileuuid, isUpload
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
    uint32_t len = buffer.lookAheadUInt32LE();
    return buffer.getOccupancy() < len + sizeof(uint32_t) ? false : true;
}

template<unsigned long SIZE>
bool Controller::handleEntireRequest(ReadRingBuffer<SIZE> &buffer, TcpSocket *client) {
    std::unique_lock<std::mutex> lock(mutex);
    uint32_t len = buffer.getUInt32LE();
    uint16_t headerLen = buffer.getUInt16LE();
    std::string header = buffer.getString(headerLen);
    JsonReader reader(header);
    std::string body = buffer.getString(len - sizeof(uint16_t) - headerLen);
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
        case SENDFILEOP: {
            std::string uuid = reader.getString("uuid");
            FileInfo file = reader.getClass<FileInfo>("file");
            ret = handleSendFileRequest(uuid, file, client);
            break;
        }
        case SENDFILEDATASTARTOP: {
            std::string uuid = reader.getString("uuid");
            std::string fileuuid = reader.getString("fileuuid");
            int64_t size = reader.getInt64("size");
            ret = handleSendFileDataStartRequest(uuid, fileuuid, size, client);
            break;
        }
        case SENDFILEDATAOP: {
            std::string uuid = reader.getString("uuid");
            std::string fileuuid = reader.getString("fileuuid");
            int64_t size = reader.getInt64("size");
            ret = handleSendFileDataRequest(uuid, fileuuid, size, body, client);
            break;
        }
        case SENDFILEDATAENDOP: {
            std::string uuid = reader.getString("uuid");
            ret = handleSendFileDataEndRequest(uuid, client);
            break;
        }
        case RECEIVEFILEDATASTARTOP: {
            std::string uuid = reader.getString("uuid");
            std::string fileuuid = reader.getString("fileuuid");
            ret = handleReceiveFileDataStartRequest(uuid, fileuuid, client);
            break;
        }
        case RECEIVEFILEDATAOP: {
            std::string uuid = reader.getString("uuid");
            ret = handleReceiveFileDataRequest(uuid, client);
            break;
        }
        case RECEIVEFILEDATAENDOP: {
            std::string uuid = reader.getString("uuid");
            ret = handleReceiveFileDataEndRequest(uuid, client);
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
    auto clientIter = globalUserClientInfo.find(client);
    if (clientIter != globalUserClientInfo.end()) {
        globalUserInfo.find(clientIter->second)->second.quit();
        globalUserClientInfo.erase(clientIter);
    }
    UserInfo userInfo;
    userInfo.username = username;
    userInfo.password = password;
    userInfo.login(client);
    globalUserInfo.insert(std::make_pair(username, userInfo));
    globalUserClientInfo.insert(std::make_pair(client, username));
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
    auto clientIter = globalUserClientInfo.find(client);
    if (clientIter != globalUserClientInfo.end()) {
        globalUserInfo.find(clientIter->second)->second.quit();
        globalUserClientInfo.erase(clientIter);
    }
    iter->second.login(client);
    globalUserClientInfo.insert(std::make_pair(client, username));
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
    for (const auto& m : iter->second.messages)
        messages.addClass(m);
    //iter->second.messages.clear();
    writter.addArray("messages", messages);
    JsonArrayWritter files(writter.getAllocator());
    for (const auto& f : iter->second.files)
        files.addClass(f);
    //iter->second.files.clear();
    writter.addArray("files", files);
    client->write(writter.getString());
    return true;
}

bool Controller::handleQuitRequest(const std::string& uuid, TcpSocket *client) {
    auto clientIter = globalUserClientInfo.find(client);
#ifdef DEBUG
    fprintf(stderr, "quit  username: %s\n", clientIter->second.c_str());
#endif
    globalUserInfo.find(clientIter->second)->second.quit();
    globalUserClientInfo.erase(clientIter);
    JsonWritter writter;
    writter.addMember("action", QUITOP);
    writter.addMember("uuid", uuid);
    writter.addMember("status", SUCCESS);
    client->write(writter.getString());
    return true;
}

bool Controller::handleSearchRequest(const std::string& uuid, TcpSocket* client) {
    auto clientIter = globalUserClientInfo.find(client);
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
    auto clientIter = globalUserClientInfo.find(client);
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
    auto clientIter = globalUserClientInfo.find(client);
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
        object->second.messages.push_back(message);
    }
    return true;
}

bool Controller::handleSendFileRequest(const std::string &uuid, FileInfo &file, TcpSocket *client) {
    auto clientIter = globalUserClientInfo.find(client);
#ifdef DEBUG
    fprintf(stderr, "send file  username: %s, object: %s, filename: %s, size: %d\n", clientIter->second.c_str(), file.object.c_str(), file.filename.c_str(), static_cast<int>(file.size));
#endif
    file.subject = clientIter->second;
    globalFileInfo.insert(std::make_pair(uuid, file));
    JsonWritter subjectWritter;
    subjectWritter.addMember("action", SENDFILEOP);
    subjectWritter.addMember("uuid", uuid);
    subjectWritter.addMember("status", SUCCESS);
    subjectWritter.addMember("fileuuid", file.uuid);
    client->write(subjectWritter.getString());
    return true;
}

bool Controller::handleSendFileDataStartRequest(const std::string &uuid, const std::string &fileuuid, const int64_t size, TcpSocket *client) {
    auto fileIter = globalFileInfo.find(fileuuid);
#ifdef DEBUG
    fprintf(stderr, "send file data start  filename: %s, size: %d\n", fileIter->second.filename.c_str(), static_cast<int>(size));
#endif
    globalFileClientInfo.insert(std::make_pair(client, FileClientInfo(fileuuid, true)));
    fileIter->second.fsize = 0;
    std::ofstream fout(fileuuid, std::ios::out | std::ios::binary);
    fout.close();
    JsonWritter subjectWritter;
    subjectWritter.addMember("action", SENDFILEDATASTARTOP);
    subjectWritter.addMember("uuid", uuid);
    subjectWritter.addMember("status", SUCCESS);
    client->write(subjectWritter.getString());
    return true;
}

bool Controller::handleSendFileDataRequest(const std::string &uuid, const std::string &fileuuid, const int64_t size, const std::string &filedata, TcpSocket *client) {
    auto fileIter = globalFileInfo.find(fileuuid);
#ifdef DEBUG
    fprintf(stderr, "send file data  filename: %s, size: %d\n", fileIter->second.filename.c_str(), static_cast<int>(size));
#endif
    fileIter->second.fsize = fileIter->second.fsize + size;
    std::ofstream fout(fileuuid, std::ios::out | std::ios::binary | std::ios::app);
    fout.write(filedata.c_str(), size);
    fout.close();
    JsonWritter subjectWritter;
    subjectWritter.addMember("action", SENDFILEDATAOP);
    subjectWritter.addMember("uuid", uuid);
    subjectWritter.addMember("status", SUCCESS);
    client->write(subjectWritter.getString());
    return true;
}

bool Controller::handleSendFileDataEndRequest(const std::string &uuid, TcpSocket *client) {
    auto fileClientIter = globalFileClientInfo.find(client);
    auto fileIter = globalFileInfo.find(fileClientIter->second.fileuuid);
#ifdef DEBUG
    fprintf(stderr, "send file data end  filename: %s\n", fileIter->second.filename.c_str());
#endif
    globalFileClientInfo.erase(fileClientIter);
    fileIter->second.fsize = -1;
    auto object = globalUserInfo.find(fileIter->second.object);
    if (object->second.isLogin()) {
        JsonWritter objectWritter;
        objectWritter.addMember("action", SENDFILEDATAENDOP);
        objectWritter.addMember("uuid", "message");
        objectWritter.addMember("status", SUCCESS);
        objectWritter.addClass("file", fileIter->second);
        object->second.client->write(objectWritter.getString());
    } else {
        object->second.files.push_back(fileIter->second);
    }
    client->shutdown();
    return true;
}

bool Controller::handleReceiveFileDataStartRequest(const std::string &uuid, const std::string &fileuuid, TcpSocket *client) {
    auto fileIter = globalFileInfo.find(fileuuid);
#ifdef DEBUG
    fprintf(stderr, "receive file data start  filename: %s\n", fileIter->second.filename.c_str());
#endif
    globalFileClientInfo.insert(std::make_pair(client, FileClientInfo(fileuuid, false)));
    fileIter->second.fsize = 0;
    JsonWritter subjectWritter;
    subjectWritter.addMember("action", RECEIVEFILEDATASTARTOP);
    subjectWritter.addMember("uuid", uuid);
    subjectWritter.addMember("status", SUCCESS);
    client->write(subjectWritter.getString());
    return true;
}

bool Controller::handleReceiveFileDataRequest(const std::string &uuid, TcpSocket *client) {
    auto fileClientIter = globalFileClientInfo.find(client);
    auto fileIter = globalFileInfo.find(fileClientIter->second.fileuuid);
    char tmp[FILEBLOCKSIZE] = {};
    fprintf(stderr, "fsize  %d\n", fileIter->second.fsize);
    std::ifstream fin(fileIter->second.uuid, fin.binary | fin.in);
    fin.seekg(fileIter->second.fsize);
    fin.read(tmp, FILEBLOCKSIZE);
    fin.close();
    int64_t delta = fileIter->second.fsize + FILEBLOCKSIZE > fileIter->second.size ? fileIter->second.size - fileIter->second.fsize : FILEBLOCKSIZE;
    std::string data(tmp, delta);
    fileIter->second.fsize = fileIter->second.fsize + delta;
    fprintf(stderr, "fsize  %d\n", fileIter->second.fsize);
#ifdef DEBUG
    fprintf(stderr, "receive file data  filename: %s, size: %d\n", fileIter->second.filename.c_str(), static_cast<int>(data.size()));
#endif
    JsonWritter subjectWritter;
    subjectWritter.addMember("action", RECEIVEFILEDATAOP);
    subjectWritter.addMember("uuid", uuid);
    subjectWritter.addMember("size", data.size());
    subjectWritter.addMember("status", SUCCESS);
    client->write(subjectWritter.getString(), data);
    return true;
}

bool Controller::handleReceiveFileDataEndRequest(const std::string &uuid, TcpSocket *client) {
    auto fileClientIter = globalFileClientInfo.find(client);
    auto fileIter = globalFileInfo.find(fileClientIter->second.fileuuid);
#ifdef DEBUG
    fprintf(stderr, "receive file data end  filename: %s\n", fileIter->second.filename.c_str());
#endif
    globalFileClientInfo.erase(fileClientIter);
    fileIter->second.fsize = -1;
    client->shutdown();
    return true;
}

bool Controller::handleClientClose(TcpSocket *client) {
    std::unique_lock<std::mutex> lock(mutex);
    auto userClientIter = globalUserClientInfo.find(client);
    if (userClientIter != globalUserClientInfo.end()) {
        globalUserInfo.find(userClientIter->second)->second.quit();
        globalUserClientInfo.erase(userClientIter);
    }
    return false;
}

void Controller::serialize(std::ofstream& out) {
#ifdef DEBUG
    fprintf(stderr, "controller serialize start\n");
#endif
    std::unique_lock<std::mutex> lock(mutex);
    int64_t size = globalUserInfo.size();
    ::serialize(out, size);
    for (auto& user : globalUserInfo) {
        ::serialize(out, user.first);
        user.second.serialize(out);
    }
}

void Controller::deserialize(std::ifstream& in) {
#ifdef DEBUG
    fprintf(stderr, "controller deserialize start");
#endif
    std::unique_lock<std::mutex> lock(mutex);
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
