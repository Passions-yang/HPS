#define WIN32_LEAN_AND_MEAN //�����˺ܶ�ԭ�еĶ��壬��Ϊ��������ͷ�ļ���ͻ
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa ���µ�socketͨ����Ҫʹ��inet_ntoa���ͱ��������˺�
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

//#pragma comment(lib,"ws2_32.lib") 
// ������windows ϵͳ���ö�̬�⣬Ҳ���������ӿ���ָ��

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
		printf(" �µ����ӱ�������ip : %s\n",inet_ntoa(client_addr.sin_addr));
		send(connect_sock, msg,strlen(msg) + 1,0);
		
	}
	closesocket(connect_sock);
	closesocket(_sock);
	WSACleanup();
	return 0;
}


