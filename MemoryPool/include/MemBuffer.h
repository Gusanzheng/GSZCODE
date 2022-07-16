/*
对于大的内存块，采用 哈希表+单链表 形式管理，过程中不是主动释放，而是被动释放
采用读写锁和自旋锁，保证线程安全
*/

#ifndef CENTRAL_CACHE_H
#define CENTRAL_CACHE_H

#include <unordered_map>
#include "Mutex.h"
#include "SpinLock.h"
#include "RwLock.h"

class MemBuffer {
public:
    MemBuffer() = default;
    ~MemBuffer();
    void* allocate(size_t size);
	void deallocate(void* ptr);

    //禁止自动生成拷贝构造，拷贝赋值，移动构造，移动赋值
    MemBuffer(const MemBuffer&) = delete;
    MemBuffer& operator=(const MemBuffer&) = delete;
    MemBuffer(MemBuffer&&) = delete;
    MemBuffer& operator=(MemBuffer&&) = delete;

private:
	struct bufferChunk { //解决大块的内存申请，如果没有，就去内存池里
		bufferChunk* next;
	};
	RwLock bufferRwLock;
	std::unordered_map<size_t, bufferChunk*> bufferMem;
	std::unordered_map<size_t, SpinLock> bufferLock;
};

#endif