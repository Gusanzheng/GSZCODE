#include "../include/SmartPointer.h"
#include <iostream>
#include <new>

using namespace std;

int main(int argc, char* argv[])
{
    cout << "This is a test of SmartPointer:" << endl;

    SharePtr<int> sp(new int(666));
    SharePtr<int> sp2(sp);
    SharePtr<int> sp3 = sp;
    cout << sp3.use_count() << endl;
    cout << sp3.unique() << endl;

    WeakPtr<int> wp(sp);
    WeakPtr<int> wp2(wp);
    WeakPtr<int> wp3 = wp;
    wp3.reset();
    SharePtr<int> sp4 = wp.lock();

    cout << "\nTest is over" << endl;  
    return 0;
}