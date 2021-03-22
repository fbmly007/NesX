#pragma once

#include "IObserver.h"
#include <unordered_set>
#include <unordered_map>
using namespace std;

class CNotificationCenter
{
private:
	CNotificationCenter();
	~CNotificationCenter();

public:
	static CNotificationCenter *GetInst();

public:
	void AddObserver(uint32_t uMsg, IObserver *pObserver);
	void RemoveObserver(uint32_t uMsg, IObserver *pObserver);
	void *Notify(uint32_t uMsg, void * pParam1 = nullptr, void * pParam2 = nullptr, const void *pSender = nullptr);

private:
	unordered_map<uint32_t, unordered_set<IObserver *>> m_Events;
};

