#pragma once
#include "Socket_Client.h"
#include "base64.hpp"
#include "Interface.h"

enum WS_Status
{
	WS_STATUS_CONNECT = 0,
	WS_STATUS_UNCONNECT = 1,
};

enum WS_FrameType
{
	WS_EMPTY_FRAME = 0xF0,
	WS_ERROR_FRAME = 0xF1,
	WS_TEXT_FRAME = 0x01,
	WS_BINARY_FRAME = 0x02,
	WS_PING_FRAME = 0x09,
	WS_PONG_FRAME = 0x0A,
	WS_OPENING_FRAME = 0xF3,
	WS_CLOSING_FRAME = 0x08
};



class WebSocket_Client 
{
public:
	WebSocket_Client();
	~WebSocket_Client();

	bool ConnectToServer();

	bool WS_ShakeHand();

	int WS_DecodeFrame(std::string inFrame, std::string& outMessage);
	int WS_EncodeFrame(std::string inMessage, std::string& outFrame, enum WS_FrameType frameType);
	void WS_onSendMessage(std::string message);
	void WS_onRecvMessage(std::string recvmsg);

	void WS_Close();

	SocketClient* GetSocketClient();

	void RegisterRecvHandle(WebSocket_Base* recvHandle);

protected:
	std::string CreateSecret();

private:
	SocketClient* m_client;
	WebSocket_Base* m_recvHandle;
};

