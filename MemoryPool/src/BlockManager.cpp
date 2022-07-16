#include "../include/BlockManager.h"
#include <stdexcept>
#include <new>

BlockManager::BlockManager(size_t size)
    : blockSize(size), headPtr(nullptr), freePtr(nullptr) {}

BlockManager::~BlockManager() {
    while (headPtr != nullptr) {    //释放各个内存块指针
        auto tmp = headPtr;
        headPtr = headPtr->next;
        delete tmp;
    }
}

void* BlockManager::allocate(int num) {
    /*一个内存块也没有*/
    if (headPtr == nullptr) {
        headPtr = new MemBlock(num, blockSize);
        return headPtr->allocate();
    }
    /*有内存块，那从头遍历看看有没有可用内存*/
    auto curr = headPtr;
    while (curr != nullptr) {
        if (curr->num != 0) {//发现还剩余可用内存
            if (curr == headPtr) return headPtr->allocate();
            //头部没有可用的，则取出空闲指针所指的内存块,因为有curr了，所以把freePtr置空，防止空悬指针
            if (curr == freePtr) freePtr = nullptr;
            /*取出当前内存块放到链表头部*/
            curr->prev->next = curr->next;
            if (curr->next != nullptr) curr->next->prev = curr->prev;
            curr->prev = nullptr;
            curr->next = headPtr;
            headPtr->prev = curr;
            headPtr = curr;
            return headPtr->allocate();
        }
        curr = curr->next;
    }
    /*没有可用内存块，则申请新的内存块,并放到链表头部*/
    auto newBlock = new MemBlock(num, blockSize);
    newBlock->next = headPtr;
    headPtr->prev = newBlock;
    headPtr = newBlock;
    return headPtr->allocate();
}

void BlockManager::deallocate(void* p, int num) {
    if (p == nullptr) {
        throw std::logic_error("回收失败,指针无效");
        return;
    } else {
        //从头遍历
        auto curr = headPtr;
        while (curr != nullptr) {
            //找到P所在的内存块
            if (p >= (void*)(curr->headPtr) && p < (void*)((char*)(curr->headPtr) +blockSize*num)) {
                curr->deallocate(p);//回收p指针所指的内存单元
                /*回收完后，发现p所在的内存块刚好剩余的内存单元为num个，即是一个空的内存块*/
                if (curr->num == num) {
                    /*把管理的链表中空闲的一个内存块释放了，把该内存块放进去*/
                    if (freePtr != nullptr && freePtr != curr) {
                        if (freePtr == headPtr) {
                            //头部就是空闲的，则头部指针后移，把freePtr取出来
                            headPtr = freePtr->next;
                            headPtr->prev = nullptr;
                        } else {
                            //从链表中取出freePtr所指的内存块
                            freePtr->prev->next = freePtr->next;
                            if (freePtr->next != nullptr) freePtr->next->prev = freePtr->prev;
                        }
                        delete(freePtr); //释放freePtr所指的内存块
                    }
                    freePtr = curr;//freePtr指向要释放的内存块，即可回收
                }
                /*把curr放到所管理的内存块链条头部*/
                if (curr == headPtr) return;
                if (curr->prev != nullptr) curr->prev->next = curr->next;
                if (curr->next != nullptr) curr->next->prev = curr->prev;
                curr->prev = nullptr;
                curr->next = headPtr;
                headPtr->prev = curr;
                headPtr = curr;
                return;
            }
            curr = curr->next;
        }
        throw std::logic_error("该指针不在我们所拥有的所有类型的内存池内");
        return;
    }
}