#include "../include/MemBuffer.h"
#include "malloc.h"

MemBuffer::~MemBuffer() {
	for (auto& m : bufferMem) {
		bufferChunk* free = m.second;
		while (free != nullptr) {
			bufferChunk* tmp = free->next;
			::free((char*)free-sizeof(size_t)); //释放内存
			free = tmp;
		}
	}
}
//malloc以字节为单位
void* MemBuffer::allocate(size_t size) {
	bufferChunk* res;
	bufferRwLock.rdLock();
	auto& lck = bufferLock[size];//创建一个引用对象
	bufferRwLock.unlock();
	lck.lock();
	if (bufferMem.find(size) == bufferMem.end()) {
		bufferMem[size] = nullptr;
	}
	if (bufferMem[size] == nullptr) {
		char* tmp = (char*)malloc(sizeof(size_t)+size);
		*(size_t*)tmp = size;	//在内存块的开始处存储内存块的大小
		res = (bufferChunk*)(tmp+sizeof(size_t));	//返回内存块的开始处的下一个位置
	} else {
		res = bufferMem[size];	//取出单链表的头部
		if (res->next) bufferMem[size] = res->next;
		else bufferMem.erase(size);
	}
	lck.unlock();
	return res;
}

void MemBuffer::deallocate(void* ptr) {
	size_t size = *(size_t*)((char*)ptr-sizeof(size_t));	//内存块大小
	bufferRwLock.rdLock();
	auto& lck = bufferLock[size];
	bufferRwLock.unlock();
	lck.lock();
	((bufferChunk*)ptr)->next = bufferMem[size];	//把不用的内存块放在单链表头部，不析构
	bufferMem[size] = (bufferChunk*)ptr;
	lck.unlock();
}