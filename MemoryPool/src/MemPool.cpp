#include "../include/MemPool.h"
#include <exception>
#include "malloc.h"

MemPool::MemPool(int num)
    : lockVec(128), num(num) 
{
    for (int i = 0; i < 128; ++i) {
        memPool.emplace_back(4+i*4);
    }
}

void* MemPool::allocate(size_t size) {
    /*我们只处理单元为4~512字节的内存*/
    if (size < 4 || size > 512) {
        void* tmp = ::malloc(size);
        if(tmp == nullptr) {
            throw std::exception();
        }
        return tmp;
    }
    // lockVec[(size-8)/4].lock();
    auto res = memPool[(size-8)/4].allocate(num);   //选择合适的内存池，从里面取一个可用的地址
    // lockVec[(size-8)/4].unlock();
    return res;
}

void MemPool::deallocate(void* p, size_t size) {
    if (size < 4 || size > 512) {
        ::free(p);
        p = nullptr;
        return;
    }
    // lockVec[(size-8)/4].lock();
    memPool[(size-8)/4].deallocate(p, num);
    // lockVec[(size-8)/4].unlock();
}