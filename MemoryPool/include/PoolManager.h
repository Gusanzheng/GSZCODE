#ifndef POOLMANAGER_H
#define POOLMANAGER_H

#include <cstddef>
#include "MemPool.h"
#include "MemBuffer.h"

static MemPool& getMemPool(int num) { 
    static MemPool memPool(num);
    return memPool;
}

static MemBuffer& getMemBuffer() {
    static MemBuffer MemBuffer;
    return MemBuffer;
}

template<typename T, int num>
class PoolManager {
public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef unsigned char data_type;
    typedef unsigned char* data_pointer;
    template<typename U>
    struct rebind{
        typedef PoolManager<U, num> other;
    };
    PoolManager() = default;
    ~PoolManager() = default;

public:
    inline T* allocate(size_t n); //内存池的内存块包含n个内存单元
    inline void deallocate(T* p, size_t n);
};

template<typename T, int num>
T* PoolManager<T, num>::allocate(size_t size)
{
    int numOfBytes = size*sizeof(T);  //需要多少bytes
    if (numOfBytes <= 512) {   //小于64 Bytes，从内存池里获取
        return (T*)getMemPool(num).allocate(numOfBytes);
    }
    else {  //大块，则从缓存区里获取
        return (T*)getMemBuffer().allocate(numOfBytes);
    }
}

template<typename T, int num>
void PoolManager<T, num>::deallocate(T* p, size_t size)
{
    int numOfBytes = size*sizeof(T);  //看一下释放多少bytes
    if (numOfBytes <= 512) {   //小于64 Bytes，还到内存池
        getMemPool(num).deallocate((void*)p, numOfBytes);
    }
    else {  //大块，则还到缓存区
        getMemBuffer().deallocate((void*)p);
    }
}
#endif