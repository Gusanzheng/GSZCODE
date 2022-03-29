#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> //IP地址转换
#include <netinet/in.h> //字节序转换,IPV4地址结构体sockaddr_in
#include <bits/socket.h> //通用地址结构体sockaddr
#include <unistd.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring> //bzero

#include <string>
#include <iostream>


const int BUF_SIZE = 1024;

int main(int argc, char* argv[])
{
    if (argc <= 2)
    {
        std::cout << "usage: " << basename(argv[0]) << " ip_address port_number" << std::endl;
        return 1;
    }
    const char* ip = argv[1]; //字符串形IP
    int port = atoi(argv[2]); //主机字节序形PORT

    /*配置目标adressIPV4地址*/
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr); //字符串形IP--->网络字节序形IP
    address.sin_port = htons(port); //主机字节序--->网络字节序形PORT

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    std::cout << "errno is: " << errno << std::endl;
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    sleep(20);//暂停20秒以等待客户端连接和相关操作完成

    /*服务器被动accept一个监听队列里的socket*/
    struct sockaddr_in client;//客户端IPV4地址,通过accept获取被接受连接的远端socket地址
    socklen_t client_addrlength = sizeof(client);
    int connfd = accept(sock, (struct sockaddr*)&client, &client_addrlength);
    if (connfd < 0) {
        std::cout << "errno is: " << errno << std::endl;
    }
    else {
        std::string buffer(BUF_SIZE, '\0');
	std::cout << "buffer setup successfully" << std::endl;
        while (true) {
            ret = recv(connfd, &buffer, BUF_SIZE - 1, 0);
            if (ret > 0) {
                std::cout << "got " << ret << " bytes of normal data: " << buffer << std::endl;
                buffer.assign(BUF_SIZE, '\0');
            }
            else if (ret == 0) {
                std::cout << "connection closed" << std::endl;
            }
            else {
                std::cout << "errno is: " << errno << std::endl;
            }
        }
        close(connfd);
    }
    close(sock);
    return 0;
}
