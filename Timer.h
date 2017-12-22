#ifndef SERVER_TIMER_H
#define SERVER_TIMER_H

#include<functional>
#include<chrono>
#include<thread>
#include<atomic>
#include<memory>
#include<mutex>
#include<condition_variable>

class Timer {
public:
    Timer();
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

    void start(int milliseconds, std::function<void()> task);
};

Timer::Timer() {}

Timer::~Timer() {}

void Timer::start(int milliseconds, std::function<void()> task) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        task();
    }
}

#endif //SERVER_TIMER_H
