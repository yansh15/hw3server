#ifndef SERVER_JSONWRITTER_H
#define SERVER_JSONWRITTER_H

#include <string>
#include "UserInfo.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

class JsonWritter;

class JsonArrayWritter;
class JsonObjectWritter;
template<typename T>
class JsonClassWritter;

class JsonArrayWritter {
    friend class JsonObjectWritter;
    friend class JsonWritter;
public:
    explicit JsonArrayWritter(rapidjson::Document::AllocatorType&);

    void addElement(int64_t value);
    void addElement(const std::string &value);
    void addArray(JsonArrayWritter &value);
    void addObject(JsonObjectWritter &value);
    template<typename T>
    void addClass(const T &value);

private:
    rapidjson::Document::AllocatorType& allocator;
    rapidjson::Value array;
};

class JsonObjectWritter {
    friend class JsonArrayWritter;
    friend class JsonWritter;
public:
    explicit JsonObjectWritter(rapidjson::Document::AllocatorType&);

    void addMember(const char *key, int64_t value);
    void addMember(const char *key, const std::string &value);
    void addArray(const char *key, JsonArrayWritter &value);
    void addObject(const char *key, JsonObjectWritter &value);
    template<typename T>
    void addClass(const char *key, const T &value);
private:
    rapidjson::Document::AllocatorType& allocator;
    rapidjson::Value object;
};

template<typename T>
class JsonClassWritter {
    friend class JsonWritter;
    friend class JsonArrayWritter;
    friend class JsonObjectWritter;
public:
    JsonClassWritter(rapidjson::Document::AllocatorType&, const T&) {};
private:
    rapidjson::Value value;
};

template<>
class JsonClassWritter<MessageInfo> {
    friend class JsonWritter;
    friend class JsonArrayWritter;
    friend class JsonObjectWritter;
public:
    JsonClassWritter(rapidjson::Document::AllocatorType& a, const MessageInfo& message) : value(rapidjson::kObjectType) {
        rapidjson::Value v1;
        v1.SetString(message.username.c_str(), message.username.size(), a);
        value.AddMember(rapidjson::Value("username"), v1, a);
        rapidjson::Value v2;
        v2.SetString(message.message.c_str(), message.message.size(), a);
        value.AddMember(rapidjson::Value("message"), v2, a);
        rapidjson::Value v3;
        v3.SetInt64(message.time);
        value.AddMember(rapidjson::Value("time"), v3, a);
    }
private:
    rapidjson::Value value;
};

template<>
class JsonClassWritter<FileInfo> {
    friend class JsonWritter;
    friend class JsonArrayWritter;
    friend class JsonObjectWritter;
public:
    JsonClassWritter(rapidjson::Document::AllocatorType& a, const FileInfo& file) : value(rapidjson::kObjectType) {
        rapidjson::Value v1;
        v1.SetString(file.username.c_str(), file.username.size(), a);
        value.AddMember(rapidjson::Value("username"), v1, a);
        rapidjson::Value v2;
        v2.SetInt64(file.size);
        value.AddMember(rapidjson::Value("size"), v2, a);
        rapidjson::Value v3;
        v3.SetString(file.filename.c_str(), file.filename.size(), a);
        value.AddMember(rapidjson::Value("filename"), v3, a);
        rapidjson::Value v4;
        v4.SetString(file.uuid.c_str(), file.uuid.size(), a);
        value.AddMember(rapidjson::Value("uuid"), v4, a);
        rapidjson::Value v5;
        v5.SetInt64(file.time);
        value.AddMember(rapidjson::Value("time"), v5, a);
    }
private:
    rapidjson::Value value;
};

class JsonWritter {
public:
    JsonWritter();

    rapidjson::Document::AllocatorType& getAllocator();

    void addMember(const char *key, int64_t value);
    void addMember(const char *key, const std::string &value);
    void addArray(const char *key, JsonArrayWritter &value);
    void addObject(const char *key, JsonObjectWritter &value);
    template<typename T>
    void addClass(const char *key, const T &value);

    std::string getString();
private:
    rapidjson::Document document;
};

JsonArrayWritter::JsonArrayWritter(rapidjson::Document::AllocatorType &a) : allocator(a), array(rapidjson::kArrayType) {}

void JsonArrayWritter::addElement(int64_t value) {
    array.PushBack(rapidjson::Value(value), allocator);
}

void JsonArrayWritter::addElement(const std::string &value) {
    rapidjson::Value e;
    e.SetString(value.c_str(), value.size(), allocator);
    array.PushBack(e, allocator);
}

void JsonArrayWritter::addArray(JsonArrayWritter &value) {
    array.PushBack(value.array, allocator);
}

void JsonArrayWritter::addObject(JsonObjectWritter &value) {
    array.PushBack(value.object, allocator);
}

template<typename T>
void JsonArrayWritter::addClass(const T &value) {
    array.PushBack(JsonClassWritter<T>(allocator, value).value, allocator);
}

JsonObjectWritter::JsonObjectWritter(rapidjson::Document::AllocatorType &a) : allocator(a), object(rapidjson::kObjectType) {}

void JsonObjectWritter::addMember(const char *key, int64_t value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), allocator);
    object.AddMember(k, rapidjson::Value(value), allocator);
}

void JsonObjectWritter::addMember(const char *key, const std::string &value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), allocator);
    rapidjson::Value v;
    v.SetString(value.c_str(), value.size(), allocator);
    object.AddMember(k, v, allocator);
}

void JsonObjectWritter::addArray(const char *key, JsonArrayWritter &value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), allocator);
    object.AddMember(k, value.array, allocator);
}

void JsonObjectWritter::addObject(const char *key, JsonObjectWritter &value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), allocator);
    object.AddMember(k, value.object, allocator);
}

template<typename T>
void JsonObjectWritter::addClass(const char *key, const T &value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), allocator);
    object.AddMember(k, JsonClassWritter<T>(allocator, value).value, allocator);
}

JsonWritter::JsonWritter() : document(rapidjson::kObjectType) {}

rapidjson::Document::AllocatorType &JsonWritter::getAllocator() {
    return document.GetAllocator();
}

void JsonWritter::addMember(const char *key, int64_t value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), document.GetAllocator());
    document.AddMember(k, rapidjson::Value(value), document.GetAllocator());
}

void JsonWritter::addMember(const char *key, const std::string &value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), document.GetAllocator());
    rapidjson::Value v;
    v.SetString(value.c_str(), value.size(), document.GetAllocator());
    document.AddMember(k, v, document.GetAllocator());
}

void JsonWritter::addArray(const char *key, JsonArrayWritter &value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), document.GetAllocator());
    document.AddMember(k, value.array, document.GetAllocator());
}

void JsonWritter::addObject(const char *key, JsonObjectWritter &value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), document.GetAllocator());
    document.AddMember(k, value.object, document.GetAllocator());
}

template<typename T>
void JsonWritter::addClass(const char *key, const T &value) {
    rapidjson::Value k;
    k.SetString(key, strlen(key), document.GetAllocator());
    document.AddMember(k, JsonClassWritter<T>(document.GetAllocator(), value).value, document.GetAllocator());
}

std::string JsonWritter::getString() {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return std::string(buffer.GetString());
}

#endif //SERVER_JSONWRITTER_H
