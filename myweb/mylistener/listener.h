#ifndef LISTENER_H
#define LISTENER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <ctime>

#include "../mytimer/mytimer.h"
#include "../mytcpsession/tcpserver.h"

const int MAX_EVENT_NUMBER = 10000;
const int TIMESLOT = 5;             //��С��ʱ��λ

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //���ļ����������÷�����
    int setnonblocking(int fd);

    //���ں��¼���ע����¼���ETģʽ��ѡ����EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot);
    
    //���ں�ʱ���ɾ��������
    void removefd(int epollfd, int fd);//��
    
    //���¼�����ΪEPOLLONESHOT
    void modfd(int epollfd, int fd, int ev);
    
    //�źŴ�����
    static void sig_handler(int sig);

    //�����źź���
    void addsig(int sig, void(handler)(int), bool restart = true);

    //��ʱ�����������¶�ʱ�Բ��ϴ���SIGALRM�ź�
    void timer_handler();

    void show_error(int connfd, const char* info);

public:
    static int* u_pipefd;
    sort_timer_lst m_timer_lst;
    static int u_epollfd;
    int m_TIMESLOT;
};

class listener {
public:
    listener() = default;
    ~listener() = default;

    void Listen(int m_listenfd);
public:
    epoll_event events[MAX_EVENT_NUMBER];
    int m_epollfd;
    int m_pipefd[2];
    Utils utils;
};
#endif
