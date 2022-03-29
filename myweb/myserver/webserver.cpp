/*
*服务器程序要处理的3件事：
* 1. I/O事件
* 2. 信号signal事件
* 3. 定时器timer事件
*/

#include <webserver.h>

#include <memory>


WebServer::WebServer()
{
    //http_conn类对象
    users = new http_conn[MAX_FD];

    //root文件夹路径
    char server_path[200] = "/home/admin/Code/hsycode/myweb";
    //getcwd(server_path, 200); //获取当前地址
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);
    std::cout << "Webserver 对象已建立, root地址: " << m_root << std::endl;
    users_timer = new client_data[MAX_FD];
}
WebServer::~WebServer()
{
    close(*m_epollfd);
    close(*m_listenfd);
    close((*m_pipefd)[1]);
    close((*m_pipefd)[0]);
    delete[] users;
    delete[] users_timer;
    delete m_pool;
}
void WebServer::init(int port, int thread_num,int max_requests)
{
    //tcp连接相关
    m_tcpserver = std::make_shared<tcpserver> ();
    m_tcpserver->init(port);            //初始化连接
    //m_listenfd = std::make_shared<int>( &(m_tcpserver->m_listenfd) );
    m_listenfd = &(m_tcpserver->m_listenfd);
    std::cout << "得到的m_listenfd= " << *m_listenfd << std::endl;

    //listener
    m_listener = std::make_shared<listener>();
    m_listener->Listen(*m_listenfd); //开启监听
    m_pipefd = &(m_listener->m_pipefd);////////
    m_epollfd = &(m_listener->m_epollfd);
    events = &(m_listener->events);

    //工具类,信号和描述符基础操作
    utils = &(m_listener->utils);
    //std::cout << "WebServer 对象初始化完成" << std::endl;

    //线程池
    m_pool = new threadpool<http_conn>(thread_num, max_requests);
    //std::cout << "开启线程池---成功" << std::endl; 

}
//主循环：1.处理timer 2.处理signal3.处理I/O
void WebServer::eventLoop()
{
    bool timeout = false;
    bool stop_server = false;
    std::cout << "开始进入主线程循环" << std::endl;
    while (!stop_server)
    {
        int number = epoll_wait(*m_epollfd, *events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR) //忽略errno=4的错误，重新epol_wait
        {
            //LOG_ERROR("%s", "epoll failure");
            std::cout << "epoll failed" << std::endl;
            break;
        }
        std::cout << "number= " << number << std::endl;
        for (int i = 0; i < number; i++)
        {
            int sockfd = (*events)[i].data.fd;
            bool t1 = sockfd == (*m_pipefd)[0];
            std::cout << "sockfd == (*m_pipefd)[0]? " << t1 << std::endl;
            bool t2 = (*events)[i].events & EPOLLIN;
            std::cout << "(*events)[i].events & EPOLLIN == true? " << t2 << std::endl;
            /********************处理定时器timer*********************/
            //处理新到的客户连接
            if (sockfd == *m_listenfd)
            {
                std::cout << "有新的客户连接" << std::endl;
                if (!dealclinetdata()) continue;
            }
            //如果有异常，直接关闭连接
            else if ((*events)[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                std::cout << "有异常,直接关闭连接" << std::endl;
                shared_ptr<util_timer> timer = users_timer[sockfd].timer;
                del_timer(timer, sockfd);
            }

            /********************处理信号signal*********************/
            
            else if ((sockfd == (*m_pipefd)[0]) && ((*events)[i].events & EPOLLIN))
            {
                std::cout << "处理信号signal删除非活动事件" << std::endl;
                if (!dealwithsignal(timeout, stop_server))
                    //LOG_ERROR("%s", "dealclientdata failure");
                    std::cout << "deal with client_data failed" << std::endl;
            }

            /********************处理I/O*********************/
            else if ((*events)[i].events & EPOLLIN)
            {
                std::cout << "检测到读事件" << std::endl;
                dealwithread(sockfd);
            }
            else if ((*events)[i].events & EPOLLOUT)
            {
                std::cout << "检测到写事件" << std::endl;
                dealwithwrite(sockfd);
            }

            else {
                std::cout << "Something else happened!" << std::endl;
            }
        }
        if (timeout)
        {
            utils->timer_handler();
            //LOG_INFO("%s", "timer tick");
            std::cout << "******************************timer tick*******************************" << std::endl;
            timeout = false;
        }
    }
}


/******************************处理定时器timer事件***************************************/
//定时器回调函数
void cb_func(client_data* user_data)
{
    //删除非活动连接在socket上的注册事件
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);

    //关闭文件描述符
    close(user_data->sockfd);

    //减少连接数
    http_conn::m_user_count--;//???
}
void WebServer::timer_init(int connfd, struct sockaddr_in client_address)
{
    std::cout << "timer_init的connd= " << connfd << std::endl;
    //初始化http_conn对象
    users[connfd].init(connfd, client_address, m_root);

    //初始化client_data数据
    users_timer[connfd].address = client_address;
    users_timer[connfd].sockfd = connfd;
    
    /*定时器相关*/
    shared_ptr<util_timer> timer(new util_timer);         //创建定时器
    timer->user_data = &users_timer[connfd];    //绑定用户数据
    timer->cb_func = cb_func;                   //设置回调函数
    time_t cur = time(nullptr);                 //获得系统时间
    timer->expire = cur + 3 * TIMESLOT;         //设置超时时间
    users_timer[connfd].timer = timer;          //添加到client_data数据

    (utils->m_timer_lst).add_timer(timer);         //将定时器添加到链表
}
//若有数据传输，则将定时器往后延迟3个单位,并对新的定时器在链表上的位置进行调整
void WebServer::adjust_timer(shared_ptr<util_timer> timer)
{
    time_t cur = time(nullptr);
    timer->expire = cur + 3 * TIMESLOT;
    (utils->m_timer_lst).adjust_timer(timer);
}
void WebServer::del_timer(shared_ptr<util_timer> timer, int sockfd)
{
    timer->cb_func(&users_timer[sockfd]);
    if (timer) (utils->m_timer_lst).del_timer(timer);
}
//LT处理数据
bool WebServer::dealclinetdata()
{
    
    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    std::cout << "开始处理clientdata, accept的fd= " << *m_listenfd << std::endl;
    int connfd = accept(*m_listenfd, (struct sockaddr *)&client_address, &client_addrlength);
    if (connfd < 0)
    {
        //LOG_ERROR("%s:errno is:%d", "accept error", errno);
        std::cout << "accept failed, errno is: " << errno << std::endl;
        return false;
    }
    if (http_conn::m_user_count >= MAX_FD)
    {
        utils->show_error(connfd, "Internal server busy");
        //LOG_ERROR("%s", "Internal server busy");
        std::cout << "Internal server busy" << std::endl;
        return false;
    }
    std::cout << "正常， connfd = " << connfd << std::endl;
    timer_init(connfd, client_address); //生成新的定时事件
    return true;
}


