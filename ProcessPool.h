//
// Created by ppd on 2021/10/13.
//

#ifndef EPOLLSERVER_PROCESSPOOL_H
#define EPOLLSERVER_PROCESSPOOL_H

#include <iostream>
#include "IOhandle.h"
const std::string GetCurrentSystemTime()
{
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm = localtime(&t);
    char date[60] = { 0 };
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return move(std::string(date));
}
class ThreadPool {
public:
    ThreadPool(uint32_t threadNum = std::thread::hardware_concurrency()*2):_max_thread_num(threadNum){
        if(threadNum < 0 || threadNum > 1000) {
            throw std::exception();
        }
        printf("============================>>threadpool init success thread num is:%d <<==========================\n",threadNum);
        for(uint32_t i=0;i<_max_thread_num;++i) {
            _thread_pool.emplace_back(task);
        }
    }

    ~ThreadPool() {
        shutdown = true;
        IOhandle::Pocesscv.notify_all();
        for(uint32_t i=0;i<_max_thread_num;++i) {
            if(_thread_pool[i].joinable()) {
                _thread_pool[i].join();
            }
        }

    }

private:
    static void task() {
        for(;;) {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                IOhandle::Pocesscv.wait(lock,[](){return IOhandle::Recvque.empty() == false;});
            }
            if(shutdown) {
                std::cout<<"thread ID:"<<std::this_thread::get_id()<<" is exited"<<std::endl;
            }
            if(!IOhandle::Recvque.empty()) {
                printf("have data need to process\n");
                std::unique_ptr<data> tmp;
                IOhandle::Recvque.pop(tmp);
                message *pdata = reinterpret_cast<message*>(tmp->ptr.get());
                printf("message id is %d\n",pdata->head.id);
                if(pdata->head.id == 0) {
                    if(strncmp(pdata->tmp,"time\r\n",pdata->head.len) == 0){
                        std::string curtime = GetCurrentSystemTime();
                        std::unique_ptr<senddata> data(new senddata);
                        data->ptr.reset(new uint8_t[curtime.size()+1]);
                        data->fd = tmp->fd;
                        data->len = curtime.size();
                        strcpy(reinterpret_cast<char*>(data->ptr.get()),curtime.c_str());
                        IOhandle::Sendque.push(std::move(data));
                        IOhandle::Sendcv.notify_one();
                    }else {
                        std::cout<<"recv data error"<<std::endl;
                    }

                }else {
                    printf("Thread ID :%ld recv data error\n",std::this_thread::get_id());
                }



            }
        }
    }


private:
    uint32_t _max_thread_num;
    uint32_t _max_queue_size;
    static std::mutex m_mutex;
    std::vector<std::thread> _thread_pool;

public:
    static volatile bool shutdown;







};
volatile bool ThreadPool::shutdown;
std::mutex ThreadPool::m_mutex;
#endif //EPOLLSERVER_PROCESSPOOL_H
