//
// Created by ppd on 2021/10/14.
//

#ifndef EPOLLSERVER_SAFEQUEUE_H
#define EPOLLSERVER_SAFEQUEUE_H

#include <queue>
#include <mutex>
template<typename T>
class safequeue {
public:
    safequeue() = default;
    safequeue(const safequeue<T> &que) = delete;
    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();

    }
    void push(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(t);
    }
    void push(T &&t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(std::move(t));

    }
    bool pop(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_queue.empty()) {
            return false;
        }
        t = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
};


#endif //EPOLLSERVER_SAFEQUEUE_H
