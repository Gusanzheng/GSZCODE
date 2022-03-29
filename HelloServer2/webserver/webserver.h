#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include "../threadpool/threadpool.h"
#include "../http/http_conn.h"
#include "../Utils/Utils.h"

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位

class WebServer
{
public:
    WebServer();
    ~WebServer();

    void init(int port,int thread_num);
    void thread_pool();
    void trig_mode();
    void eventListen();
    void eventLoop();

public:
    /*定时器*/
    void timer(int connfd, struct sockaddr_in client_address);
    void adjust_timer(Util_timer *timer);
    void deal_timer(Util_timer *timer, int sockfd);
    bool dealclinetdata();

    /*信号signal*/
    bool dealwithsignal(bool& timeout, bool& stop_server);

    /*I/O*/
    void dealwithread(int sockfd);
    void dealwithwrite(int sockfd);

public:
    //基础
    int m_port;
    char *m_root;
    int m_actormodel;

    int m_pipefd[2];
    int m_epollfd;
    http_conn *users;


    //线程池相关
    threadpool<http_conn> *m_pool;
    int m_thread_num;

    //epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listenfd;

    //定时器相关
    client_data *users_timer;
    Utils utils;
};
#endif
