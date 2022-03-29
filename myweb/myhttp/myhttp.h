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

#include "../mythread/locker.h" //����ͬ��������
#include "../mylistener/listener.h" //���ù�����Utils

using std::map;
using std::string;

class http_conn {
public:
    static const int FILENAME_LEN = 200;//�ļ�����󳤶�
    static const int READ_BUFFER_SIZE = 2048;//����������С
    static const int WRITE_BUFFER_SIZE = 1024;//д��������С
    enum METHOD //HTTP���󷽷�
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
    enum CHECK_STATE //�����ͻ�����ʱ����״̬��������״̬
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE //����������HTTP����Ŀ��ܽ��
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
    enum LINE_STATUS //�еĶ�ȡ״̬
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    http_conn() {}
    ~http_conn() {}

public:
    /*��ʼ���½��ܵ�����*/
    void init(int sockfd, const sockaddr_in& addr, char*);
    void close_conn(bool real_close = true); //�ر�����
    void process(); //����ͻ�����
    bool read_once(); //��
    bool write(); //������д����
    sockaddr_in* get_address()
    {
        return &m_address;
    }
    int timer_flag;
    int improv;

private:
    void init(); //��ʼ������
    HTTP_CODE process_read(); //����HTTP����
    bool process_write(HTTP_CODE ret); //���HTTPӦ��

    /*������һ�麯����process_read�����Է���HTTP����*/
    HTTP_CODE parse_request_line(char* text);
    HTTP_CODE parse_headers(char* text);
    HTTP_CODE parse_content(char* text);
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; };
    LINE_STATUS parse_line();

    /*������һ�麯����process_write���������HTTPӦ��*/
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
    /*����socket�ϵ��¼�����ע�ᵽͬһ��epoll�ں��¼����У����Խ�epoll�ļ�����������Ϊ��̬��*/
    static int m_epollfd;               //epoll�ں��¼���fd
    static int m_user_count;            //ͳ���û�����
    int m_state;                        //��Ϊ0, дΪ1

private:
    /*������*/
    Utils http_utils;

    /*��HTTP���ӵ�socket�ͶԷ���socket��ַ*/
    int m_sockfd;
    sockaddr_in m_address;

    char m_read_buf[READ_BUFFER_SIZE];  //��������
    int m_read_idx;                     //��ʶ���������Ѿ�����Ŀͻ����ݵ����һ���ֽڵ���һ��λ��
    int m_checked_idx;                  //��ǰ���ڷ������ַ��ڻ�������λ��
    int m_start_line;                   //��ǰ���ڽ������е���ʼλ��
    char m_write_buf[WRITE_BUFFER_SIZE];//д������
    int m_write_idx;                    //д�����������͵��ֽ���

    CHECK_STATE m_check_state;          //��״̬����ǰ������״̬
    METHOD m_method;                    //���󷽷�

    char m_real_file[FILENAME_LEN];     //�ͻ������Ŀ���ļ�������·��
    char* m_url;                        //�ͻ������Ŀ���ļ����ļ�����
    char* m_version;                    //HTTP�汾Э��ţ����ǽ�֧��HTTP/1.1
    char* m_host;                       //������
    int m_content_length;               //HTTP������Ϣ�ĳ���
    bool m_linger;                      //HTTP�����Ƿ�Ҫ�󱣳�����

    char* m_file_address;               //�ͻ������Ŀ���ļ���mmap���ڴ����ʼλ��
    struct stat m_file_stat;            //Ŀ���ļ���״̬

    struct iovec m_iv[2];               //����writev��ִ��д����
    int m_iv_count;                     //д�ڴ�� ������

    int cgi;                            //�Ƿ����õ�POST
    char* m_string;                     //�洢����ͷ����
    int bytes_to_send;                  //Ҫ����
    int bytes_have_send;                //�ѷ���
    char* doc_root;                     //��վ��Ŀ¼
};

#endif // !MYHTTP_H
