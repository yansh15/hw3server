#ifndef SERVER_READRINGBUFFER_H
#define SERVER_READRINGBUFFER_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

template <unsigned long SIZE>
class ReadRingBuffer {
public:
    ReadRingBuffer();
    ~ReadRingBuffer();

    unsigned long getSize() const;
    unsigned long getOccupancy() const;
    unsigned long getCapacity() const;

    std::string getAllData();
    uint8_t getUInt8();
    void putUInt8(uint8_t);
    int8_t getInt8();
    void putInt8(int8_t);
    uint16_t getUInt16LE();
    void putUInt16LE(uint16_t);
    int16_t getInt16LE();
    void putInt16LE(int16_t);
    uint32_t getUInt32LE();
    void putUInt32LE(uint32_t);
    int32_t getInt32LE();
    void putInt32LE(int32_t);
    uint64_t getUInt64LE();
    void putUInt64LE(uint64_t);
    int64_t getInt64LE();
    void putInt64LE(int64_t);
    std::string getString(unsigned long n);
    void putString(const std::string& str);
    void getCharArray(char *arr, unsigned long n);
    void putCharArray(char *arr, unsigned long n);

    uint16_t lookAheadUInt16LE() const;

private:
    char *buffer;
    char *get;
    char *put;
    unsigned long occupancy;

    void addGetPos(ptrdiff_t ptrdiff);
    void addPutPos(ptrdiff_t ptrdiff);
    void getNBytes(char *tmp, unsigned long n);
    void putNBytes(const char *tmp, unsigned long n);
    void lookAheadNBytes(char *tmp, unsigned long n) const;
};

template<unsigned long SIZE>
ReadRingBuffer<SIZE>::ReadRingBuffer() : buffer(new char[SIZE]), occupancy(0) {
    get = buffer;
    put = buffer;
}

