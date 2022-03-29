#include "./myserver/webserver.h"

int main(int argc, char *argv[])
{
    //配置类
    Config config;
    //服务器类
    WebServer server;

    //初始化
    server.init(config.PORT, config.thread_num, config.max_requests);

    //运行循环
    server.eventLoop();

    return 0;
}