#define WIN32_LEAN_AND_MEAN //屏蔽了很多原有的定义，因为下面两个头文件冲突
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa 在新的socket通信中要使用inet_ntoa，就必须声明此宏
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <thread>

using namespace std;
#pragma comment(lib,"ws2_32.lib") 

static bool g_exit_flag = true;

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_NEW_USER_JOIN_RESULT,
	CMD_ERROR,
	CMD_ERROR_RESULT
};
struct hps_data_header
{
	short cmd;
	int datalen;
};

struct hps_login :public hps_data_header
{
	hps_login() {
		cmd = CMD_LOGIN;
		datalen = sizeof(hps_login);
	}
	char user[32];
	char password[32];
};
struct hps_login_result :public hps_data_header
{
	hps_login_result() {
		cmd = CMD_LOGIN_RESULT;
		datalen = sizeof(hps_login_result);
		result = 0;
	}
	int result;
};

struct hps_logout :public hps_data_header
{
	hps_logout() {
		cmd = CMD_LOGOUT;
		datalen = sizeof(hps_logout);
	}
	char user[32];
};
struct hps_logout_result :public hps_data_header
{
	hps_logout_result() {
		cmd = CMD_LOGOUT_RESULT;
		datalen = sizeof(hps_logout_result);
		result = 0;
	}
	int result;
};
struct hps_error :public hps_data_header
{
	hps_error() {
		cmd = CMD_ERROR;
		datalen = sizeof(hps_error);
	}
	char err_cmd[32];
};
struct hps_error_result :public hps_data_header
{
	hps_error_result() {
		cmd = CMD_ERROR_RESULT;
		datalen = sizeof(hps_error_result);
		result = 0;
	}
	int result;
};
struct hps_new_user_join :public hps_data_header
{
	hps_new_user_join() {
		cmd = CMD_NEW_USER_JOIN;
		datalen = sizeof(hps_new_user_join);
		sock = 0;
	}
	int sock;
};
struct hps_new_user_join_result :public hps_data_header
{
	hps_new_user_join_result() {
		cmd = CMD_NEW_USER_JOIN_RESULT;
		datalen = sizeof(hps_new_user_join_result);
		result = 0;
	}
	int result;
};

static int processor(int fd)
{
	int ret = 0;
	hps_data_header header;
	memset(&header, 0x00, sizeof(header));
	ret = recv(fd, (char *)&header, sizeof(header), 0);
	if (ret <= 0) {
		printf("客户端退出，任务结束 fd: %d\n", fd);
		return -1;
	}
	switch (header.cmd) {
		case CMD_LOGIN_RESULT:
		{
			hps_login_result result;
			ret = recv(fd, (char *)&result + sizeof(header), sizeof(result) - sizeof(header), 0);
			if (ret < 0) {
				printf("recv is err\n");
			}
			printf("login CMD_LOGIN_RESULT  result: %d\n",result.result);
		}
		break;
		case CMD_LOGOUT_RESULT:
		{
			hps_logout_result result;
			ret = recv(fd, (char *)&result + sizeof(header), sizeof(result) - sizeof(header), 0);
			if (ret < 0) {
				printf("recv is err\n");
			}
			printf("logout CMD_LOGOUT_RESULT  result: %d\n", result.result);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			hps_new_user_join result;
			ret = recv(fd, (char *)&result + sizeof(header), sizeof(result) - sizeof(header), 0);
			if (ret < 0) {
				printf("recv is err\n");
			}
			printf("user join CMD_NEW_USER_JOIN  sock: %d\n", result.sock);
		}
		break;
	}
	return 0;
}
void cmd_thread(int fd)
{
	char cmd[256] = {0};
	while (NULL != fgets(cmd, sizeof(cmd), stdin)) {
		if (0 == strncmp(cmd, "exit",strlen("exit"))) {
			g_exit_flag = false;
			printf("客户端线程退出\n");
			break;
		}
		if (0 == strncmp(cmd, "login",strlen("login"))) {
			hps_login login;
			strcpy(login.user,"yangzhichao");
			strcpy(login.password, "sunyan");
			send(fd,(char *)&login,sizeof(login),0);
		}
	}
}
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

	int max_fd = _sock;
	fd_set read_fds;
	fd_set write_fds;
	fd_set except_fds;
	struct timeval tv = {1,0};

	thread td(cmd_thread,_sock);
	td.detach();

	while (g_exit_flag) {
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);
		FD_ZERO(&except_fds);
		FD_SET(_sock, &read_fds);
		FD_SET(_sock, &read_fds);
		FD_SET(_sock, &read_fds);
		int ret = select(max_fd + 1,&read_fds,&write_fds,&except_fds,&tv);
		if (ret < 0) {
			printf("select is err\n");
			continue;
		}
		if (FD_ISSET(_sock,&read_fds)) {
			if (-1 == processor(_sock)) {
				printf(" 客户端已断开\n");
				break;
			}
			FD_CLR(_sock,&read_fds);
		}
		printf("空闲时间处理其他事务\n");
	}
	closesocket(_sock);
	WSACleanup();
	getchar();
	return 0;
}


