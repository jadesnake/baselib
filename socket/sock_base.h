#pragma once

#include "jadesocketiocp.h"
#include "Interface.h"
#include "tools.h"
class CheckAlive : public NETCHECK__
{
public:
	enum State
	{
		Connecting,		//正在连接
		Connected,		//连接状态
		Disconnectd,	//已断开连接
		Error			//异常
	};
	class Service
	{
	public:
		CAtlString SvrIp;	//服务器地址
		UINT  unPort;		//服务器端口号
		UINT  unType;		//0---tcp,1---udp
		UINT  unTimout;		//超时设置单位秒
		char  TestPack[100];	//测试数据包
		Service& operator=(const Service& s)
		{
			Service *pT = const_cast<Service*>(&s);
			this->SvrIp   = pT->SvrIp;
			this->unPort  = pT->unPort;
			this->unType  = pT->unType;
			this->unTimout= pT->unTimout;
			memcpy(this->TestPack,pT->TestPack,sizeof(this->TestPack));
			return (*this);
		}
	};
	class Proxy : public base::IRef
	{
	public:
		virtual DWORD onStart(JadeSocketIocp<> *socket,CheckAlive *pbase) = 0;
		virtual void  onRecv(PRECV pRecv,DWORD &dwID) = 0;
		virtual void  onSend(PSEND pSend,DWORD &dwID) = 0;
		virtual void  onConnect(PCONNECTEX pConnect,DWORD &dwID) = 0;
		virtual void  onDisconnect(DWORD &dwID) = 0;
		virtual bool  IsConnected(void) = 0;
	};
	CheckAlive(const Service &svr,DWORD dwUser,PNETCHECK_CALLBACK fn);
	virtual ~CheckAlive();
	DWORD		Startup(Proxy *proxy);
	DWORD		PostEndService(void);
	Service*	GetService(void);
	int			DoCallBack(LPCTSTR pMsg,bool bSuc,bool bDis);
protected:
	static DWORD WINAPI BackService(LPVOID lpParameter);
	DWORD  Connect(void);
	void   OnSendCmd(PSEND,DWORD);
	void   OnSendCmd(PSENDTO,DWORD);
	CAtlString	State2String(State);
private:
	PNETCHECK_CALLBACK  m_fn;			//事件回调函数
	DWORD				m_dwUser;
	Service				m_svr;			//服务器参数
	Proxy				*m_pProxy;
	JadeSocketIocp<>	m_socket;		//socket对象
	HANDLE				m_hThread;		//线程对象
	int					m_suspended;	//线程SUSPENDED
	int					m_bEnd;			//线程内结束标记
	volatile State		m_state;		//状态标记
	DWORD				m_threadId;		//线程id
};


#include "proxysock5.h"
#include "proxyhttp.h"
