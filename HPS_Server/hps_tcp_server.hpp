#ifndef _HPS_TCP_SERVER_H
#define _HPS_TCP_SERVER_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN //屏蔽了很多原有的定义，因为下面两个头文件冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa 在新的socket通信中要使用inet_ntoa，就必须声明此宏
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#define socklen_t int
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#define SOCKET	int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif /* _WIN32 */

#include <stdio.h>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include "hps_message_header.hpp"
using namespace std;

class hps_socket_common
{
private:
	SOCKET _sock;
	vector<int> fds_set;
public:
	hps_socket_common()
	{
		_sock = INVALID_SOCKET;
	}
	virtual ~hps_socket_common()
	{
		hps_close();
	}
	// 初始化socket
	SOCKET hps_init_socket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);//在动态库中调用了winsoket的实现
							   //由于WSAStartup函数是在动态库中，所以必须引用windows动态库
							   // 一下编写socket代码
							   //创建socket
#endif
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (_sock < 0) {
			printf("socket create is err \n");
			return INVALID_SOCKET;
		}
		return _sock;
	}
	// bind socket
	int hps_bind(const char *ip,unsigned int short port)
	{
		// 绑定地址
		struct sockaddr_in _addr;
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(port);
		if (NULL == ip) {
#if _WIN32 || _WIN64
			_addr.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
#else
			_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
#endif
		}
		else {
#ifdef _WIN32
			_addr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
			_addr.sin_addr.s_addr = inet_addr(ip);
#endif
		}
		if (SOCKET_ERROR == bind(_sock, (const struct sockaddr *)&_addr, sizeof(struct sockaddr_in))) {
			printf(" bind is err\n");
			hps_close();
		}
		else {
			printf("bind is success\n");
		}
		return 0;
	}
	// listen socket
	int hps_listen(int backlog)
	{
		// 监听端口
		if (SOCKET_ERROR == listen(_sock, backlog)) {
			printf("listen is ERR\n");
			hps_close();
		}
		else {
			printf("listen is success fd : %d\n", (int)_sock);
		}
		return 0;
	}
	// accept socket
	int hps_accept()
	{
		struct sockaddr_in client_addr;
		memset(&client_addr, 0x00, sizeof(client_addr));
		socklen_t socklen = sizeof(struct sockaddr_in);
		int connect_sock = (int)INVALID_SOCKET;
		connect_sock = (int)accept(_sock, (struct sockaddr *)&client_addr, (socklen_t *)&socklen);
		if (SOCKET_ERROR == connect_sock) {
			printf("accept is ERR\n");
		}
		printf(" A new connection is created，ip : %s，fd : %d\n", inet_ntoa(client_addr.sin_addr), connect_sock);
		return connect_sock;
	}
	bool hps_on_select()
	{
		if (!hps_is_run()) {
			printf("sockect is no create,please create socket\n");
			return false;
		}
		int max_fd = (int)_sock;
		fd_set read_fds;
		fd_set write_fds;
		fd_set except_fds;
		while (true) {
			FD_ZERO(&read_fds);
			FD_ZERO(&write_fds);
			FD_ZERO(&except_fds);
			FD_SET(_sock, &read_fds);
			FD_SET(_sock, &write_fds);
			FD_SET(_sock, &except_fds);
			struct timeval tv = { 1,0 };
			for (unsigned int i = 0; i < fds_set.size(); i++) {
				printf(" set fd : %d\n", fds_set[i]);
				FD_SET(fds_set[i], &read_fds);
			}
			int count = select(max_fd + 1, &read_fds, &write_fds, &except_fds, 0);
			if (count < 0) {
				printf("select is err\n");
				continue;
			}
			if (FD_ISSET(_sock, &read_fds)) {
				// 判断是否是 connect 连接
				int connect_sock = hps_accept();
				max_fd = max_fd > connect_sock ? max_fd : connect_sock;
				hps_send_to_all_data(connect_sock);
				// 将新链接的socket 添加到监听vector
				fds_set.push_back(connect_sock);
			}
			for (unsigned int i = 0; i < fds_set.size(); i++) {
				if (FD_ISSET(fds_set[i], &read_fds)) {
					printf("Processing request for fd = %d\n", fds_set[i]);
					if (-1 == hps_recv_data(fds_set[i])) {
						// 如果出错，那么说明socket已经关闭，此时需要将 vector中的socket fd移除
						// 将 select监听队列中的数据移除
						FD_CLR(fds_set[i], &read_fds);
						auto iter = fds_set.begin() + i;
						if (iter != fds_set.end()) {
							printf("delete fd = %d\n", *iter);
							fds_set.erase(iter);
						}
					}
				}
			}
			printf("Other tasks in free time...\n");
		}
	}
	// 接收数据
	int hps_recv_data(int fd)
	{
		int ret = 0;
		char recv_buff[4096] = {0};
		hps_data_header *header = NULL;
		memset(recv_buff, 0x00, sizeof(recv_buff));
		ret = recv(fd, (char *)recv_buff, sizeof(hps_data_header), 0);
		if (ret <= 0) {
			printf("The server exits and the task ends fd: %d\n", fd);
			return -1;
		}
		header = (hps_data_header *)recv_buff;
		ret = recv(fd, recv_buff + sizeof(hps_data_header), header->datalen - sizeof(hps_data_header), 0);
		if (ret < 0) {
			printf("recv is ERR\n");
			memset(recv_buff, 0x00, sizeof(recv_buff));
			return -2;
		}
		// 利用指针和子类继承的特性，将数据传下去。
		hps_response_net_massage(fd,header);
		return 0;
	}
	// 处理数据
	void hps_response_net_massage(int fd,hps_data_header *header)
	{
		switch (header->cmd) {
		case CMD_LOGIN:
		{
			hps_login *login = (hps_login *)header;
			printf("login CMD_LOGIN : %d, username: %s,datalen: %d\n",
				login->cmd,  login->user, login->datalen);
			hps_login_result result;
			//hps_send_data(fd, (hps_data_header *)&result);
		}
		break;
		case CMD_LOGOUT:
		{
			hps_logout *logout = (hps_logout *)header;
			printf("logout CMD_LOGOUT : %d,datalen: %d, username: %s\n",
				logout->cmd, logout->datalen, logout->user);
			hps_logout_result result;
			//hps_send_data(fd, (hps_data_header *)&result);
		}
		break;
		default:
		{
			hps_error *error = (hps_error *)header;
			printf("error cmd_error : %d,datalen: %d, err_cmd: %s\n",
				error->cmd, error->datalen, error->err_cmd);
			hps_error_result result;
			//hps_send_data(fd, (hps_data_header *)&result);
		}
		break;
		}
	}
	// 判断是否运行
	bool hps_is_run()
	{
		return _sock != INVALID_SOCKET;
	}
	// 发送数据
	int hps_send_data(int sock, hps_data_header *header)
	{
		if (hps_is_run()) {
			return send(sock, (const char *)header, header->datalen, 0);
		}
		else {
			printf("sockect is no create,please create socket\n");
		}
		return -1;
	}
	// 群发数据
	void hps_send_to_all_data(int fd)
	{
		hps_new_user_join new_user;
		new_user.sock = fd;
		for (unsigned int i = 0; i < fds_set.size(); i++) {
			hps_send_data(fds_set[i], (hps_data_header *)&new_user);
		}
	}
	// close socket
	void hps_close() {
		if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
			printf("close fd : %llu\n", _sock);
			closesocket(_sock);
			_sock = INVALID_SOCKET;
			WSACleanup();
#else
			printf("close fd : %d\n", _sock);
			close(_sock);
			_sock = INVALID_SOCKET;
#endif //_WIN32
		}
	}
	void hps_close_all() {
#ifdef _WIN32
		for (unsigned int i = 0; i < fds_set.size(); i++) {
			closesocket(fds_set[i]);
			fds_set[i] = INVALID_SOCKET;
		}
#else
		for (unsigned int i = 0; i < fds_set.size(); i++) {
			close(fds_set[i]);
			fds_set[i] = INVALID_SOCKET;
		}
#endif
		hps_close();
	}
};

#endif //_HPS_TCP_SERVER_H
