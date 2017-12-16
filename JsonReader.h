#ifndef SERVER_JSONREADER_H
#define SERVER_JSONREADER_H

#include <vector>
#include "rapidjson/document.h"

class JsonReader;

class JsonReader {
public:
    JsonReader();
    explicit JsonReader(const std::string& str);

    void parse(const std::string& str);

    std::string getString(const char* key);
    int64_t getInt64(const char* key);
    template <typename T>
    std::vector<T> getArray(const char* key);
    template <typename T>
    T getClass(const char* key);
private:
    rapidjson::Document document;
};

JsonReader::JsonReader() {}

JsonReader::JsonReader(const std::string& str) {
    parse(str);
}

void JsonReader::parse(const std::string& str) {
    document.Parse(str.c_str());
}

std::string JsonReader::getString(const char *key) {
    return std::string(document[key].GetString());
}

int64_t JsonReader::getInt64(const char *key) {
    return document[key].GetInt64();
}

template <>
std::vector<std::string> JsonReader::getArray<std::string>(const char *key) {
    rapidjson::Value& value = document[key];
    std::vector<std::string> ret;
    for (const rapidjson::Value& item : value.GetArray())
        ret.push_back(std::string(item.GetString()));
    return ret;
}

template <>
std::vector<int64_t> JsonReader::getArray<int64_t>(const char *key) {
    rapidjson::Value& value = document[key];
    std::vector<int64_t > ret;
    for (const rapidjson::Value& item : value.GetArray())
        ret.push_back(item.GetInt64());
    return ret;
}

template <>
MessageInfo JsonReader::getClass<MessageInfo>(const char *key) {
    rapidjson::Value& value = document[key];
    return MessageInfo(value["username"].GetString(), value["message"].GetString(), value["time"].GetInt64());
}

#endif //SERVER_JSONREADER_H
