#include <iostream>
#include <string>
//#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include "base64.hpp"
#include <ctime>
//#include <WinSock2.h>
#include "Socket_Client.h"
#include "WebSocket_Client.h"
#include "RecvHandle.h"

using namespace std;

//#pragma comment(lib,"ws2_32.lib")

string CreateSecret()
{
	string key;
	srand(time(0));
	
	for (int i = 0; i < 16; i++)
	{
		int value = rand();
		key += (char)(value % 256);
	}
	return websocketpp::base64_encode(key);
}

void BuildConnect(SocketClient* client)
{
	string message;
	string host;
	message = "GET /HTTP/1.1\r\n";
	message += "Upgrade: websocket\r\n";
	message += "Connection: Upgrade\r\n";
	char temp[512] = { 0 };
	sprintf(temp, "Host: %s \r\n", host);
	message += "Host: localhost:1234\r\n";
//	message += "Origin: \r\n";
	message += "Sec-WebSocket-Key: "; 
//	message += CreateSecret();
	message += "\r\n";
	message += "Sec-WebSocket-Version: 13\r\n\r\n";

	string message2 = "GET / HTTP/1.1\r\nHost: localhost:60000\r\nConnection: Upgrade \r\nUpgrade: websocket\r\nSec-WebSocket-Key: ";
	message2 += CreateSecret();
	message2 += "\r\nSec-WebSocket-Version: 13\r\n\r\n";

	client->onSendMessage(message2);
}

int main()
{
//	SocketClient client("127.0.0.1", 1234);
//	client.Connect();
	string message;
//	BuildConnect(&client);

	WebSocket_Client client;
	RecvHandle* recvHandle = new RecvHandle();
	client.RegisterRecvHandle(recvHandle);
	client.ConnectToServer();
	

	while (1)
	{
		cin >> message;
		if (message == "quit")
			break;

		client.WS_onSendMessage(message);
	}
	delete recvHandle;
	recvHandle = NULL;

	return 0;
}