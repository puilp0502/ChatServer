#pragma comment(lib, "ws2_32.lib")
#include "stdafx.h"
#include "MTQueue.h"
#include <WinSock2.h>
#include <conio.h>
#include <process.h>

#define DEFAULT_SOCKET 20248
#define MAX_RETRY 3
#define MAX_CLIENT 1024

using namespace std;

unsigned WINAPI ListenThread(void* args);
unsigned WINAPI SocketThread(void* args);

HANDLE hClientThreads[MAX_CLIENT];
int threadCount;
bool running;

Queue<string> queue = Queue<string>();

int main(){
	running = true;
	string cmd;
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, ListenThread, NULL, 0, NULL);
	while (running){
		cin >> cmd;
		if (cmd == "quit"){
			running = false;
			_endthreadex(0);
			CloseHandle(handle);
		}

	}
	WaitForSingleObject(handle, INFINITE);
}

unsigned WINAPI ListenThread(void* args){
	WSADATA wsaData;
	sockaddr_in server;
	int temp_ret;
	if ((temp_ret = WSAStartup(MAKEWORD(2,2), &wsaData)) != 0){
		cout << "Error occured while initializing winsock. Terminating...";
		
		return 0;
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(DEFAULT_SOCKET);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKET new_socket;
	if (sock == INVALID_SOCKET)
		return 0;
	if (bind(sock, (sockaddr*)&server, sizeof(server)) != 0)
		return 0;
	cout << "Binding succeded." << endl;
	if (listen(sock, 3) != 0)
		return 0;
	cout << "Socket set to mode listen." << endl;
	SOCKET client;
	sockaddr_in from;
	int fromlen = sizeof(from);
	while (running){
		
		int c = sizeof(struct sockaddr_in);
		SOCKET new_socket = accept(sock, (struct sockaddr *)&client, &c);
		if (new_socket == INVALID_SOCKET)
		{
			cout << "Accept failed with error code %d" << WSAGetLastError();
		}
		hClientThreads[threadCount++] = (HANDLE)_beginthreadex(NULL, 0, SocketThread, (void*)new_socket, 0, NULL);
		cout << "connected" << endl;
	}
}
unsigned WINAPI SocketThread(void* args){
	SOCKET s = (SOCKET)args;
	const timeval tv = { 0, 0 }; //select 함수가 멈추지 않도록 합니다.
	char buffer[4096];
	DWORD tempVal;
	int retry_count = MAX_RETRY;
	unsigned long long lastMsgId = 0;
	fd_set fs;
	FD_ZERO(&fs);
	string temp;

	while (running){
		while (queue.getLastMsgId() > lastMsgId){
			string entry = queue.getEntry(lastMsgId++, threadCount);
			if (entry != "")
				send(s, entry.c_str(), entry.size(), 0);
		}
		FD_ZERO(&fs);
		FD_SET(s, &fs);
		tempVal = select(0, &fs, NULL, NULL, &tv);
		switch (tempVal){
		case 0: //읽을 수 없음
			break;
		case 1: //select 함수는 fd_set 구조체 안의 사용 가능한 소켓의 개수를 리턴
				//그러므로 1을 리턴했다면 소켓을 읽을 준비가 되었다는 의미임

			if ((tempVal = recv(s, buffer, 4096, 0)) == SOCKET_ERROR){
				while (retry_count--){
					cout << ("error occured") << endl;
					switch ((tempVal = WSAGetLastError())){
					case 10054:
						cout << "connection reset: ";
						cout << to_string(s) << endl;
						return -1;
						break;
					case 10061:
						cout << ("connection refused: ");
						cout << to_string(s) << endl;
						cout << ("try "+to_string(MAX_RETRY - retry_count));
						cout << endl;
						break;
					default:
						/*
						wchar_t error_reason_wchar[1024];
						wstring error_reason;
						string error_reason_string;
						FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, tempVal, MAKELANGID(0x02, 0x02), error_reason_wchar, 1024, NULL);
						error_reason = wstring(error_reason_wchar);
						error_reason_string.assign(error_reason.begin(), error_reason.end());
						cout << (error_reason_string);*/
						break;
					}
				}
			}
			buffer[tempVal] = '\0';
			queue.add(buffer);
			retry_count = MAX_RETRY;
			break;
		default:
			cout << ("[E] 알 수 없는 오류 발생함 : " + to_string(tempVal));
			break;
		}
		Sleep(10);
	}
}