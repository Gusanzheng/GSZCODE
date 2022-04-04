#ifndef SMARTPOINTER_H
#define SMARTPOINTER_H

#include <iostream>
using std::endl;
using std::cout;

class Counter
{
public:
    Counter() : s(0), w(0) {}

    int s;  //sharePtr的引用计数
    int w;  //weakPtr的引用计数
};

template <class T>
class WeakPtr;
 
template <class T>
class SharePtr
{
    friend class WeakPtr<T>;
public:
    SharePtr() : _ptr(nullptr), cnt(nullptr) {}

    SharePtr(T *p) : _ptr(p) {
        cnt = new Counter();
        if (p) cnt->s = 1;
        cout << "copy internal pointer " << cnt->s << endl;
    }

    ~SharePtr() {
        release();
    }
 
    SharePtr(const SharePtr<T> &s) {
        _ptr = s._ptr;
        (s.cnt)->s++;
        cout << "copy sharePtr " << (s.cnt)->s << endl;
        cnt = s.cnt;
    }

    SharePtr(WeakPtr<T> const &w) {
        _ptr = w._ptr;
        (w.cnt)->s++;
        cout << "copy weakPtr " << (w.cnt)->s << endl;
        cnt = w.cnt;
    }

    SharePtr<T>& operator=(const SharePtr<T> &s) {
        if (this != &s)
        {
            release();
            (s.cnt)->s++;
            cout << "assign construct " << (s.cnt)->s << endl;
            cnt = s.cnt;
            _ptr = s._ptr;
        }
        return *this;
    }
    
    T& operator*()
    {
        return *_ptr;
    }
    T* operator->()
    {
        return _ptr;
    }

public:
    int use_count() {
        return cnt->s;
    }

    bool unique() {
        return cnt->s == 1 ? true : false;
    }

protected:
    //计数-1
    void release() {
        cnt->s--;
        cout << "sharePtr release " << cnt->s << endl;
        if (cnt->s < 1)
        {
            delete _ptr;
            _ptr = nullptr;
            if (cnt->w < 1)//还有weakPtr在绑定该sharePtr
            {
                delete cnt;
                cnt = nullptr;
            }
        }
    }
 
private:
    T* _ptr;
    Counter* cnt;
};

template <class T>
class WeakPtr
{
    friend class SharePtr<T>;
public: 
    WeakPtr() : _ptr(nullptr), cnt(nullptr) {}

    WeakPtr(T* p) = delete;//拷贝构造不能有从原始指针进行构造

    WeakPtr(const SharePtr<T> &s) 
        : _ptr(s._ptr), cnt(s.cnt)
    {
        cnt->w++;
    }

    WeakPtr(const WeakPtr<T> &w)
        : _ptr(w._ptr), cnt(w.cnt)
    {
        cnt->w++;
    }

    ~WeakPtr() {
        release();
    }

    WeakPtr<T> &operator=(const WeakPtr<T> &w) {
        if (this != &w)
        {
            release(); //先把自身释放了
            cnt = w.cnt;
            cnt->w++;
            _ptr = w._ptr;
        }
        return *this;
    }

    WeakPtr<T> &operator=(const SharePtr<T> &s) {
        cout << "w = s" << endl;
        release();
        cnt = s.cnt;
        cnt->w++;
        _ptr = s._ptr;
        return *this;
    }

public:
    int use_count() {
        return cnt->s;
    }

    bool expired() {
        if (cnt->s > 0)
        {
            cout << "not expire " << cnt->s << endl;
            return false;
        }
        return true;
    }

    SharePtr<T> lock() {
        if (cnt && cnt->s == 0) return nullptr;
        return SharePtr<T>(*this);
    }

    void reset() {
        _ptr = nullptr;
        cnt->w--;
        cnt = nullptr;
    }

protected:
    void release() {
        if (cnt)
        {
            cnt->w--;
            cout << "weakptr release " << cnt->w << endl;
            if (cnt->w < 1 && cnt->s < 1)
            {
                delete cnt;
                cnt = nullptr;
            }
        }
    }
 
private:
    T *_ptr;
    Counter *cnt;
};

template<typename T>
class UniquePtr {
public:
    UniquePtr() : _ptr(nullptr) {}

    UniquePtr(T *p) : _ptr(p) {}

    ~UniquePtr() {
        delete _ptr;
        _ptr = nullptr;
    }

    UniquePtr(const UniquePtr &) = delete;
    UniquePtr& operator=(const UniquePtr &) = delete;

public:
    T* release() {
        auto res = _ptr;
        _ptr = nullptr;
        return res;
    }
private:
    T *_ptr;
};
#endif