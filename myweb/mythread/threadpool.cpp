#include "./threadpool.h"

/*3锟轿的癸拷锟届函锟斤拷*/
template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests)
    : m_thread_number(thread_number), m_max_requests(max_requests), m_threads(nullptr)
{
    if (thread_number <= 0 || max_requests <= 0) //锟斤拷锟矫伙拷叱袒锟矫伙拷锟斤拷锟斤拷锟斤拷壮锟斤拷斐�
        throw std::exception();
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads) //锟斤拷锟絥ew锟斤拷锟斤拷失锟杰ｏ拷锟斤拷锟阶筹拷锟届常
        throw std::exception();
    /*锟斤拷锟斤拷thread_number锟斤拷锟竭程ｏ拷锟斤拷锟斤拷锟斤拷锟角讹拷锟斤拷锟斤拷为锟斤拷锟斤拷锟竭筹拷*/
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
/*锟斤拷员锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷*/
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
bool threadpool<T>::append(T* request, int state) //锟斤拷锟斤拷
{
    /*锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷时一锟斤拷要锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫段达拷锟斤拷亩锟斤拷锟斤拷裕锟斤拷锟轿�锟斤拷锟斤拷锟斤拷锟斤拷锟竭程癸拷锟斤拷*/
    m_queuelocker.lock();
    if (m_workqueue.size() >= m_max_requests)//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷false
    {
        m_queuelocker.unlock();
        return false;
    }
    request->m_state = state;//////////////////
    m_workqueue.push_back(request);//锟斤拷锟斤拷压锟斤拷锟斤拷锟斤拷锟斤拷锟�
    m_queuelocker.unlock();
    m_queuestat.post();//锟斤拷锟斤拷锟脚号ｏ拷锟斤拷锟斤拷锟斤拷锟絯ait锟斤拷锟竭程匡拷锟皆斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
    return true;
}

/*锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟饺★拷锟斤拷锟�,锟斤拷锟节达拷锟斤拷pthread_create锟斤拷锟斤拷锟竭筹拷*/
template <typename T>
void* threadpool<T>::worker(void* arg)
{
    threadpool* pool = (threadpool*)arg;//锟斤拷锟斤拷锟斤拷转锟斤拷为锟竭筹拷锟斤拷
    pool->run();
    return pool;
}

/*锟斤拷锟斤拷锟斤拷锟斤拷*/
template <typename T>
void threadpool<T>::run() {
    while (true) //锟竭程诧拷锟斤拷锟斤拷
    {
        m_queuestat.wait();//锟饺达拷锟斤拷锟脚猴拷锟斤拷-1锟斤拷锟脚猴拷锟斤拷为0时锟斤拷锟竭程斤拷锟斤拷锟斤拷锟斤拷

        /*锟斤拷锟斤拷锟窖猴拷踊锟斤拷锟斤拷锟�*/
        m_queuelocker.lock();
        if (m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        /*取锟斤拷锟斤拷一锟斤拷锟斤拷锟今，诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷斜锟斤拷锟缴撅拷锟�*/
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();

        if (!request) continue;
        request->process(); //锟斤拷锟斤拷锟斤拷T锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
    }
}