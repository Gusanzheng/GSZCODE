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

#include <iostream>
#include <string>


int main(int argc, char* argv[])
{
    if (argc <= 2)
    {
        std::cout << "usage: " << basename(argv[0]) <<" ip_address port_number" << std::endl;
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

    /*客户端主动connect一个socket*/
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    if (connect(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cout << "connection failed" << std::endl;
    }
    else {
        const std::string client_data = "Hello, server, this is client.\n";
        while (true) {
            if (send(sockfd, &client_data, client_data.size(), 0) >= 0) {
                std::cout << "sending finished" << std::endl;
            }
            else {
                std::cout << "errno is: " << errno << std::endl;
            }
            sleep(1);
        }
    }
    close(sockfd);
    return 0;
}
