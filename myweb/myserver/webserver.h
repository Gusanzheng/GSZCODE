#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <iostream>
#include <memory>
#include "./config.h"
#include "../mythread/threadpool.h"
#include "../mylistener/listener.h"
#include "../myhttp/myhttp.h"
#include "../mytcpsession/tcpserver.h"
#include "../mytimer/mytimer.h"

const int MAX_FD = 65536;           //最大文件描述符

class WebServer
{
public:
    WebServer();
    ~WebServer();
    void init(int port, int thread_num, int max_requests);
    void eventLoop();

public:
    /*timer*/
    void timer_init(int connfd, struct sockaddr_in client_address);
    void adjust_timer(shared_ptr<util_timer> timer);
    void del_timer(shared_ptr<util_timer> timer, int sockfd);
    bool dealclinetdata();

    /*signal*/
    bool dealwithsignal(bool& timeout, bool& stop_server);

    /*I/O*/
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);

public:
    //基础
    char *m_root;
    int (*m_pipefd)[2];    ///////////指向int m_pipefd[2]
    
    //http相关
    http_conn* users;

    //线程池相关
    threadpool<http_conn> *m_pool;

    //tcp连接相关
    shared_ptr<tcpserver> m_tcpserver;
    //std::shared_ptr<int> m_listenfd = nullptr;
    int *m_listenfd;

    //listener相关
    shared_ptr<listener> m_listener;
    int *m_epollfd = nullptr;       //epoll内核事件表fd
    epoll_event (*events)[MAX_EVENT_NUMBER];  //指向内核事件表

    //int m_epollfd;
    //epoll_event events[MAX_EVENT_NUMBER];

    //定时器,工具类相关
    client_data *users_timer;
    Utils *utils = nullptr;
};
#endif
