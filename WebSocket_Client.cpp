#include "WebSocket_Client.h"
#include <ctime>
#include <windows.h>
#include <stdint.h>

using namespace std;

WebSocket_Client::WebSocket_Client()
{
	m_client = new SocketClient("127.0.0.1", 1235);
	m_recvHandle = NULL;
}


WebSocket_Client::~WebSocket_Client()
{
	delete m_client;
	m_client = NULL;
}

string WebSocket_Client::CreateSecret()
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

bool WebSocket_Client::WS_ShakeHand()
{
	string message;
	string host;
	message = "GET / HTTP/1.1\r\n";
	message += "Upgrade: websocket\r\n";
	message += "Connection: Upgrade\r\n";
//	char temp[512] = { 0 };
//	sprintf(temp, "Host: %s \r\n", host);
	message += "Host: localhost:1234\r\n";
	//	message += "Origin: \r\n";
	message += "Sec-WebSocket-Key: ";
	message += CreateSecret();
	message += "\r\n";
	message += "Sec-WebSocket-Version: 13\r\n\r\n";

//	string message2 = "GET / HTTP/1.1\r\nHost: localhost:1235\r\nConnection: Upgrade \r\nUpgrade: websocket\r\nSec-WebSocket-Key: ";
//	message2 += CreateSecret();
//	message2 += "\r\nSec-WebSocket-Version: 13\r\n\r\n";

	m_client->onSendMessage(message);
	string recv_message;
	int len = m_client->Recv(recv_message);
	if (len == 0)
	{
		cout << "socket is disconnect ....." << endl;
		return false;
	}
	cout << "recv_message : " << recv_message << endl;
	return true;
}

bool WebSocket_Client::ConnectToServer()
{
	m_client->Connect();
	if (!WS_ShakeHand())
		return false;
	m_client->StartRecvProcess(this);
	return true;
}

int WebSocket_Client::WS_DecodeFrame(std::string inFrame, std::string& outMessage)
{
	int ret = WS_OPENING_FRAME;
	const char *frameData = inFrame.c_str();
	const int frameLength = inFrame.size();
	if (frameLength < 2)
	{
		ret = WS_ERROR_FRAME;
	}

	// 检查扩展位并忽略  
	if ((frameData[0] & 0x70) != 0x0)
	{
		ret = WS_ERROR_FRAME;
	}

	// fin位: 为1表示已接收完整报文, 为0表示继续监听后续报文  
	ret = (frameData[0] & 0x80);
	if ((frameData[0] & 0x80) != 0x80)
	{
		ret = WS_ERROR_FRAME;
	}

	// mask位, 为1表示数据被加密  
	if ((frameData[1] & 0x80) != 0x80)
	{
		ret = WS_ERROR_FRAME;
	}

	// 操作码  
	uint16_t payloadLength = 0;
	uint8_t payloadFieldExtraBytes = 0;
	uint8_t opcode = static_cast<uint8_t>(frameData[0] & 0x0f);
	if (opcode == WS_TEXT_FRAME)
	{
		// 处理utf-8编码的文本帧  
		payloadLength = static_cast<uint16_t>(frameData[1] & 0x7f);
		if (payloadLength == 0x7e)
		{
			uint16_t payloadLength16b = 0;
			payloadFieldExtraBytes = 2;
			memcpy(&payloadLength16b, &frameData[2], payloadFieldExtraBytes);
			payloadLength = ntohs(payloadLength16b);
		}
		else if (payloadLength == 0x7f)
		{
			// 数据过长,暂不支持  
			ret = WS_ERROR_FRAME;
		}
	}
	else if (opcode == WS_BINARY_FRAME || opcode == WS_PING_FRAME || opcode == WS_PONG_FRAME)
	{
		// 二进制/ping/pong帧暂不处理  
	}
	else if (opcode == WS_CLOSING_FRAME)
	{
		ret = WS_CLOSING_FRAME;
	}
	else
	{
		ret = WS_ERROR_FRAME;
	}

	// 数据解码  
	if ((ret != WS_ERROR_FRAME) && (payloadLength > 0))
	{
		// header: 2字节, masking key: 4字节  
		const char *maskingKey = &frameData[2 + payloadFieldExtraBytes];
		char *payloadData = new char[payloadLength + 1];
		memset(payloadData, 0, payloadLength + 1);
		memcpy(payloadData, &frameData[2 + payloadFieldExtraBytes + 4], payloadLength);
		for (int i = 0; i < payloadLength; i++)
		{
			payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
		}

		outMessage = payloadData;
		delete[] payloadData;
	}
	else
	{
		outMessage = inFrame.substr(2);
	}
	return ret;
}

int WebSocket_Client::WS_EncodeFrame(std::string inMessage, std::string& outFrame, enum WS_FrameType frameType)
{
	int ret = WS_EMPTY_FRAME;
	const uint32_t messageLength = inMessage.size();
	if (messageLength > 32767)
	{
		// 暂不支持这么长的数据  
		return WS_ERROR_FRAME;
	}

	uint8_t payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
	// header: 2字节, mask位设置为0(不加密), 则后面的masking key无须填写, 省略4字节  
	uint8_t frameHeaderSize = 2 + payloadFieldExtraBytes;
	uint8_t *frameHeader = new uint8_t[frameHeaderSize];
	memset(frameHeader, 0, frameHeaderSize);
	// fin位为1, 扩展位为0, 操作位为frameType  
	frameHeader[0] = static_cast<uint8_t>(0x80 | frameType);

	// 填充数据长度  
	if (messageLength <= 0x7d)
	{
		frameHeader[1] = static_cast<uint8_t>(messageLength);
	}
	else
	{
		frameHeader[1] = 0x7e;
		uint16_t len = htons(messageLength);
		memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);
	}

	// 填充数据  
	uint32_t frameSize = frameHeaderSize + messageLength;
	char *frame = new char[frameSize + 1];
	memcpy(frame, frameHeader, frameHeaderSize);
	memcpy(frame + frameHeaderSize, inMessage.c_str(), messageLength);
	frame[frameSize] = '\0';
	outFrame = frame;

	delete[] frame;
	delete[] frameHeader;
	return ret;
}

void WebSocket_Client::WS_onSendMessage(std::string message)
{
	string outFrame;
	WS_EncodeFrame(message, outFrame, WS_TEXT_FRAME);
	m_client->onSendMessage(outFrame);
}

void WebSocket_Client::WS_onRecvMessage(std::string recvmsg)
{
	std::string outmessage;
	std::cout << "recv : " << recvmsg << std::endl;
	WS_DecodeFrame(recvmsg, outmessage);
	if (m_recvHandle != NULL)
	{
		m_recvHandle->onRecvMessage(outmessage);
	}

	std::cout << "ws recv : " << outmessage << std::endl;
}

void WebSocket_Client::WS_Close()
{
	m_client->close();
}

SocketClient* WebSocket_Client::GetSocketClient()
{
	return m_client;
}

void WebSocket_Client::RegisterRecvHandle(WebSocket_Base* recvHandle)
{
	m_recvHandle = recvHandle;
}