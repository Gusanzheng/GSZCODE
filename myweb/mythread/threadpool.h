#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <exception>
#include <pthread.h>
#include <iostream>
#include <locker.h>

/*T是任务类,比如http连接类*/
template <typename T>
class threadpool
{
public:
    threadpool() = default;
    threadpool(int thread_number, int max_requests);
    ~threadpool() {
        delete[] m_threads;
    }

    /*往请求队列中添加任务*/
    bool append(T *request, int state);
    bool append_p(T *request);

private:
    static void *worker(void *arg);/*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    void run(); //工作函数，逻辑处理

private:
    int m_thread_number;        //线程池中的线程数,8
    int m_max_requests;         //请求队列中允许的最大请求数,10000
    pthread_t* m_threads;       //指向线程池的数组，其大小为m_thread_number

    std::list<T *> m_workqueue; //请求队列
    locker m_queuelocker;       //互斥锁，保护请求队列
    sem m_queuestat;            //信号量，判断是否有任务需要处理
};
/*3�εĹ��캯��*/
template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests)
        : m_thread_number(thread_number), m_max_requests(max_requests), m_threads(nullptr)
{
    if (thread_number <= 0 || max_requests <= 0) //���û�̻߳�û�������׳��쳣
        throw std::exception();
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads) //���new����ʧ�ܣ����׳��쳣
        throw std::exception();
    /*����thread_number���̣߳��������Ƕ�����Ϊ�����߳�*/
    for (int i = 0; i < thread_number; ++i)
    {
        if (pthread_create(m_threads + i, nullptr, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}
/*��Ա����������������������*/
template <typename T>
bool threadpool<T>::append_p(T* request)
{
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests)
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}
template <typename T>
bool threadpool<T>::append(T* request, int state) //����
{
    /*������������ʱһ��Ҫ�����������öδ���Ķ����ԣ���Ϊ���������̹߳���*/
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests)//����������������������������������������false
    {
        m_queuelocker.unlock();
        return false;
    }
    request->m_state = state;//////////////////
    m_workqueue.push_back(request);//����ѹ���������
    m_queuelocker.unlock();
    m_queuestat.post();//�����źţ��������wait���߳̿��Խ����������
    return true;
}

/*�����������ȡ����,���ڴ���pthread_create�����߳�*/
template <typename T>
void* threadpool<T>::worker(void* arg)
{
    threadpool* pool = (threadpool*)arg;//������ת��Ϊ�߳���
    pool->run();
    return pool;
}

/*��������*/
template <typename T>
void threadpool<T>::run() {
    while (true) //�̲߳�����
    {
        m_queuestat.wait();//�ȴ����ź���-1���ź���Ϊ0ʱ���߳̽�������

        /*�����Ѻ�ӻ�����*/
        m_queuelocker.lock();
        if (m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        /*ȡ����һ�����񣬲�������������б���ɾ��*/
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();

        if (!request) continue;
        request->process(); //������T����������
    }
}
#endif
