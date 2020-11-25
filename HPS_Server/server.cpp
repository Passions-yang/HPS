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

using namespace std;
// 上面是windows 系统引用动态库，也可以在链接库中指定

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_NEW_USER_JOIN_RESULT,
	CMD_EXIT,
	CMD_EXIT_RESULT,
	CMD_ERROR,
	CMD_ERROR_RESULT
};
struct hps_data_header
{
	short cmd;
	int datalen;
};

struct hps_login:public hps_data_header
{
	hps_login() {
		cmd = CMD_LOGIN;
		datalen = sizeof(hps_login);
		memset(user,0x00,sizeof(user));
		memset(password, 0x00, sizeof(password));
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
	if (ret < 0) {
		printf("The client exits and the task ends fd: %d\n", fd);
		return -1;
	}
	switch (header.cmd) {
		case CMD_LOGIN:
		{
			hps_login login;
			ret = recv(fd, (char *)&login + sizeof(header), sizeof(login) - sizeof(header), 0);
			if (ret < 0) {
				printf("recv is err\n");
			}
			else if (0 == ret) {
				printf("client is close\n");
				return -1;
			}
			printf("login CMD_LOGIN : %d,datalen: %d, username: %s, password: %s\n",
				login.cmd, login.datalen, login.user, login.password);
			hps_login_result result;
			send(fd, (char *)&result, sizeof(result), 0);
		}
		break;
		case CMD_LOGOUT:
		{
			hps_logout logout;
			ret = recv(fd, (char *)&logout + sizeof(header), sizeof(logout) - sizeof(header), 0);
			if (ret < 0) {
				printf("recv is err\n");
			}
			printf("logout CMD_LOGOUT : %d,datalen: %d, username: %s\n",
				logout.cmd, logout.datalen, logout.user);
			hps_logout_result result;
			send(fd, (char *)&result, sizeof(result), 0);
		}
		break;
		default:
		{
			hps_error error;
			ret = recv(fd, (char *)&error + sizeof(header), sizeof(error) - sizeof(header), 0);
			if (ret < 0) {
				printf("recv is err\n");
			}
			printf("error cmd_error : %d,datalen: %d, err_cmd: %s\n",
				error.cmd, error.datalen, error.err_cmd);
			hps_error_result result;
			send(fd, (char *)&result, sizeof(result), 0);
		}
		break;
	}
	return 0;
}
int main(int argc, char * argv[])
{
#ifdef _WIN32
	WORD ver = MAKEWORD(2,2);
	WSADATA data;
	WSAStartup(ver,&data);//在动态库中调用了winsoket的实现
	//由于WSAStartup函数是在动态库中，所以必须引用windows动态库
	// 一下编写socket代码
	//创建socket
#endif
	SOCKET _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	// 绑定地址
	struct sockaddr_in _addr;
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(2020);
#ifdef _WIN32
	_addr.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
#else
	_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
#endif
	if (SOCKET_ERROR == bind(_sock, (const struct sockaddr *)&_addr, sizeof(struct sockaddr_in))){
		printf(" bind is err\n");
#ifdef _WIN32
		closesocket(_sock);
		WSACleanup();
		return -1;
#else
		close(_sock);
		return -1;
#endif //_WIN32
	}
	else {
		printf("bind is success\n");
	}
	// 监听端口
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("listen is ERR\n");
#ifdef _WIN32
		closesocket(_sock);
		WSACleanup();
		return -1;
#else
		close(_sock);
		return -1;
#endif //_WIN32
	}
	else{
		printf("listen is success fd : %d\n",(int)_sock);
	}

	struct sockaddr_in client_addr;
	memset(&client_addr,0x00,sizeof(client_addr));
	socklen_t socklen = sizeof(struct sockaddr_in);
	int max_fd = (int)_sock;
	fd_set read_fds;
	fd_set write_fds;
	fd_set except_fds;
	vector<int> fds_set;
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
		int count = select(max_fd + 1,&read_fds,&write_fds,&except_fds,&tv);
		if(count < 0){
			printf("select is err\n");
			continue;
		}
		if (FD_ISSET(_sock,&read_fds)) {
			// 判断是否是 connect 连接
			int connect_sock = (int)INVALID_SOCKET;
			connect_sock = (int)accept(_sock, (struct sockaddr *)&client_addr, (socklen_t *)&socklen);
			if (SOCKET_ERROR == connect_sock) {
				printf("accept is ERR\n");
			}
			printf(" A new connection is created，ip : %s，fd : %d\n", inet_ntoa(client_addr.sin_addr), connect_sock);
			max_fd = max_fd > connect_sock ? max_fd : connect_sock;
			hps_new_user_join new_user;
			new_user.sock = connect_sock;
			for (unsigned int i = 0; i < fds_set.size(); i++) {
				send(fds_set[i], (char *)&new_user, sizeof(new_user), 0);
			}
			// 将新链接的socket 添加到监听vector
			fds_set.push_back(connect_sock);
		}
		for (unsigned int i = 0; i < fds_set.size(); i++) {
			if (FD_ISSET(fds_set[i], &read_fds)) {
				printf("Processing request for fd = %d\n", fds_set[i]);
				if (-1 == processor(fds_set[i])) {
					// 如果出错，那么说明socket已经关闭，此时需要将 vector中的socket fd移除
					// 将 select监听队列中的数据移除
					FD_CLR(fds_set[i], &read_fds);
					auto iter = fds_set.begin() + i;
					if (iter != fds_set.end()){
						printf("delete fd = %d\n", iter);
						fds_set.erase(iter);
					}
				}
			}
		}
		printf("Other tasks in free time...\n");
	}
#ifdef _WIN32
	for (unsigned int i = 0; i < fds_set.size(); i++) {
		closesocket(fds_set[i]);
	}
	closesocket(_sock);
	WSACleanup();
#else
	for (unsigned int i = 0; i < fds_set.size(); i++) {
		close(fds_set[i]);
	}
	close(_sock);
#endif
	getchar();
	return 0;
}