template<unsigned long SIZE>
ReadRingBuffer<SIZE>::~ReadRingBuffer() {
    if (buffer)
        delete buffer;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::addGetPos(ptrdiff_t ptrdiff) {
    get = get + ptrdiff;
    occupancy = occupancy - ptrdiff;
    if (get >= buffer + SIZE)
        get = get - SIZE;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::addPutPos(ptrdiff_t ptrdiff) {
    put = put + ptrdiff;
    occupancy = occupancy + ptrdiff;
    if (put >= buffer + SIZE)
        put = put - SIZE;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::getNBytes(char *tmp, unsigned long n) {
    unsigned long len = buffer + SIZE - get;
    if (len >= n) {
        memmove(tmp, get, n);
        addGetPos(n);
    } else {
        memmove(tmp, get, len);
        addGetPos(len);
        tmp = tmp + len;
        memmove(tmp, get, n - len);
        addGetPos(n - len);
    }
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putNBytes(const char *tmp, unsigned long n)  {
    unsigned long len = buffer + SIZE - put;
    if (len >= n) {
        memmove(put, tmp, n);
        addPutPos(n);
    } else {
        memmove(put, tmp, len);
        addPutPos(len);
        tmp = tmp + len;
        memmove(put, tmp, n - len);
        addPutPos(n - len);
    }
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::lookAheadNBytes(char *tmp, unsigned long n) const {
    char *cur = get;
    unsigned long len = buffer + SIZE - cur;
    if (len >= n) {
        memmove(tmp, cur, n);
    } else {
        memmove(tmp, cur, len);
        tmp = tmp + len;
        cur = buffer;
        memmove(tmp, cur, n - len);
    }
}

template<unsigned long SIZE>
unsigned long ReadRingBuffer<SIZE>::getSize() const {
    return SIZE;
}

template<unsigned long SIZE>
unsigned long ReadRingBuffer<SIZE>::getOccupancy() const {
    return occupancy;
}

template<unsigned long SIZE>
unsigned long ReadRingBuffer<SIZE>::getCapacity() const {
    return SIZE - occupancy;
}

template<unsigned long SIZE>
std::string ReadRingBuffer<SIZE>::getAllData() {
    unsigned long n = occupancy;
    char *tmp = new char[n];
    getNBytes(tmp, n);
    return std::string(tmp, n);
}

template<unsigned long SIZE>
uint8_t ReadRingBuffer<SIZE>::getUInt8() {
    uint8_t ret = *(uint8_t*)get;
    addGetPos(sizeof(uint8_t));
    return 0;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putUInt8(uint8_t v) {
    *(uint8_t*)put = v;
    addPutPos(sizeof(uint8_t));
}

template<unsigned long SIZE>
int8_t ReadRingBuffer<SIZE>::getInt8() {
    int8_t ret = *(int8_t*)get;
    addGetPos(sizeof(int8_t));
    return ret;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putInt8(int8_t v) {
    *(int8_t*)put = v;
    addPutPos(sizeof(int8_t));
}

template<unsigned long SIZE>
uint16_t ReadRingBuffer<SIZE>::getUInt16LE() {
    char tmp[sizeof(uint16_t)];
    getNBytes(tmp, sizeof(uint16_t));
    return *(uint16_t*)tmp;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putUInt16LE(uint16_t v) {
    auto *tmp = (char*)&v;
    putNBytes(tmp, sizeof(uint16_t));
}

template<unsigned long SIZE>
int16_t ReadRingBuffer<SIZE>::getInt16LE() {
    char tmp[sizeof(int16_t)];
    getNBytes(tmp, sizeof(int16_t));
    return *(int16_t*)tmp;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putInt16LE(int16_t v) {
    auto *tmp = (char*)&v;
    putNBytes(tmp, sizeof(int16_t));
}

template<unsigned long SIZE>
uint32_t ReadRingBuffer<SIZE>::getUInt32LE() {
    char tmp[sizeof(uint32_t)];
    getNBytes(tmp, sizeof(uint32_t));
    return *(uint32_t *)tmp;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putUInt32LE(uint32_t v) {
    auto *tmp = (char*)&v;
    putNBytes(tmp, sizeof(uint32_t));
}

template<unsigned long SIZE>
int32_t ReadRingBuffer<SIZE>::getInt32LE() {
    char tmp[sizeof(int32_t)];
    getNBytes(tmp, sizeof(int32_t));
    return *(int32_t *)tmp;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putInt32LE(int32_t v) {
    auto *tmp = (char*)&v;
    putNBytes(tmp, sizeof(int32_t));
}

template<unsigned long SIZE>
uint64_t ReadRingBuffer<SIZE>::getUInt64LE() {
    char tmp[sizeof(uint64_t)];
    getNBytes(tmp, sizeof(uint64_t));
    return *(uint64_t *)tmp;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putUInt64LE(uint64_t v) {
    auto *tmp = (char*)&v;
    putNBytes(tmp, sizeof(uint64_t));
}

template<unsigned long SIZE>
int64_t ReadRingBuffer<SIZE>::getInt64LE() {
    char tmp[sizeof(int64_t)];
    getNBytes(tmp, sizeof(int64_t));
    return *(int64_t *)tmp;
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putInt64LE(int64_t v) {
    auto *tmp = (char*)&v;
    putNBytes(tmp, sizeof(int64_t));
}

template<unsigned long SIZE>
std::string ReadRingBuffer<SIZE>::getString(unsigned long n) {
    auto *tmp = new char[n];
    getNBytes(tmp, n);
    return std::string(tmp, n);
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putString(const std::string& str) {
    putNBytes(str.data(), str.size());
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::getCharArray(char *arr, unsigned long n) {
    getNBytes(arr, n);
}

template<unsigned long SIZE>
void ReadRingBuffer<SIZE>::putCharArray(char *arr, unsigned long n) {
    putNBytes(arr, n);
}

template<unsigned long SIZE>
uint16_t ReadRingBuffer<SIZE>::lookAheadUInt16LE() const {
    char tmp[sizeof(uint16_t)];
    lookAheadNBytes(tmp, sizeof(uint16_t));
    return *(uint16_t*)tmp;
}

#endif //SERVER_READRINGBUFFER_H
