#pragma once

#include <map>
#include "LockGuard.h"
#include "../baseX.h"
#include "../SingleCpp.h"
#include <atlutil.h>

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
		virtual ~classLife(){ }
		virtual void* Query(LPCTSTR pClass) =0;
		virtual unsigned int AddRef() = 0;
		virtual unsigned int RelRef() = 0;
	};
	class Back2Front
	{
	public:
		typedef std::map<unsigned long,classLife*> KEY_STACK;
		bool pushStack(classLife *c,long nT=0);
		void clear();
		bool postFront(HWND win,UINT msg,classLife *param);
		template<class S>
		S *cast(unsigned long p,LPCTSTR type)
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
		S *take(unsigned long p,LPCTSTR type)
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
		void remove(unsigned long p)
		{
			CLockGuard lock(&m_mutex);
			KEY_STACK::iterator it = m_stack.find(p);
			if( it==m_stack.end() )
				return ;
			classLife *ret = it->second;
			if(ret)
			{
				ret->RelRef();
				m_stack.erase(it);
			}
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
		class TimerVal
		{
		public:
			TimerVal(){

			}
			TimerVal(LARGE_INTEGER a,UINT b){
				dueTime = a;
				interval= b;
			}
			LARGE_INTEGER dueTime;
			UINT interval;
		};
		BackLogicBase(HWND win);
		virtual ~BackLogicBase(void);
		void resetWin(HWND win);
		void start();
		virtual int addTimer(UINT nInterval,   // 以毫秒为单位
							 bool bImmediately = false);
		virtual bool stopTimer(int i);
		virtual bool resumTimer(int i);
		virtual void close(DWORD waitTM=3000);
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
		static unsigned int WINAPI ThreadProc(void* p1);
		void UpdateStatus(BackLogicBase::Status v);
	protected:
		virtual bool Run() = 0;
		virtual bool Timer(int i){ return false; };
		virtual void Done() = 0;
	protected:
		HWND	m_win;
		HANDLE	m_thread;
		HANDLE  m_quit;
		HANDLE  m_run;
		HANDLE  m_timer[32];
		std::map<int,TimerVal> m_timerVal;
		CAtlString m_error;
		CLock   m_lockStatus;
		CLock   m_lockError;
		unsigned m_id;
		volatile Status m_curStats;
	};
	/*--------------------------------------------------------------------------*/
	class CTaskJob {

	public:
		CTaskJob();
		virtual ~CTaskJob();
	public:
		virtual void Run(int i=0)=0;
		void SetTag(int ntag);
		void SetHwnd(HWND hWnd);
		void SetPid(LPCTSTR pszPid);
	protected:
		int m_nTag;
		HWND m_hnotifyWnd;
		CAtlString m_strPid;
	};
}
