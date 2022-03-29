/*
*配置页面
*/

#ifndef CONFIG_H
#define CONFIG_H

class Config
{
public:
    Config() {
        PORT = 9006;
        thread_num = 8;
        max_requests = 10000;
    }
    ~Config(){}

public:
    int PORT;           //端口号
    int thread_num;     //线程池内的线程数量
    int max_requests;    //最大请求数量

};

#endif