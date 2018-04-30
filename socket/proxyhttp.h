#pragma once

#include "sock_base.h"
#include "../base.h"
class ProxyHttp : public base::Ref<CheckAlive::Proxy>
{
public:
	enum HttpState
	{
		StateDisconnected, /*<The client is in disconnected state.*/
		StateConnecting,   /*<The client is currently trying to establish a connection.*/
		StateConnected,    /*<The client is connected to the server but authentication is not (yet) done.*/
		StateError
	};
	class Proxy
	{
	public:
		CAtlString SvrIp;	//服务器地址
		UINT  unType;		//服务类型0-sock4,1-sock5,2-http
		UINT  unPort;		//端口
		CAtlString name;	//用户名称
		CAtlString pass;	//口令
		Proxy& operator=(const Proxy& s)
		{
			Proxy *pT = const_cast<Proxy*>(&s);
			this->SvrIp   = pT->SvrIp;
			this->unPort  = pT->unPort;
			this->unType  = pT->unType;
			this->name	  = pT->name;
			this->pass    = pT->pass;
			return (*this);
		}
	};
	ProxyHttp(const Proxy& proxy);
	virtual ~ProxyHttp();
public:
	virtual DWORD onStart(JadeSocketIocp<> *socket,CheckAlive *pbase);
	virtual void  onRecv(PRECV pRecv,DWORD &dwID);
	virtual void  onSend(PSEND pSend,DWORD &dwID);
	virtual void  onConnect(PCONNECTEX pConnect,DWORD &dwID);
	virtual void  onDisconnect(DWORD &dwID);
	virtual bool  IsConnected(void);
protected:
	CAtlString	State2String(HttpState n);
	bool		SendIo(char *pBuf,UINT unLen);
private:
	CheckAlive			*m_pbase;
	Proxy				m_proxy;
	JadeSocketIocp<>	*m_socket;		//socket对象
	HttpState			m_state;
	UINT				m_unRetry;
	std::string			m_strPack;		//网络协议包
};