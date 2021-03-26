#include "EasyTcpClient.h"

EasyTcpClient::EasyTcpClient()
{
	_sock = INVALID_SOCKET;
}

EasyTcpClient::~EasyTcpClient()
{
	closeSocket();
}

int EasyTcpClient::initSocket()
{

	if (_sock != INVALID_SOCKET) {
		printf("该sock=%d已被初始话...\n", _sock);
		return -1;
	}
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == INVALID_SOCKET) {
		printf("错误， 创建socket失败.\n");
		return -1;
	}
	else {
		printf("创建socket=%d成功.\n", _sock);
	}
	return 0;
}

int EasyTcpClient::connectSocket(const char * ip, unsigned short port)
{
	if (_sock == INVALID_SOCKET) {
		printf("请初始该soket...\n");
		return -1;
	}
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32 
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif
	printf("正在链接服务器...\n");
	int ret = connect(_sock, (sockaddr *)&_sin, sizeof(sockaddr_in));
	if (ret == INVALID_SOCKET) {
		printf("<socket=%d>selecct任务结束。\n", _sock);
	}
	else {
		printf("链接服务器成功...\n");
	}
	
	return ret;
}

void EasyTcpClient::closeSocket()
{
	if (_sock != INVALID_SOCKET) {
#ifdef _WIN32
		closesocket(_sock);
#else
		close(_sock);
#endif
		_sock = INVALID_SOCKET;
	}

}

bool EasyTcpClient::onRun()
{
	if (isRun()) {
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t;
		t.tv_sec = 1;
		t.tv_usec = 0;
		int ret = select(_sock + 1, &fdReads, 0, 0, &t);
		if (ret < 0) {
			printf("<socket=%d>selecct任务结束。\n", _sock);
			return false;
		}
		if (FD_ISSET(_sock, &fdReads)) {
			FD_CLR(_sock, &fdReads);
			if (-1 == recvData()) {
				printf("<socket=%d>select任务已结束...\n", _sock);
				return false;
			}
		}
		return true;
	}

}

bool EasyTcpClient::isRun()
{
	return _sock != INVALID_SOCKET;
}

int EasyTcpClient::recvData()
{
	char msgBuf[1024];
	int nLen = recv(_sock, (char*)&msgBuf, sizeof(DataHeader), 0);
	if (nLen < 0) {
		printf("与服务器断开链接， 任务结束...\n");
		return -1;
	}
	DataHeader *dh = (DataHeader *)msgBuf;
	int mLen = recv(_sock, (char*)&msgBuf + sizeof(DataHeader), dh->dataLength - sizeof(DataHeader), 0);
	if (mLen >= 0) {
		onNetMsg(dh, msgBuf, mLen);
	}
	return 0;
}

void EasyTcpClient::onNetMsg(DataHeader *dh, char* msgBuf, int len) {
	switch (dh->cmd) {
		case CMD_LOGIN_RESULT: {
			LoginResult *loginResult = (LoginResult *)msgBuf;
			printf("登入返回:%d\n", loginResult->result);
		}
		break;
		case CMD_LOGINOUT_RESULT: {
			LogoutResult *logoutResult = (LogoutResult *)msgBuf;
			printf("登出返回:%d\n", logoutResult->result);
		}
		break;
		case CMD_NEW_UUSE_JOIN: {
			NewUserJoin *userJoin = (NewUserJoin *)msgBuf;
			printf("新客户端<Sock=%d>加入链接\n", userJoin->socketID);
		}
		break;
		default: {
			printf("未知协议..\n");
		}
		 break;
	}
}

int EasyTcpClient::sendMsg(DataHeader *dh){

	if (isRun() && dh) {
		send(_sock, (const char*)dh, dh->dataLength, 0);
		return -1;
	}
	return 0;
}
