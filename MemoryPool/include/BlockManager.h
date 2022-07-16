#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include "MemBlock.h"

class BlockManager {
public:
    BlockManager() = default;
    BlockManager(size_t size);
    ~BlockManager();

public:
    //分配1个大小为blockSize的可用地址,当没有可用内存块时，num用于生成新的内存块
    void* allocate(int num);
    //回收p指针所指的大小为blockSize的内存单元,如果p所在内存块刚好空了，则把内存块也释放了
    void deallocate(void* p, int num);  

private:
    int blockSize;  //内存块内单元尺寸

    MemBlock* headPtr;
    MemBlock* freePtr;
};


#endif