/*******************************处理信号signal事件**************************************/
/*信号量SIG，用于通知进程状态的改变或者系统异常*/
//处理超时事件
bool WebServer::dealwithsignal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv((*m_pipefd)[0], signals, sizeof(signals), 0);
    if (ret <= 0)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < ret; ++i)
        {
            switch (signals[i])
            {
                case SIGALRM:   //超时
                {
                    timeout = true;
                    break;
                }
                case SIGTERM:   //终止
                {
                    stop_server = true;
                    break;
                }
            }
        }
    }
    return true;
}


/********************************处理I/O事件********************************************/
/*采用同步I/O模拟proactor模式(反应堆)*/
//处理I/O read
void WebServer::dealwithread(int sockfd)
{
    shared_ptr<util_timer> timer = users_timer[sockfd].timer;
    //proactor模式，若监测到读事件，将该事件放入请求队列
    if (users[sockfd].read_once())
    {
        m_pool->append_p(users + sockfd);
        if (timer) adjust_timer(timer);
    }
    else //没检测到，则删除timer，并关闭http_conn
    {
        del_timer(timer, sockfd);
    }
}
//处理I/O write
void WebServer::dealwithwrite(int sockfd)
{
    shared_ptr<util_timer> timer = users_timer[sockfd].timer;
    //proactor模式，若监测到写事件，将该事件放入请求队列
    if (users[sockfd].write())
    {
        //LOG_INFO("send data to the client(%s)", inet_ntoa(users[sockfd].get_address()->sin_addr));
        if (timer) adjust_timer(timer);
    }
    else //写失败了，则删除timer,并关闭http_conn
    {
        del_timer(timer, sockfd);
    }
}

