#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <pthread.h>
#include <exception>

class SpinLock {
public:
    SpinLock() {
        if (pthread_spin_init(&m_spin, PTHREAD_PROCESS_PRIVATE) != 0)
            throw std::exception();
    }

    ~SpinLock() {
        pthread_spin_destroy(&m_spin);
    }

    bool lock(){
        if (pthread_spin_lock(&m_spin) != 0)
            return false;
        return true;
    }
    bool unlock() {
        if (pthread_spin_unlock(&m_spin) != 0)
            return false;
        return true;
    }
    int tryLock() {
        int state = pthread_spin_trylock(&m_spin);
        if (state == EBUSY) {
            return 1;
        }
        if (state != 0) {
            return -1;
        }
        return 0;
    }

    pthread_spinlock_t* get() {
        return &m_spin;
    }

    //禁止自动生成拷贝构造、拷贝赋值、移动构造、移动赋值
    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    SpinLock& operator=(const SpinLock&) = delete;
    SpinLock& operator=(SpinLock&&) = delete;
private:
    pthread_spinlock_t m_spin;
};


#endif //MYPTHREAD_SPINLOCK_H
