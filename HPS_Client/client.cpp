#include "hps_tcp_client.hpp"
#include <thread>


using namespace std;

void cmd_thread(hps_tcp_common *client)
{
	char cmd[256] = {0};
	while (NULL != fgets(cmd, sizeof(cmd), stdin)) {
		if (0 == strncmp(cmd, "exit",strlen("exit"))) {
			client->hps_close();
			printf("Client thread exit\n");
			break;
		}
		if (0 == strncmp(cmd, "login",strlen("login"))) {
			hps_login login;
			strcpy(login.user,"yangzhichao");
			strcpy(login.password, "sunyan");
			client->hps_send_data((hps_data_header *)&login);
		}
	}
}
int main(int argc, char * argv[])
{
	hps_tcp_common client;
	hps_tcp_common client1;
//	client.hps_init_socket();
	client.hps_connect("192.168.31.35",2020);
	thread td(cmd_thread,&client);
	td.detach();

	client1.hps_connect("192.168.31.35", 2021);
	thread td1(cmd_thread, &client1);
	td1.detach();

	while (client.hps_is_run() || client1.hps_is_run()) {
		client.hps_on_select();
		client1.hps_on_select();
	//	printf("空闲时间处理其他事务\n");
	}
	client.hps_close();	
	client1.hps_close();
	getchar();
	return 0;
}


