//
// Created by ppd on 2021/10/13.
//

#ifndef EPOLLSERVER_IOHANDLE_H
#define EPOLLSERVER_IOHANDLE_H
#include <mutex>
#include <queue>
#include <sys/socket.h>
#include <memory>
#include <cstring>
#include <iostream>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <functional>
#include "safequeue.h"
#pragma pack(1)
struct data{
    std::unique_ptr<uint8_t> ptr;
    int fd;
};
#pragma pack(1)
struct header{
    uint8_t id;
    uint32_t len;
};
#pragma pack(1)
struct message {
    header head;
    char tmp[30];
};
#pragma pack(1)
struct senddata {
    std::unique_ptr<uint8_t> ptr;
    int fd;
    uint32_t len;
};
class IOhandle {
public:
    static void readThread();
    static void writeThread();
    static std::mutex readMutex;
    static std::mutex SendMutex;
    static safequeue<int> Readque;
    static safequeue<std::unique_ptr<data>> Recvque;
    static safequeue<std::unique_ptr<senddata>> Sendque;
    static std::condition_variable Readcv;
    static std::condition_variable Pocesscv;
    static std::condition_variable Sendcv;
};

#endif //EPOLLSERVER_IOHANDLE_H
