#define WIN32_LEAN_AND_MEAN //屏蔽了很多原有的定义，因为下面两个头文件冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa 在新的socket通信中要使用inet_ntoa，就必须声明此宏
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib") 


int main(int argc, char * argv[])
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);//在动态库中调用了winsoket的实现
						   //由于WSAStartup函数是在动态库中，所以必须引用windows动态库
						   // 一下编写socket代码
						   //创建socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	// 绑定地址
	struct sockaddr_in _addr;
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(2020);
	_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	int ret = connect(_sock,(struct sockaddr *)&_addr,sizeof(struct sockaddr_in));
	if (SOCKET_ERROR == ret) {
		printf(" connect 连接服务端失败\n");
		closesocket(_sock);
	}
	else {
		printf(" connect 连接服务端成功\n");
	}
	char buff[1024] = {0};
	int retlen = recv(_sock,buff,sizeof(buff),0);
	if (retlen > 0) {
		printf("recv data: %s\n",buff);
	}
	closesocket(_sock);
	WSACleanup();
	getchar();
	return 0;
}


