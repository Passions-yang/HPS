#ifndef _HPS_TCP_CLIENT_H
#define _HPS_TCP_CLIENT_H
#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN //屏蔽了很多原有的定义，因为下面两个头文件冲突
	#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa 在新的socket通信中要使用inet_ntoa，就必须声明此宏
	#define _CRT_SECURE_NO_WARNINGS
	#include <windows.h>
	#include <WinSock2.h>
	#pragma comment(lib,"ws2_32.lib") 
#elif _WIN64
	#define WIN32_LEAN_AND_MEAN //屏蔽了很多原有的定义，因为下面两个头文件冲突
	#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa 在新的socket通信中要使用inet_ntoa，就必须声明此宏
	#define _CRT_SECURE_NO_WARNINGS
	#include <windows.h>
	#include <WinSock2.h>
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
#ifdef _WIN64
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hps_message_header.hpp"

class hps_tcp_common
{
	SOCKET _sock;
public:
	// 初始化相关资源
	hps_tcp_common()
	{
		_sock = INVALID_SOCKET;
	}
	// 释放资源
	virtual ~hps_tcp_common()
	{
		hps_close();
	}
	//初始化socket
	SOCKET hps_init_socket()
	{
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA data;
		WSAStartup(ver, &data);
#endif //_WIN32
		_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (_sock == SOCKET_ERROR) {
			printf("create socket is err\n");
		}
		return _sock;
	}
	//连接服务端
	int hps_connect(const char *ip, unsigned int port)
	{
		if (!hps_is_run()) {
			hps_init_socket();
		}
		// 绑定地址
		struct sockaddr_in _addr;
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(port);
#ifdef _WIN32
		_addr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_addr.sin_addr.s_addr = inet_addr(ip);
#endif //_WIN32
		int ret = connect(_sock, (struct sockaddr *)&_addr, sizeof(struct sockaddr_in));
		if (SOCKET_ERROR == ret) {
			printf(" connect err\n");
			hps_close();
		}
		else {
			printf("connect success\n");
		}
		return ret;
	}
	//处理小心，关闭文件描述符
	void hps_close()
	{
		if (_sock != INVALID_SOCKET) {
			
#ifdef _WIN32
			printf("close fd : %lu\n", _sock);
			closesocket(_sock);
			_sock = INVALID_SOCKET;
			WSACleanup();
#elif _WIN64
			printf("close fd : %llu\n", _sock);
#else
			close(_sock);
			_sock = INVALID_SOCKET;
			printf("close fd : %d\n", _sock);
#endif //_WIN32
		}
	}
	// 判断是否连接
	int hps_is_run()
	{
		return _sock != INVALID_SOCKET;
	}
	// 发送数据
	// 接收数据
	// 处理数据
	bool hps_on_select()
	{
		if (hps_is_run()) {
			fd_set read_fds;
			FD_ZERO(&read_fds);
			FD_SET(_sock, &read_fds);
			struct timeval tv = { 1,0 };
			int ret = select((SOCKET)_sock + 1, &read_fds, NULL, NULL, &tv);
			if (ret < 0) {
#if defined(_WIN32) || defined(_WIN64)
				printf("<socket= %llu>select is err\n", _sock);
#else
				printf("<socket= %d>select is err\n", _sock);
#endif
				return false;
			}
			if (FD_ISSET(_sock, &read_fds)) {
				if (-1 == hps_recv_data()) {
					printf(" Client disconnected\n");
					FD_CLR(_sock, &read_fds);
					return false;
				}
				FD_CLR(_sock, &read_fds);
			}
		}
		return true;
	}
	//此处后续需要处理粘包问题，也是接收数据的接口
	int hps_recv_data() 
	{
		int ret = 0;
		char recv_buff[1024];
		hps_data_header *header = NULL;
		memset(recv_buff,0x00,sizeof(recv_buff));
		ret = recv(_sock, recv_buff, sizeof(hps_data_header), 0);
		if (ret <= 0) {
#if defined(_WIN32) || defined(_WIN64)
			printf("The client exits and the task ends fd: %llu\n", _sock);
#else
			printf("The client exits and the task ends fd: %d\n", _sock);
#endif
			return -1;
		}
		header = (hps_data_header *)recv_buff;
		recv(_sock, recv_buff + sizeof(hps_data_header), header->datalen - sizeof(hps_data_header), 0);
		// 利用指针和子类继承的特性，将数据传下去。
		hps_response_net_massage(header);
		return 0;
	}
	//真正的处理消息
	void hps_response_net_massage(hps_data_header *header)
	{
		switch (header->cmd) {
			case CMD_LOGIN_RESULT:
			{
				hps_login_result *result = (hps_login_result *)header;
				printf("login CMD_LOGIN_RESULT  result: %d\n", result->result);
			}
			break;
			case CMD_LOGOUT_RESULT:
			{
				hps_logout_result *result = (hps_logout_result *)header;
				printf("logout CMD_LOGOUT_RESULT  result: %d\n", result->result);
			}
			break;
			case CMD_NEW_USER_JOIN:
			{
				hps_new_user_join *result = (hps_new_user_join *)header;
				printf("user join CMD_NEW_USER_JOIN  sock: %d\n", result->sock);
			}
			break;
		}
	}
	int hps_send_data(hps_data_header *header)
	{
		if (hps_is_run() && header) {
			return send(_sock,(char *)header,header->datalen,0);
		}
		return SOCKET_ERROR;
	}
private:

};

#endif // _HPS_TCP_CLIENT_H
