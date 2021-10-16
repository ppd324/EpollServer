//
// Created by ppd on 2021/10/13.
//


#include "IOhandle.h"
void IOhandle::readThread() {
    printf("===============>>readThread init success<<==================\n");
    uint8_t buf[4096];
    for(;;) {
        memset(buf,0,sizeof(buf));
        {
            std::unique_lock<std::mutex> l1(readMutex);
            Readcv.wait(l1,[]{return Readque.empty() == false;});
        }
        if(!Readque.empty()) {
            int fd;
            Readque.pop(fd);
            int ret = recv(fd,buf,sizeof(buf),MSG_WAITALL);
            printf("%s",buf);
            if(ret>0) {
                std::unique_ptr<data> pData(new data);
                std::unique_ptr<uint8_t> tmp(new uint8_t[ret]);
                memcpy(tmp.get(),buf,ret);
                pData->ptr = std::move(tmp);
                pData->fd = fd;
                Recvque.push(std::move(pData));
                Pocesscv.notify_one();
            }
        }
    }

}

void IOhandle::writeThread() {
    printf("===============>>writeThread init success<<==================\n");
    for(;;) {
        int fd;
        size_t len;
        {
        std::unique_lock<std::mutex> l2(SendMutex);
        Sendcv.wait(l2, [] { return Sendque.empty() == false; });
        }
        if(!Sendque.empty()) {
            std::unique_ptr<senddata> data;
            Sendque.pop(data);
            int ret = send(data->fd,data->ptr.get(),data->len,0);
            if(ret < 0) {
                printf("send failed\n");
            }else {
                printf("send %d bytes success\n",ret);
            }

        }

    }

}
std::mutex IOhandle::SendMutex;
std::mutex IOhandle::readMutex;
safequeue<std::unique_ptr<data>> IOhandle::Recvque;
safequeue<std::unique_ptr<senddata>> IOhandle::Sendque;
std::condition_variable IOhandle::Readcv;
std::condition_variable IOhandle::Pocesscv;
std::condition_variable IOhandle::Sendcv;
safequeue<int> IOhandle::Readque;

