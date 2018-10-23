#pragma once
#include <atlbase.h>
static void CALLBACK TimerProc(void*, BOOLEAN);
//
//   类名称：CTimer
//   类描述：定时器类
//
class CTimer
{
public:
    CTimer()
    {
        m_hTimer = NULL;
        m_mutexCount = 0;
    }
	virtual ~CTimer()
    {
        Stop();
    }
	
    bool Start(unsigned int nInterval,   // 以毫秒为单位
               bool bImmediately = false,// true:立即执行
               bool bOnce = false)       // true:只执行一次
    {
        if( m_hTimer )
        {
            return false;
        }

        SetCount(0);
        BOOL bSuccess = CreateTimerQueueTimer( &m_hTimer,
                                              NULL,
											  TimerProc,
                                              this,
                                              bImmediately ? 0 : nInterval,
                                              bOnce ? 0 : nInterval,
											  WT_EXECUTEINTIMERTHREAD);
        return( bSuccess != FALSE );
    }

    void Stop()
    {
		//fixed by jiayh
		//If this parameter is INVALID_HANDLE_VALUE, the function waits for any running timer callback functions to complete before returning.
        DeleteTimerQueueTimer( NULL, m_hTimer, INVALID_HANDLE_VALUE );
        m_hTimer = NULL ;
    }
	//定时器事件
    virtual void OnTimedEvent()
    {
        
    }
	//调用次数
    void SetCount(int value)
    {
        InterlockedExchange( &m_mutexCount, value );
    }
	//获取调用次数
    int GetCount()
    {
        return InterlockedExchangeAdd( &m_mutexCount, 0 );
    }
	operator HANDLE()
	{
		return m_hTimer;
	}
private:
    HANDLE m_hTimer;
    long m_mutexCount;
	bool m_bCall;
};

//定时器回调函数
void CALLBACK TimerProc(void* param, BOOLEAN timerCalled)
{
    CTimer* timer = static_cast<CTimer*>(param);
    timer->SetCount( timer->GetCount()+1 );
    timer->OnTimedEvent();
};

//
//   类名称：CTemplateTimer
//   类描述：定时器模板类
//
template <class T> class CTemplateTimer : public CTimer
{
public:
    typedef private void (T::*TimedFunction)(void);

    CTemplateTimer()
    {
        m_pTimedFunction = NULL;
        m_pClass = NULL;
    }

    void SetTimedEvent(T *pClass, TimedFunction pFunc)
    {
        m_pClass         = pClass;
        m_pTimedFunction = pFunc;
    }

protected:
    void OnTimedEvent()  
    {
        if (m_pTimedFunction && m_pClass)
        {
            (m_pClass->*m_pTimedFunction)();
        }
    }

private:
    T *m_pClass;
    TimedFunction m_pTimedFunction;
};

HANDLE CreateWaitTimer(DWORD dwInterval)
{
	HANDLE hRet = ::CreateWaitableTimer(NULL,FALSE,NULL);
	if(hRet==NULL)
		return NULL;
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -10000*(__int64)dwInterval;
	BOOL bRet = SetWaitableTimer(hRet,&liDueTime, dwInterval,NULL,NULL,TRUE);
	if (!bRet)
	{
		CloseHandle(hRet);
		return NULL;
	}
	return hRet;
}