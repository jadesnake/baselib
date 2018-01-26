#pragma once

namespace SingleCpp
{
	template<class theC>
	class Garbo
	{
	public:
		Garbo()
		{
			InitializeCriticalSection(&m_csLock);
			gSingle = NULL;
			m_init  = false;
		}
		virtual ~Garbo()
		{
			Destroy();
		}
		theC* Get()
		{
			theC *ret = NULL;
			EnterCriticalSection(&m_csLock);
			if(gSingle == NULL)
				gSingle = new theC;
			ret = gSingle;
			LeaveCriticalSection(&m_csLock);
			return ret;
		}
		void Destroy()
		{
			EnterCriticalSection(&m_csLock);
			if (gSingle)
				delete gSingle;
			gSingle = NULL;
			LeaveCriticalSection(&m_csLock);
			DeleteCriticalSection(&m_csLock);
		}
	private:
		bool m_init;
		theC * volatile gSingle;
		CRITICAL_SECTION m_csLock;
	};
}

#define GarboSingle(className) private: friend SingleCpp::Garbo<className>;		\
	className(const className&);					\
	className& operator=(const className&);			\
	className(void); virtual ~className(void);		\
	public:											\
	static className *Get(){ static SingleCpp::Garbo<className> logic; return logic.Get(); }	\
	static void Destroy(){ static SingleCpp::Garbo<className> logic;  logic.Destroy();  }