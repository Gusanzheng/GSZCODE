#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> //IP��ַת��
#include <netinet/in.h> //�ֽ���ת��,IPV4��ַ�ṹ��sockaddr_in
#include <bits/socket.h> //ͨ�õ�ַ�ṹ��sockaddr
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
    const char* ip = argv[1]; //�ַ�����IP
    int port = atoi(argv[2]); //�����ֽ�����PORT

    /*����Ŀ��adressIPV4��ַ*/
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr); //�ַ�����IP--->�����ֽ�����IP
    address.sin_port = htons(port); //�����ֽ���--->�����ֽ�����PORT

    /*�ͻ�������connectһ��socket*/
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
