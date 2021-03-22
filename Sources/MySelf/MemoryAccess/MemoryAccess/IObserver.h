#pragma once

#include <stdint.h>

class IObserver
{
public:
	virtual ~IObserver() {}
	virtual void *OnNotify(uint32_t uMsg, void *pParam1, void *pParam2,const void *pSender) = 0;
};