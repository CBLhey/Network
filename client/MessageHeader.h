#pragma once

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