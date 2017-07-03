#include "RecvHandle.h"
#include <iostream>

using namespace std;

RecvHandle::RecvHandle()
{
}


RecvHandle::~RecvHandle()
{
}

void RecvHandle::onRecvMessage(std::string message)
{
	cout << "recv message : " << message << endl;
}
