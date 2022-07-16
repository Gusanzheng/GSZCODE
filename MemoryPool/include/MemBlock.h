#ifndef MEMBLOCK_H
#define MEMBLOCK_H

#include "memory"
#include "vector"

using std::vector;

class MemBlock {
    friend class BlockManager;
public:
    MemBlock() = default;
    MemBlock(int num, size_t size);    //num*size的内存块
    ~MemBlock();

public:
    void* allocate();           //分配1个可用地址
    void deallocate(void* p);   //回收p指针所指的1个内存单元

private:
    struct Obj {
        Obj* _next;
    };
    Obj* freePtr;
    Obj* headPtr;
    int num;    //可用内存单元数量

    MemBlock* prev;
    MemBlock* next;
};

#endif