#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include <WinSock2.h>
	#include <windows.h>
#else
	#include <unistd.h>
	#include <arpa/inet.h>
	#include <string.h>
	#define SOCKET  int
	#define INVALID_SOCKET 		(SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include <iostream>
#include <vector>

using namespace std;
enum CMD {
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_UUSE_JOIN,
	CMD_ERROR
};

struct DataHeader {
	short dataLength;
	short cmd;
};

struct Login : public DataHeader {
	Login() {
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];

};

struct LoginResult : public DataHeader {
	LoginResult() {
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
	}
	int result;
};

struct Logout : public DataHeader {
	Logout() {
		dataLength = sizeof(Logout);
		cmd = CMD_LOGINOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader {
	LogoutResult() {
		dataLength = sizeof(LogoutResult);
		cmd = CMD_LOGINOUT_RESULT;
	}
	int result;
};

struct NewUserJoin : public DataHeader {
	NewUserJoin() {
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_UUSE_JOIN;
		socketID = 0;
	}

	int socketID;
};

int  processor(SOCKET _cSock) {

	char szRecv[1024] = {};
	int nLen = recv(_cSock, szRecv, sizeof(DataHeader), 0);
	if (nLen <= 0) {
		printf("客户端<Soket=%d>已退出！\n", _cSock);
		return -1;
	}
	DataHeader  *header = (DataHeader*)szRecv;
	switch (header->cmd) {
	case CMD_LOGIN: {

		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Login *login = (Login*)szRecv;
		printf("收到<Soket=%d>命令:CMD_LOGIN, 收到数据长度:%d, userName:%s, passWord=%s\n", _cSock, login->dataLength, login->userName, login->PassWord);
		LoginResult loginRet = {};
		loginRet.result = 0;
		send(_cSock, (char*)&loginRet, sizeof(LoginResult), 0);
	}
					break;
	case CMD_LOGINOUT: {
		recv(_cSock, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		Logout *logout = (Logout *)szRecv;
		printf("收到<Soket=%d>命令:CMD_LOGINOUT, 收到数据长度:%d, userName:%s\n", _cSock, logout->dataLength, logout->userName);
		LogoutResult logoutRet = {};
		logoutRet.result = 0;
		send(_cSock, (char*)&logoutRet, sizeof(LogoutResult), 0);
	}
					   break;
	default: {
		DataHeader header = {};
		header.cmd = CMD_ERROR;
		header.dataLength = sizeof(DataHeader);
		send(_cSock, (char*)&header, sizeof(DataHeader), 0);
		printf("收到未知协议,踢除客户端<Soket=%d>已退出...\n", _cSock);
		return -1;
	}
					   break;
	}
	return 0;
}

vector<SOCKET> g_clients;

int  main() {

#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif

	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in  _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
	_sin.sin_addr.s_addr = INADDR_ANY;
#endif
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
		printf("错误, 绑定用于接受客户端连接的端口失败！\n");
		return 0;
	}

	if (SOCKET_ERROR == listen(_sock, 5)) {
		printf("错误, 监听网络端口失败！\n");
	}

	printf("监听网络端口成功...\n");

	while (true) {

		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExp;
		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExp);

		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);

		SOCKET maxSock = _sock;
		for (size_t n = 0; n < g_clients.size(); n++) {
			FD_SET(g_clients[n], &fdRead);
			if (maxSock < g_clients[n]) {
				maxSock = g_clients[n];
			}
		}

		timeval t;
		t.tv_sec = 1;
		t.tv_usec = 0;
		int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
		if (ret < 0) {
			printf("selecct任务结束。\n");
			break;
		}

		if (FD_ISSET(_sock, &fdRead)) {
			FD_CLR(_sock, &fdRead);
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr);
			SOCKET _cSock = INVALID_SOCKET;
#ifdef _WIN32 
			_cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
			_cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
			if (_cSock == INVALID_SOCKET) {
				printf("错误，接受到无效的客户端链接！\n");
			}
			else {
				printf("新客户端<Soket=%d>连入：IP =%s \n", _cSock, inet_ntoa(clientAddr.sin_addr));

				NewUserJoin	userJoin = {};
				for (size_t n = 0; n < g_clients.size(); n++) {
					userJoin.socketID = _cSock;
					send(g_clients[n], (const char *)&userJoin, userJoin.dataLength, 0);
				}

				g_clients.push_back(_cSock);
				printf("空闲时间处理其他业务...\n");
			}

		}

		for (int n = (int)g_clients.size() - 1; n >= 0; n--) {
			if (FD_ISSET(g_clients[n], &fdRead)) {
				if (-1 == processor(g_clients[n])) {
#ifdef _WIN32
					closesocket(g_clients[n]);
#else
					close(g_clients[n]);
#endif
					g_clients.erase(g_clients.begin() + n);
				}
			}
		}

		printf("空闲时间处理其他业务...\n");
	}

#ifdef _WIN32
	for (size_t n = 0; n < g_clients.size(); n++) {
		closesocket(g_clients[n]);
	}
	closesocket(_sock);
	WSACleanup();
#else
	for (size_t n = 0; n < g_clients.size(); n++) {
		close(g_clients[n]);
	}
	close(_sock);
#endif

	printf("已退出， 任务结束。\n");
	getchar();
	return 0;
}