#ifndef TIMER_H
#define TIMER_H

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
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../Utils/Utils.h"

class Util_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    Util_timer *timer;
};

class Util_timer
{
public:
    Util_timer() : prev(nullptr), next(nullptr) {}

public:
    time_t expire;
    
    void (* cb_func)(client_data *);
    client_data *user_data;
    Util_timer *prev;
    Util_timer *next;
};

class Timer_lst
{
public:
    Timer_lst();
    ~Timer_lst();

    void add_timer(Util_timer *timer);
    void adjust_timer(Util_timer *timer);
    void del_timer(Util_timer *timer);
    void tick();

private:
    void add_timer(Util_timer *timer, Util_timer *lst_head);

    Util_timer *head;
    Util_timer *tail;
};


void cb_func(client_data *user_data);

#endif
