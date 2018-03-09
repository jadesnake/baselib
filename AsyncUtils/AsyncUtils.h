#pragma once

#include <map>
#include "LockGuard.h"
#include "../baseX.h"
#include "../SingleCpp.h"

namespace base{

	template< UINT T=WM_USER >
	class LogicEvent
	{
	public:
		enum TaskID
		{
			UM_LOGIC = T+1,
			UM_TASK_END
		};
		static bool isValid(UINT msg)
		{
			if( msg>=UM_LOGIC && msg<=UM_TASK_END)
				return true;
			return false;
		}
	};
	class classLife
	{
	public:
		virtual void* Query(LPCTSTR pClass) =0;
		virtual unsigned int AddRef() = 0;
		virtual unsigned int RelRef() = 0;
	};
	class Back2Front
	{
	public:
		typedef std::map<long,classLife*> KEY_STACK;
		bool pushStack(classLife *c,long nT=0);
		void clear();
		bool postFront(HWND win,UINT msg,classLife *param);
		template<class S>
		S *cast(long p,LPCTSTR type)
		{
			CLockGuard lock(&m_mutex);
			KEY_STACK::iterator it = m_stack.find(p);
			if( it==m_stack.end() )
				return NULL;
			classLife *ret = it->second;
			void *what = ret->Query(type);
			return reinterpret_cast<S*>(what);
		}
		template<class S>
		S *take(long p,LPCTSTR type)
		{
			CLockGuard lock(&m_mutex);
			KEY_STACK::iterator it = m_stack.find(p);
			if( it==m_stack.end() )
				return NULL;
			classLife *ret = it->second;
			void *what = ret->Query(type);
			if(what)
			{
				ret->RelRef();
				m_stack.erase(it);
				return reinterpret_cast<S*>(what);
			}
			return NULL;
		}
	protected:
		KEY_STACK m_stack;
		CLock     m_mutex;
		GarboSingle(Back2Front);
	};
	/*--------------------------------------------------------------------------*/
	class BackLogicBase : public Ref<classLife>
	{
	public:
		enum Status{
			WORKING=1,
			FINISH =2,
			UNSTART=3,
			STOP
		};
		BackLogicBase(HWND win);
		virtual ~BackLogicBase(void);
		void resetWin(HWND win);
		virtual void start();
		virtual void close();
		virtual void stop();
		virtual Status  getStatus();
		virtual CAtlString getLastError();
		virtual void UpdateLastError(const CAtlString& v);
		void* Query(LPCTSTR pClass)
		{
			if(0==_tcscmp(pClass,_T("BackLogicBase")))
				return this;
			return NULL;
		}
	private:
		static DWORD WINAPI ThreadProc(LPVOID p1);
		void UpdateStatus(BackLogicBase::Status v);
	protected:
		virtual bool Run() = 0;
		virtual void Done() = 0;
	protected:
		HWND	m_win;
		HANDLE	m_thread;
		HANDLE  m_quit;
		HANDLE  m_run;
		CAtlString m_error;
		CLock   m_lockStatus;
		CLock   m_lockError;
		volatile Status m_curStats;
	};
	/*--------------------------------------------------------------------------*/
	class CTaskJob {

	public:
		CTaskJob();
		virtual ~CTaskJob();
	public:
		virtual void Run()=0;
		void SetTag(int ntag);
		void SetHwnd(HWND hWnd);
		void SetPid(LPCTSTR pszPid);
	protected:
		int m_nTag;
		HWND m_hnotifyWnd;
		CAtlString m_strPid;
	};
}
