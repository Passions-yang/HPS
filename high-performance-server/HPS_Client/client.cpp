#define WIN32_LEAN_AND_MEAN //�����˺ܶ�ԭ�еĶ��壬��Ϊ��������ͷ�ļ���ͻ
#define _WINSOCK_DEPRECATED_NO_WARNINGS //inet_ntoa ���µ�socketͨ����Ҫʹ��inet_ntoa���ͱ��������˺�
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib") 


int main(int argc, char * argv[])
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA data;
	WSAStartup(ver, &data);//�ڶ�̬���е�����winsoket��ʵ��
						   //����WSAStartup�������ڶ�̬���У����Ա�������windows��̬��
						   // һ�±�дsocket����
						   //����socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	// �󶨵�ַ
	struct sockaddr_in _addr;
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(2020);
	_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	
	int ret = connect(_sock,(struct sockaddr *)&_addr,sizeof(struct sockaddr_in));
	if (SOCKET_ERROR == ret) {
		printf(" connect ���ӷ����ʧ��\n");
		closesocket(_sock);
	}
	else {
		printf(" connect ���ӷ���˳ɹ�\n");
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


