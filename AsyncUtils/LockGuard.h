#pragma once

class CLock
{
	friend class CLockGuard;
public:
	CLock()
	{ 
		InitializeCriticalSection(&m_tgSection);  
	}

	virtual ~CLock()
	{
		DeleteCriticalSection(&m_tgSection);
	}
	void Lock()
	{
		EnterCriticalSection(&m_tgSection);
	}

	BOOL TryLock()
	{
		return TryEnterCriticalSection(&m_tgSection);
	}

	void UnLock()
	{
		LeaveCriticalSection(&m_tgSection);
	}

private:
	CRITICAL_SECTION	m_tgSection;
};

class CLockGuard
{
public:

	CLockGuard(CLock* cs)
	{
		m_cs = cs;
		
		if (m_cs != NULL)
		{
			m_cs->Lock();
		}
	}

	void Unlock()
	{
		if (m_cs)
		{
			m_cs->UnLock();
			m_cs = NULL;
		}
		
	}

	virtual ~CLockGuard(void)
	{
		Unlock();
	}

private:
	CLock* m_cs;
};
