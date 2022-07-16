#ifndef MYPTHREAD_RWLOCK_H
#define MYPTHREAD_RWLOCK_H

#include <pthread.h>
#include <exception>

class RwLock {
public:
    RwLock() {
        if (pthread_rwlock_init(&m_rwlock, nullptr) != 0)
            throw std::exception();
    }

    ~RwLock() {
        pthread_rwlock_destroy(&m_rwlock);
    }

    bool rdLock() {
        if (pthread_rwlock_rdlock(&m_rwlock) != 0)
            return false;
        return true;
    }

    bool wrLock() {
        if (pthread_rwlock_wrlock(&m_rwlock) != 0)
            return false;
        return true;
    }

    bool unlock() 
    {
        if (pthread_rwlock_unlock(&m_rwlock) != 0)
            return false;
        return true;
    }

    int tryRdLock()
    {
        int state = pthread_rwlock_tryrdlock(&m_rwlock);
        if (state == EBUSY) {
            return 1;
        }
        if (state != 0) {
            return -1;
        }
        return 0;
    }

    int tryWrLock() {
        int state = pthread_rwlock_trywrlock(&m_rwlock);
        if (state == EBUSY) {
            return 1;
        }
        if (state != 0) {
            return -1;
        }
        return 0;
    }
    
    pthread_rwlock_t* get() {
        return &m_rwlock;
    }

    //禁止自动生成拷贝构造、拷贝赋值、移动构造、移动赋值
    RwLock(const RwLock&) = delete;
    RwLock(RwLock&&) = delete;
    RwLock& operator=(const RwLock&) = delete;
    RwLock& operator=(RwLock&&) = delete;
private:
    pthread_rwlock_t  m_rwlock;
};


#endif //MYPTHREAD_RWLOCK_H
