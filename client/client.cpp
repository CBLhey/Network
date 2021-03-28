#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <thread>
#include "EasyTcpClient.h"

bool g_bRun = true;

void cmdThead(EasyTcpClient *client) {
	while (true) {
		char cmdBuuf[256] = {};
		scanf("%s", cmdBuuf);
		if (0 == strcmp(cmdBuuf, "exit")) {
			g_bRun = false;
			printf("<socket=%d>退出cmdThead线程...\n", client->_sock);
			break;
		}
		else if (0 == strcmp(cmdBuuf, "login")) {
			Login login;
			strcpy(login.userName, "lyd");
			strcpy(login.PassWord, "lyd");
			client->sendMsg(&login);
		}
		else if (0 == strcmp(cmdBuuf, "logout")) {
			Logout logout;
			strcpy(logout.userName, "lyd");
			client->sendMsg(&logout);
		}
		else {
			printf("<socket=%d>未知命令请重新输入...\n", client->_sock);
		}

	}
}

int  main() {

#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

	EasyTcpClient client;
	if (client.initSocket() != 0) {
		return - 1;
	}
	if (client.connectSocket("127.0.0.1", 4567) != 0) {
		return -1;
	}

	EasyTcpClient client2;
	if (client2.initSocket() != 0) {
		return - 1;
	}
	if (client2.connectSocket("127.0.0.1", 4567) != 0) {
		return -1;
	}

	//启动线程
	std::thread t1(cmdThead, &client);
	t1.detach();

	std::thread t2(cmdThead, &client2);
	t2.detach();
	while (g_bRun) {
		if (client.onRun() == false) {
			break;
		}

		if (client2.onRun() == false) {
			break;
		}
		//printf("空闲时间处理其他业务...\n");
	}
	client.closeSocket();
	client2.closeSocket();

#ifdef _WIN32
	WSACleanup();
#endif
	getchar();

	return 0;
}