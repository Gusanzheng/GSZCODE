#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <netinet/in.h> //�ֽ���ת��,IPV4��ַ�ṹ��sockaddr_in
#include <bits/socket.h> //ͨ�õ�ַ�ṹ��sockaddr
#include <arpa/inet.h> //IP��ַת��
#include <sys/types.h> //ͨ��
#include <sys/socket.h> //ͨ��
#include <unistd.h> //�ر�socket����
#include <iostream>
#include <cerrno>
#include <cstring>
#include <cassert>

const int BUF_SIZE = 1024;

class tcpserver {
public:
	tcpserver() = default;
	~tcpserver() = default;

	void init(int m_port);

public:
	int m_listenfd;
	struct sockaddr_in address;
	char tcp_buffer[BUF_SIZE];
};

#endif // !TCPSERVER_H
