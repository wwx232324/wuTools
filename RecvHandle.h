#pragma once
#include "Interface.h"
class RecvHandle :	public WebSocket_Base
{
public:
	RecvHandle();
	~RecvHandle();
	virtual void onRecvMessage(std::string message);
};

