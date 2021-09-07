#include "StdAfx.h"
#include "AsyncUtils.h"

namespace base
{
	CTaskJob::CTaskJob():m_nTag(-1),m_hnotifyWnd(NULL)
	{

	}

	CTaskJob::~CTaskJob()
	{

	}

	void CTaskJob::SetTag(int ntag)
	{
		m_nTag = ntag;
	}

	void CTaskJob::SetHwnd( HWND hWnd )
	{
		m_hnotifyWnd = hWnd;
	}

	void CTaskJob::SetPid( LPCTSTR pszPid )
	{
		m_strPid = pszPid;
	}
	/*-------------------------------------------------------------------------------------*/
	Back2Front::Back2Front()
	{

	}
	Back2Front::~Back2Front()
	{

	}
	bool Back2Front::pushStack(classLife *c,long nT)
	{
		long key = nT;
		CLockGuard lock(&m_mutex);
		if(nT==0)
			key = (unsigned long)c;	//如果没给索引那么用地址做索引
		KEY_STACK::iterator it = m_stack.find(key);
		if( it!=m_stack.end() )
			return false;
		c->AddRef();
		m_stack[key] = c;
		return true;
	}
	void Back2Front::clear()
	{
		CLockGuard lock(&m_mutex);
		KEY_STACK::iterator it = m_stack.begin();
		for(it;it!=m_stack.end();it++)
			it->second->RelRef();
		m_stack.clear();
	}
	bool Back2Front::postFront(HWND win,UINT msg,classLife *param)
	{
		if(win==NULL)
			return false;
		return (::PostMessage(win,msg,0,(LPARAM)(void*)param)?true:false);
	}
	/*-------------------------------------------------------------------------------------*/
	BackLogicBase::BackLogicBase(HWND win)
	{
		m_id = 0;
		m_thread = NULL;
		m_quit = NULL;
		m_run  = NULL;
		m_win    = win;
		m_curStats = UNSTART;
		memset(m_timer,0,sizeof(m_timer));
	}
	BackLogicBase::~BackLogicBase(void)
	{
		if(m_quit)
		{
			::CloseHandle(m_quit);
			m_quit = NULL;
		}
		if(m_run)
		{
			::CloseHandle(m_run);
			m_run = NULL;
		}
		int nNum = (sizeof(m_timer)/sizeof(HANDLE));
		for(int n=0;n<nNum;n++)
		{
			if(m_timer[n])
			{
				 ::CloseHandle(m_timer[n]);
				 m_timer[n] = 0;
			}
		}
	}
	void BackLogicBase::resetWin(HWND win)
	{
		m_win = win;
	}
	void BackLogicBase::start()
	{
		if(m_thread==NULL)
		{
			m_quit = ::CreateEvent(NULL,FALSE,FALSE,NULL);
			m_run = ::CreateEvent(NULL,FALSE,FALSE,NULL);
 			m_thread = (HANDLE)_beginthreadex(NULL,0,&BackLogicBase::ThreadProc,this,0,&m_id);
			Back2Front::Get()->pushStack(this);
		}
		if(m_run)
			::SetEvent(m_run);
	}
	int BackLogicBase::addTimer(UINT nInterval,bool bImmediately)
	{
		if(m_thread)
			return 0;
		int idx=-1;
		int nNum = (sizeof(m_timer)/sizeof(HANDLE));
		for(int n=0;n<nNum;n++)
		{
			if(m_timer[n]==0)
			{
				idx = n;
				break;
			}
		}
		if(idx==-1)
			return 0;		
		HANDLE hEV = ::CreateWaitableTimer(NULL,FALSE,NULL);
		LARGE_INTEGER liDueTime;
		liDueTime.QuadPart = -10000*(__int64)nInterval;
		BOOL bRet = FALSE;
		if(bImmediately)
			bRet = ::SetWaitableTimer(hEV,NULL, nInterval,NULL,NULL,TRUE);
		else
			bRet = ::SetWaitableTimer(hEV,&liDueTime, nInterval,NULL,NULL,TRUE);
		if (!bRet)
		{
			CloseHandle(hEV);
			return 0;
		}
		m_timer[idx] = hEV;
		m_timerVal[idx] = TimerVal(liDueTime,nInterval);
		return (idx+1);
	}
	bool BackLogicBase::stopTimer(int i)
	{
		int nNum = (sizeof(m_timer)/sizeof(HANDLE));
		if(i>nNum)
			return false;
		int idx = 0;
		if(i>=1)
			idx = i-1;
		BOOL bRet = FALSE;
		if(m_timer[idx])
		{
			bRet = ::CancelWaitableTimer(m_timer[idx]);
			return true;
		}
		return false;
	}
	bool BackLogicBase::resumTimer(int i)
	{
		int nNum = (sizeof(m_timer)/sizeof(HANDLE));
		if(i>nNum)
			return false;
		int idx = 0;
		if(i>=1)
			idx = i-1;
		BOOL bRet = FALSE;
		if(m_timer[idx])
		{
 			bRet = ::SetWaitableTimer(m_timer[idx],&m_timerVal[idx].dueTime, m_timerVal[idx].interval,NULL,NULL,TRUE);
			return true;
		}
		return false;
 	}
	void BackLogicBase::stop()
	{
		if(m_thread)
		{
			::ResetEvent(m_run);
			UpdateStatus(STOP);
		}
	}
	void BackLogicBase::close(DWORD waitTM)
	{
		DWORD tid = GetCurrentThreadId();
		if (m_thread) 
		{
			::ResetEvent(m_run);
			::SetEvent(m_quit);		//结束后台
			if(m_id!=tid)
			{
				if(WAIT_TIMEOUT==::WaitForSingleObject(m_thread,waitTM))
					return ;
			}
			::CloseHandle(m_thread);
			UpdateStatus(FINISH);
			m_thread = NULL;
			Back2Front::Get()->remove((unsigned long)this);
		}
	}
	BackLogicBase::Status  BackLogicBase::getStatus()
	{
		BackLogicBase::Status ret;
		CLockGuard lock(&m_lockStatus);
		ret = m_curStats;
		return ret;
	}
	void BackLogicBase::UpdateStatus(BackLogicBase::Status v)
	{
		CLockGuard lock(&m_lockStatus);
		m_curStats = v;
	}
	CAtlString BackLogicBase::getLastError()
	{
		CAtlString ret;
		CLockGuard lock(&m_lockError);
		ret = m_error;
		return ret;
	}
	void BackLogicBase::UpdateLastError(const CAtlString& v)
	{
		CLockGuard lock(&m_lockError);
		m_error = v;
	}
	unsigned int BackLogicBase::ThreadProc(void* p1)
	{
		BackLogicBase *pT = reinterpret_cast<BackLogicBase*>(p1);
		int nCountEv = 0;
		HANDLE events[128]={0};
		events[0] = pT->m_quit;
		nCountEv += 1;
		int nNum = (sizeof(pT->m_timer)/sizeof(HANDLE));
		int idx  = 0;
		for(int n=0;n<nNum;n++)
		{
			if(pT->m_timer[n])
			{
				events[nCountEv] = pT->m_timer[n];
				nCountEv += 1;
			}
		}
		events[nCountEv] = pT->m_run;
		nCountEv += 1;
		::CoInitialize(NULL);
		::OleInitialize(NULL);
		while(1)
		{
			bool bPost=false;
			DWORD dwWait = WaitForMultipleObjects(nCountEv,events,FALSE,INFINITE);
			if(dwWait==WAIT_OBJECT_0)
				break;
			if(events[dwWait]==pT->m_run || events[dwWait]==pT->m_quit)
				idx = 0;
			else
				idx = dwWait;
			pT->UpdateStatus(WORKING);
			if(idx==0)
				bPost = pT->Run();
			else
				bPost = pT->Timer(idx);
			pT->UpdateStatus(STOP);
			if( bPost && pT->m_win)
				Back2Front::Get()->postFront(pT->m_win,LogicEvent<>::UM_LOGIC,pT);
		}
		pT->UpdateStatus(FINISH);
		pT->Done();
		::OleUninitialize();
		::CoUninitialize();		
		_endthreadex(0xdead);
		return 0xdead;
	}
}
