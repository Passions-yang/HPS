#include "hps_tcp_server.hpp"



// ������windows ϵͳ���ö�̬�⣬Ҳ���������ӿ���ָ��

int main(int argc, char * argv[])
{
	hps_socket_common server;
	server.hps_init_socket();
	server.hps_bind("127.0.0.1",2020);
	server.hps_listen(1024);
	while (server.hps_is_run()) {
		server.hps_on_select();
	}
	getchar();
	return 0;
}


