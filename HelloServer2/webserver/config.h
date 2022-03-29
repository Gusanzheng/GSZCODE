#ifndef CONFIG_H
#define CONFIG_H

class Config
{
public:
    Config() {
        //端口号,默认9006
        PORT = 9006;
        //线程池内的线程数量,默认8
        thread_num = 8;
    }
    ~Config(){}
public:
    int PORT;
    int thread_num;
};

#endif