#ifndef MYTIMER_H
#define MYTIMER_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <ctime>
#include <memory>

using std::shared_ptr;

class util_timer;

/*连接资源类*/
struct client_data
{
    sockaddr_in address;           //客户端socket地址
    int sockfd;                    //socket文件描述符
    shared_ptr<util_timer> timer;  //定时器
};

/*定时器类*/
class util_timer
{
public:
    util_timer() : prev(nullptr), next(nullptr) {}
    ~util_timer() {}

public:
    time_t expire;                      //超时时间
    void (*cb_func)(client_data *);     //任务回调函数
    client_data *user_data;             //连接资源
    shared_ptr<util_timer> prev;        //指向前一个定时器
    shared_ptr<util_timer> next;        //指向下一个定时器
};

/*定时器容器---升序链表*/
class sort_timer_lst
{
public:
    sort_timer_lst() : head(nullptr), tail(nullptr) {}
    ~sort_timer_lst() {
        while (head)
            head = head->next;
    }

    void add_timer(shared_ptr<util_timer> timer);      //添加
    void adjust_timer(shared_ptr<util_timer> timer);   //调整
    void del_timer(shared_ptr<util_timer> timer);      //删除

    //定时器任务处理函数:使用统一事件源，SIGALRM信号每次被触发
    //主循环中调用一次定时任务处理函数，处理链表容器中到期的定时器
    void tick();

private:
    /*重载函数，将目标定时器timer添加到节点lst_head之后的部分链表中*/
    void add_timer(shared_ptr<util_timer> timer, shared_ptr<util_timer> lst_head);

    shared_ptr<util_timer> head;
    shared_ptr<util_timer> tail;
};
#endif
