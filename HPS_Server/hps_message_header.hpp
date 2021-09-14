#ifndef _HPS_MESSAGE_HEADER_H
#define _HPS_MESSAGE_HEADER_H
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

struct hps_login :public hps_data_header
{
	hps_login() {
		cmd = CMD_LOGIN;
		datalen = sizeof(hps_login);
		memset(buff, 0x00, sizeof(buff));
	}
	char user[32];
	char password[32];
	char buff[1024];
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

#endif //_HPS_MESSAGE_HEADER_H
