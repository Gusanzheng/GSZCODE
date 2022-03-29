#ifndef UTILS_H
#define UTILS_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <sys/types.h> //通用
#include <sys/socket.h> //通用
#include <signal.h>
#include <cassert>
#include <cstring> // memset
#include <memory>

#include "../timer/Timer.h"

class Timer_lst;

class Utils {
public:
    Utils() {
        m_timer_lst = std::make_shared<Timer_lst>();
    }
    ~Utils(){}
    
public:
    void init(int timeslot);
    //对文件描述符设置非阻塞
    int setnonblocking(int fd);
    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot);
    //从内核时间表删除描述符
    void removefd(int epollfd, int fd);
    //将事件重置为EPOLLONESHOT
    void modfd(int epollfd, int fd, int ev);

    //信号处理函数
    static void sig_handler(int sig);
    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);
    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();
    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    static int u_epollfd;
    int m_TIMESLOT;
    std::shared_ptr<Timer_lst> m_timer_lst;
};
#endif