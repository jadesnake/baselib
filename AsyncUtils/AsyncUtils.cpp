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
			key = (long)c;	//如果没给索引那么用地址做索引
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
		m_thread = NULL;
		m_win    = win;
		Back2Front::Get()->pushStack(this);
	}
	BackLogicBase::~BackLogicBase(void)
	{

	}
	void BackLogicBase::start()
	{
		if(m_thread==NULL)
		{
			m_curStats = WORKING;
			m_thread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)&BackLogicBase::ThreadProc,this,0,NULL);
		}
	}
	void BackLogicBase::close()
	{
		if (m_thread) 
		{
			::WaitForSingleObject(m_thread, 2000);
			::CloseHandle(m_thread);
			m_thread = NULL;
		}
	}
	BackLogicBase::Status  BackLogicBase::getStatus()
	{
		DWORD dwCode = 0;
		if( m_curStats == FINISH ){
			return FINISH;
		}	
		if(m_thread){
			::GetExitCodeThread(m_thread,&dwCode);
			if( dwCode==STILL_ACTIVE ){
				return WORKING;
			}
		}
		return FINISH;
	}
	CAtlString BackLogicBase::getLastError()
	{
		if( WORKING==getStatus() )
			return NULL;
		return m_error;
	}
	DWORD BackLogicBase::ThreadProc(LPVOID p1)
	{
		BackLogicBase *pT = reinterpret_cast<BackLogicBase*>(p1);
		pT->Run();
		pT->m_curStats = FINISH;
		if(pT->m_win)
			Back2Front::Get()->postFront(pT->m_win,LogicEvent<>::UM_LOGIC,pT);
		return 0xdead;
	}
}
