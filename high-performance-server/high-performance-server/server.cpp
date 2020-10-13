#define WIN32_LEAN_AND_MEAN //屏蔽了很多原有的定义，因为下面两个头文件冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa 在新的socket通信中要使用inet_ntoa，就必须声明此宏
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

//#pragma comment(lib,"ws2_32.lib") 
// 上面是windows 系统引用动态库，也可以在链接库中指定

int main(int argc, char * argv[])
{
	WORD ver = MAKEWORD(2,2);
	WSADATA data;
	WSAStartup(ver,&data);//在动态库中调用了winsoket的实现
	//由于WSAStartup函数是在动态库中，所以必须引用windows动态库
	// 一下编写socket代码
	//创建socket
	SOCKET _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	// 绑定地址
	struct sockaddr_in _addr;
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(2020);
	_addr.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	if (SOCKET_ERROR == bind(_sock, (const struct sockaddr *)&_addr, sizeof(struct sockaddr_in))){
		printf(" bind is err\n");
		closesocket(_sock);
	}
	else {
		printf("bind is success\n");
	}
	// 监听端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("listen is ERR\n");
		closesocket(_sock);
	}
	else{
		printf("listen is success\n");
	}
	char  * msg = "hello client\n";
	struct sockaddr_in client_addr;
	int socklen_t = sizeof(struct sockaddr_in);
	SOCKET connect_sock = INVALID_SOCKET;
	while (true) {
		connect_sock = accept(_sock, (struct sockaddr *)&client_addr, &socklen_t);
		 if (SOCKET_ERROR == connect_sock){
		 printf("accept is ERR\n");
			closesocket(_sock);
		}
		printf(" 新的连接被创建，ip : %s\n",inet_ntoa(client_addr.sin_addr));
		send(connect_sock, msg,strlen(msg) + 1,0);
		
	}
	closesocket(connect_sock);
	closesocket(_sock);
	WSACleanup();
	return 0;
}


