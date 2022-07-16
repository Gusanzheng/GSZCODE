#ifndef Mutex_H
#define Mutex_H

#include <exception>
#include <pthread.h>

class Mutex
{
public:
    Mutex() {
        if (pthread_mutex_init(&m_mutex, nullptr) != 0)
            throw std::exception();
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_mutex);
    }

    bool lock() {
        if (pthread_mutex_lock(&m_mutex) != 0)
            return false;
        return true;
    }

    bool unlock() {
        if (pthread_mutex_unlock(&m_mutex) != 0)
            return false;
        return true;
    }

    int tryLock() {
        int state = pthread_mutex_trylock(&m_mutex);
        if (state == EBUSY) {
            return 1;
        }
        if (state != 0) {
            return -1;
        }
        return 0;
    }

    pthread_mutex_t* get(){
        return &m_mutex;
    }

    //禁止自动生成拷贝构造、拷贝赋值、移动构造、移动赋值
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;
    Mutex& operator=(Mutex&&) = delete;
private:
    pthread_mutex_t m_mutex;
};
#endif
