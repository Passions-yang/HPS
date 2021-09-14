#include "hps_tcp_client.hpp"
#include <pthread.h>
#include <thread>


using namespace std;
int g_exit_flag = false;

void cmd_thread(hps_tcp_common *client)
{
	char cmd[256] = {0};
	while (NULL != fgets(cmd, sizeof(cmd), stdin)) {
		if (0 == strncmp(cmd, "exit", strlen("exit"))) {
			g_exit_flag = true;
			printf("Client thread exit\n");
			break;
		}
	}
}
int main(int argc, char * argv[])
{
	hps_tcp_common *pclient = new hps_tcp_common[60];
//	hps_tcp_common client1;
//	client.hps_init_socket();
	for (int i = 0; i < 60; i++)
	{
		pclient[i].hps_init_socket();
		pclient[i].hps_connect("127.0.0.1", 2020);
	}
	thread td(cmd_thread,&pclient[0]);
	td.detach();

//	client1.hps_connect("192.168.31.35", 2021);
//	thread td1(cmd_thread, &client1);
//	td1.detach();
	hps_login login;
	strcpy(login.user, "yangzhichao");
	strcpy(login.password, "sunyan");

	while (!g_exit_flag) {
		//client.hps_on_select();
		for (int i = 0; i < 60; i++){
			pclient[i].hps_send_data((hps_data_header *)&login);
		}
	}
	for(int i = 0; i < 60; i++){
		pclient[i].hps_close();
	}
	getchar();
	return 0;
}


