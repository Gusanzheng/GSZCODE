#include <mytimer.h>
#include <iostream>


void sort_timer_lst::add_timer(shared_ptr<util_timer> timer)
{
    
    if (!timer)
    {
        std::cout << "定时器升序链表add_timer_nullptr" << std::endl;
        return;
    }
    if (!head)
    {
        std::cout << "定时器升序链表add_timer:head = tail = timer" << std::endl;
        head = tail = timer;
        return;
    }
    /*如果目标定时器的超时时间<当前链表中所有定时器的超时时间，就把目标定时器插入链表头部*/
    if (timer->expire < head->expire)
    {
        std::cout << "定时器升序链表add_timer,插入头部" << std::endl;
        timer->next = head;
        head->prev = timer;
        head = timer;
        return;
    }
    /*否则，插入链表中*/
    std::cout << "定时器升序链表add_timer,插入链表中" << std::endl;
    add_timer(timer, head);
}

/*调整定时器在链表中的位置，超时时间长的往尾部走*/
void sort_timer_lst::adjust_timer(shared_ptr<util_timer> timer)
{
    if (!timer)
    {
        return;
    }
    shared_ptr<util_timer> tmp = timer->next;
    /*目标定时器已经在尾部了 或者 该定时器的超时时间已经比后面的小了，就不调整*/
    if (!tmp || (timer->expire < tmp->expire))
    {
        return;
    }
    /*如果目标定时器是头部，则拿出来重新插入链表*/
    if (timer == head)
    {
        head = head->next;
        head->prev = nullptr;
        timer->next = nullptr;
        add_timer(timer, head);
    }
    else//如果不是头部，则拿出来重新插入它所在位置之后的链表中
    {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
}

void sort_timer_lst::del_timer(shared_ptr<util_timer> timer)
{
    if (!timer)
    {
        return;
    }
    /*表示只有1个定时器*/
    if ((timer == head) && (timer == tail))
    {
        timer.reset();
        head = nullptr;
        tail = nullptr;
        return;
    }
    if (timer == head)//处于头部
    {
        head = head->next;
        head->prev = nullptr;
        timer.reset();
        return;
    }
    if (timer == tail)//处于尾部
    {
        tail = tail->prev;
        tail->next = nullptr;
        timer.reset();
        return;
    }
    /*处于中间*/
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    timer.reset();
}

void sort_timer_lst::tick()
{
    if (!head)
    {
        return;
    }
    //cout << "timer tick" << endl;
    time_t cur = time(nullptr);//获取当前系统的时间

    /*从头节点开始处理，直到遇到一个还没到期的定时器*/
    while (head)
    {
        if (cur < head->expire)//定时器都是用的是绝对时间,没超时呢
        {
            break;
        }
        head->cb_func(head->user_data);//调用定时器回调函数执行任务
        head = head->next;
        if (head)
        {
            head->prev = nullptr;
        }
    }
}

void sort_timer_lst::add_timer(shared_ptr<util_timer> timer, shared_ptr<util_timer> lst_head)
{
    shared_ptr<util_timer> prev = lst_head;
    shared_ptr<util_timer> tmp = prev->next;
    while (tmp)
    {
        if (timer->expire < tmp->expire)
        {
            prev->next = timer;
            timer->next = tmp;
            tmp->prev = timer;
            timer->prev = prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    /*遍历完都没找到合适位置，则放到尾部*/
    if (!tmp)
    {
        prev->next = timer;
        timer->prev = prev;
        timer->next = nullptr;
        tail = timer;
    }
}
