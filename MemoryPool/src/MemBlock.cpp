#include "../include/MemBlock.h"
#include "malloc.h"
#include <stdexcept>

MemBlock::MemBlock(int num, size_t size) 
    : num(num), headPtr(nullptr), freePtr(nullptr), prev(nullptr), next(nullptr)
{
    while (true) {
        if (headPtr == nullptr) {
            headPtr = (Obj *) malloc(num * size); //申请大块内存，返回一个Obj*指针
        } else {
            break;
        }
    }
    freePtr = headPtr;
    /*把内存单元都存上下一单元的地址，然后串起来*/
    for (int i = 0; i < num-1; ++i) {
        freePtr->_next = (Obj*)((char*)freePtr + size);
        freePtr = freePtr->_next;
    }
    freePtr->_next = nullptr;
    freePtr = headPtr;
}

MemBlock::~MemBlock() {
    free(headPtr);  //把malloc申请的大块内存直接释放掉
}

void* MemBlock::allocate() {
    if (num == 0) {
        throw std::logic_error("没有可用内存来分配了,需要新的内存块");
        return nullptr;
    }
    Obj* cur = freePtr;
    freePtr = freePtr->_next;
    --num;
    return cur;
}

void MemBlock::deallocate(void* p) {
    if (p == nullptr) {
        throw std::logic_error("回收失败,指针无效");
        return;
    } else {
        ((Obj*)p)->_next = freePtr; //把P指针所指内存的内容换成下一个内存单元的地址,即串起来
        freePtr = (Obj*) p; //把空闲指针倒回去，就表示回收了
        ++num;
    }
}