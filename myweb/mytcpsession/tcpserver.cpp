#include <tcpserver.h>

void tcpserver::init(int m_port) {
	//�1�7�1�7�1�7�1�7�1�7�1�7�1�7�0�9�1�7�1�7�1�7�1�7�1�7�1�7�1�7
	m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(m_listenfd >= 0);

	//{0,1}�1�7�1�7�1�7�1�7�1�7�0�7�0�1�1�9�1�7,{1,1}�1�7�1�7�1�7�0�7�0�1�1�9�1�7,P112
	struct linger tmp = { 0, 1 };
	setsockopt(m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

	/*IPV4�1�7�1�7�0�7�1�7�1�7�1�7�1�7*/
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(m_port);

	//�1�7�1�7�1�7�0�9�1�7�1�7�1�3�1�7�0�7,P107
	int flag = 1;
	setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	int ret = bind(m_listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(m_listenfd, 5);
	assert(ret != -1);

	/*�1�7�1�7�1�7�1�7�1�7�1�7�1�7�1�7�0�3�1�7�1�7*/
	memset(tcp_buffer, '\0', BUF_SIZE - 1);
}