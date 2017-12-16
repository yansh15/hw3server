cmake_minimum_required(VERSION 3.9)
project(server)

set(CMAKE_CXX_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable (server main.cpp Constant.h Tcp.h ReadRingBuffer.h Controller.h UserInfo.h rapidjson JsonWritter.h JsonReader.h)
target_link_libraries (server ${CMAKE_THREAD_LIBS_INIT})