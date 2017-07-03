#pragma once
#include <string>

class WebSocket_Base
{
public:
	virtual void onRecvMessage(std::string message) = 0;
};