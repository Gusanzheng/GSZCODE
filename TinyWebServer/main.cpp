#include "./webserver/config.h"
#include "./webserver/webserver.h"

int main(int argc, char *argv[])
{
    Config config;

    WebServer server;

    //初始化
    server.init(config.PORT, config.thread_num);

    //线程池
    server.thread_pool();

    //监听
    server.eventListen();

    //运行
    server.eventLoop();

    return 0;
}