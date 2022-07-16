#include <iostream>
#include <ctime>
#include <vector>
#include "../include/PoolManager.h"

using namespace std;

MemPool memPool(100000);
class A{
public:
    static void* operator new(size_t size) { 
        return memPool.allocate(size);
    }
    static void operator delete(void* ptr, size_t size) {
        memPool.deallocate(ptr, size);
    }
private:
    double a, b, c, d, e, f, g, h, i, j, k;
};

class B{
private:
    double a, b, c, d, e, f, g, h, i, j, k;
};

int main(int argc, char **argv)
{
    long long n = 10000000;
    vector<B, std::allocator<B>> u;
    clock_t start = clock();
    for (int i = 0; i < n; i++) {
        u.emplace_back();
    }
    for (int i = 0; i < n; i++) {
        u.pop_back();
    }
    for (int i = 0; i < n; i++) {
        u.emplace_back();
    }
    cout << "origin: "<< (double)(clock()-start) / CLOCKS_PER_SEC << endl;

    vector<B, PoolManager<B, 100000>> v;
    start = clock();
    for (int i = 0; i < n; i++) {
        v.emplace_back();
    }
    for (int i = 0; i < n; i++) {
        v.pop_back();
    }
    for (int i = 0; i < n; i++) {
        v.emplace_back();
    }
    cout << "memory pool: " << (double)(clock()-start) / CLOCKS_PER_SEC << endl;

    n = 100000;
    cout << "origin: ";
    B* q[n];
    start = clock();
    for (int i = 0; i < n; ++i){
        q[i] = new B;
    }
    cout << (double)(clock()-start) / CLOCKS_PER_SEC << " ";
    start = clock();
    for (int i = 0; i < n; i++){
        delete q[i];
    }
    cout << (double)(clock()-start) / CLOCKS_PER_SEC << endl;

    cout << "memory pool: ";
    A* p[n];
    start = clock();
    for (int i = 0; i < n; ++i){
        p[i] = new A;
    }
    cout << (double)(clock()-start) / CLOCKS_PER_SEC << " ";
    start = clock();
    for (int i = 0; i < n; i++){
        delete p[i];
    }
    cout << (double)(clock()-start) / CLOCKS_PER_SEC << endl;

    return 0;
}