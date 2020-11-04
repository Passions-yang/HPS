#define WIN32_LEAN_AND_MEAN //�����˺ܶ�ԭ�еĶ��壬��Ϊ��������ͷ�ļ���ͻ
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa ���µ�socketͨ����Ҫʹ��inet_ntoa���ͱ��������˺�
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <vector>
using namespace std;
//#pragma comment(lib,"ws2_32.lib") 
// ������windows ϵͳ���ö�̬�⣬Ҳ���������ӿ���ָ��

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

struct hps_login:public hps_data_header
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
	if (ret < 0) {
		printf("�ͻ����˳���������� fd: %d\n", fd);
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
			else if (ret = 0) {
				printf("client is close\n");
				break;
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
	WORD ver = MAKEWORD(2,2);
	WSADATA data;
	WSAStartup(ver,&data);//�ڶ�̬���е�����winsoket��ʵ��
	//����WSAStartup�������ڶ�̬���У����Ա�������windows��̬��
	// һ�±�дsocket����
	//����socket
	SOCKET _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	// �󶨵�ַ
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
	// �����˿�
	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("listen is ERR\n");
		closesocket(_sock);
	}
	else{
		printf("listen is success fd : %d\n",_sock);
	}
	char  * msg = "hello client\n";
	struct sockaddr_in client_addr;
	memset(&client_addr,0x00,sizeof(client_addr));
	int socklen_t = sizeof(struct sockaddr_in);
	int max_fd = _sock;
	struct timeval tv = { 1,0 };
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
		for (unsigned int i = 0; i < fds_set.size(); i++) {
			FD_SET(fds_set[i], &read_fds);
		}
		int count = select(max_fd + 1,&read_fds,&write_fds,&except_fds,&tv);
		if(count < 0){
			printf("select is err\n");
			continue;
		}
		if (FD_ISSET(_sock,&read_fds)) {
			// �ж��Ƿ��� connect ����
			int connect_sock = INVALID_SOCKET;
			connect_sock = accept(_sock, (struct sockaddr *)&client_addr, &socklen_t);
			if (SOCKET_ERROR == connect_sock) {
				printf("accept is ERR\n");
			}
			printf(" �µ����ӱ�������ip : %s��fd : %d\n", inet_ntoa(client_addr.sin_addr), connect_sock);
			max_fd = max_fd > connect_sock ? max_fd : connect_sock;
			hps_new_user_join new_user;
			new_user.sock = connect_sock;
			for (unsigned int i = 0; i < fds_set.size(); i++) {
				send(fds_set[i], (char *)&new_user, sizeof(new_user), 0);
			}
			// �������ӵ�socket ��ӵ�����vector
			fds_set.push_back(connect_sock);
		}
		for (unsigned int i = 0; i < fds_set.size(); i++) {
			if (FD_ISSET(fds_set[i], &read_fds)) {
				printf("���� fd = %d ������\n", fds_set[i]);
				if (-1 == processor(fds_set[i])) {
					// ���������ô˵��socket�Ѿ��رգ���ʱ��Ҫ�� vector�е�socket fd�Ƴ�
					// �� select���������е������Ƴ�
					FD_CLR(fds_set[i], &read_fds);
					auto iter = find(fds_set.begin(), fds_set.end(), fds_set[i]);
					if (iter != fds_set.end()){
						fds_set.erase(iter);
					}
				}
			}
		}
		printf("����ʱ�䴦����������\n");
	}
	for (unsigned int i = 0; i < fds_set.size(); i++) {
		closesocket(fds_set[i]);
	}
	closesocket(_sock);
	WSACleanup();
	getchar();
	return 0;
}


