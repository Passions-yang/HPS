#define WIN32_LEAN_AND_MEAN //�����˺ܶ�ԭ�еĶ��壬��Ϊ��������ͷ�ļ���ͻ
#include <windows.h>
#include <WinSock2.h>

//#pragma comment(lib,"ws2_32.lib") 
// ������windows ϵͳ���ö�̬�⣬Ҳ���������ӿ���ָ��

int main(int argc, char * argv[])
{
	WORD ver = MAKEWORD(2,2);
	WSADATA data;
	WSAStartup(ver,&data);//�ڶ�̬���е�����winsoket��ʵ��
	//����WSAStartup�������ڶ�̬���У����Ա�������windows��̬��
	// һ�±�дsocket����
	WSACleanup();
	return 0;
}