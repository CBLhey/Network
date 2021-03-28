#pragma once

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS
	#include <WinSock2.h>
	#include <windows.h>
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>
	#define SOCKET  int
	#define INVALID_SOCKET (SOCKET)(~0)
	#define INVALID_ERROR 		   (~1)
#endif 
#include <iostream>
#include "MessageHeader.h"


class EasyTcpClient
{
public:
	SOCKET _sock;
public:
	EasyTcpClient();
	virtual ~EasyTcpClient();
	int initSocket();
	int connectSocket(const char *ip, unsigned short port);
	void closeSocket();
	bool onRun();
	bool isRun();
	int recvData();
	void onNetMsg(DataHeader *dh, char* msgBuf, int len);
	int sendMsg(DataHeader *dh);

private:

};
