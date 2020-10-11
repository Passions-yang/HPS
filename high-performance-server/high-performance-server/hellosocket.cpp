#define WIN32_LEAN_AND_MEAN //屏蔽了很多原有的定义，因为下面两个头文件冲突
#include <windows.h>
#include <WinSock2.h>

//#pragma comment(lib,"ws2_32.lib") 
// 上面是windows 系统引用动态库，也可以在链接库中指定

int main(int argc, char * argv[])
{
	WORD ver = MAKEWORD(2,2);
	WSADATA data;
	WSAStartup(ver,&data);//在动态库中调用了winsoket的实现
	//由于WSAStartup函数是在动态库中，所以必须引用windows动态库
	// 一下编写socket代码
	WSACleanup();
	return 0;
}