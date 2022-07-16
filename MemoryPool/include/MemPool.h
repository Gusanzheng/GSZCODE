#ifndef MEMPOOL_H
#define MEMPOOL_H

#include "vector"
#include "memory"
#include "BlockManager.h"
#include "Mutex.h"
#include "SpinLock.h"
#include "RwLock.h"

using std::vector;
using std::shared_ptr;
using std::make_shared;

class MemPool {
public:
    MemPool() = default;
    explicit MemPool(int num);
    ~MemPool() = default;

    //禁止自动生成拷贝构造，拷贝赋值，移动构造，移动赋值
    MemPool(const MemPool&) = delete;
    MemPool& operator=(const MemPool&) = delete;
    MemPool(MemPool&&) = delete;
    MemPool& operator=(MemPool&&) = delete;

public:
    void* allocate(size_t size);   //分配1个大小为size的可用地址
    void deallocate(void* p, size_t size);  //回收p指针所指的大小为size的内存单元

private:
    int num;
    vector<BlockManager> memPool;   //管理不同blockSize的内存块的manager
    vector<Mutex> lockVec;  //可换成spinlock,rwlock

};



#endif