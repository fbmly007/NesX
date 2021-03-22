#include "NotificationCenter.h"

CNotificationCenter::CNotificationCenter()
{
}


CNotificationCenter::~CNotificationCenter()
{
	m_Events.clear();
}

CNotificationCenter * CNotificationCenter::GetInst()
{
	static CNotificationCenter center;
	return &center;
}

void CNotificationCenter::AddObserver(uint32_t uMsg, IObserver * pObserver)
{
	if (!pObserver) return;
	unordered_map<uint32_t, unordered_set<IObserver *>>::iterator it = m_Events.find(uMsg);
	if (it != m_Events.end()) it->second.insert(pObserver);
	else m_Events[uMsg] = unordered_set<IObserver *>{ pObserver };
}

void CNotificationCenter::RemoveObserver(uint32_t uMsg, IObserver * pObserver)
{
	if (!pObserver) return;
	unordered_map<uint32_t, unordered_set<IObserver *>>::iterator it = m_Events.find(uMsg);
	if (it == m_Events.end()) return;
	it->second.erase(pObserver);
	if (it->second.empty()) m_Events.erase(uMsg);
}

void *CNotificationCenter::Notify(uint32_t uMsg, void *pParam1, void *pParam2, const void * pSender)
{
	unordered_map<uint32_t, unordered_set<IObserver *>>::iterator it = m_Events.find(uMsg);
	if (it == m_Events.end()) return nullptr;
	unordered_set<IObserver *>& observers = it->second;

	void *result = nullptr;
	for (unordered_set<IObserver *>::iterator p = observers.begin(); p != observers.end(); ++p)
	{
		result = (*p)->OnNotify(uMsg, pParam1, pParam2, pSender);

		// 如果消息链中已经有对象处理过了, 就中止发布通知
		if (result) break;
	}

	return result;
}
