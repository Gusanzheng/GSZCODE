#ifndef MYHTTP_H
#define MYHTTP_H
#include <unistd.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <cstdarg>
#include <cerrno>
#include <csignal>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "../mythread/locker.h" //借用同步控制类
#include "../mylistener/listener.h" //借用工具类Utils

using std::map;
using std::string;

class http_conn {
public:
    static const int FILENAME_LEN = 200;//文件名最大长度
    static const int READ_BUFFER_SIZE = 2048;//读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;//写缓冲区大小
    enum METHOD //HTTP请求方法
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE //解析客户请求时，主状态机所处的状态
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE //服务器处理HTTP请求的可能结果
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS //行的读取状态
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    /*初始化新接受的连接*/
    void init(int sockfd, const sockaddr_in& addr, char*);
    void close_conn(bool real_close = true); //关闭连接
    void process(); //处理客户请求
    bool read_once(); //读
    bool write(); //非阻塞写操作
    sockaddr_in* get_address()
    {
        return &m_address;
    }
    int timer_flag;
    int improv;

private:
    void init(); //初始化连接
    HTTP_CODE process_read(); //解析HTTP请求
    bool process_write(HTTP_CODE ret); //填充HTTP应答

    /*下面这一组函数被process_read调用以分析HTTP请求*/
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_content(char* text);
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; };
    LINE_STATUS parse_line();

    /*下面这一组函数被process_write调用以填充HTTP应答*/
    void unmap();
    bool add_response(const char* format, ...);
    bool add_content(const char* content);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    /*所有socket上的事件都被注册到同一个epoll内核事件表中，所以将epoll文件描述符设置为静态的*/
    static int m_epollfd;               //epoll内核事件表fd
    static int m_user_count;            //统计用户数量
    int m_state;                        //读为0, 写为1

private:
    /*工具类*/
    Utils http_utils;

    /*该HTTP连接的socket和对方的socket地址*/
    int m_sockfd;
    sockaddr_in m_address;

    char m_read_buf[READ_BUFFER_SIZE];  //读缓冲区
    int m_read_idx;                     //标识读缓冲中已经读入的客户数据的最后一个字节的下一个位置
    int m_checked_idx;                  //当前正在分析的字符在缓冲区的位置
    int m_start_line;                   //当前正在解析的行的起始位置
    char m_write_buf[WRITE_BUFFER_SIZE];//写缓冲区
    int m_write_idx;                    //写缓冲区待发送的字节数

    CHECK_STATE m_check_state;          //主状态机当前所处的状态
    METHOD m_method;                    //请求方法

    char m_real_file[FILENAME_LEN];     //客户请求的目标文件的完整路径
    char* m_url;                        //客户请求的目标文件的文件命名
    char* m_version;                    //HTTP版本协议号，我们仅支持HTTP/1.1
    char* m_host;                       //主机名
    int m_content_length;               //HTTP请求消息的长度
    bool m_linger;                      //HTTP请求是否要求保持连接

    char* m_file_address;               //客户请求的目标文件被mmap到内存的起始位置
    struct stat m_file_stat;            //目标文件的状态

    struct iovec m_iv[2];               //采用writev来执行写操作
    int m_iv_count;                     //写内存块 的数量

    int cgi;                            //是否启用的POST
    char* m_string;                     //存储请求头数据
    int bytes_to_send;                  //要发送
    int bytes_have_send;                //已发送
    char* doc_root;                     //网站根目录
};

#endif // !MYHTTP_H
