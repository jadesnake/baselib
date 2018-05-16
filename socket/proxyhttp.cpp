#include "stdafx.h"
#include "proxyhttp.h"
#include "../base64.h"

ProxyHttp::ProxyHttp(const Proxy& proxy)
	:m_socket(NULL),m_pbase(NULL),m_unRetry(0)
{
	m_proxy = proxy;
	m_state	= StateError;
}
ProxyHttp::~ProxyHttp()
{

}
bool  ProxyHttp::IsConnected(void)
{
	return (m_state==StateConnected);
}
DWORD ProxyHttp::onStart(JadeSocketIocp<> *socket,CheckAlive *pbase)
{
	DWORD	dwRet = 0;
	m_socket = socket;
	m_pbase  = pbase;
	dwRet = m_socket->ConnectEx(0,CT2CA(m_proxy.SvrIp.GetString()),m_proxy.unPort,NULL,0);
	if( dwRet != 997 )
		m_pbase->DoCallBack(State2String(m_state),false,true);
	return dwRet;
}
void ProxyHttp::onRecv(PRECV pRecv,DWORD &dwID)
{
	if( m_state == StateConnecting )
	{
		m_strPack.append((char*)pRecv->pBuf,pRecv->dwLen);
		if( ( !m_strPack.compare( 0, 12, "HTTP/1.0 200" )
			|| !m_strPack.compare( 0, 12, "HTTP/1.1 200" ) )
			&& !m_strPack.compare( m_strPack.length() - 4, 4, "\r\n\r\n" ) )
		{
			m_strPack = "";
			m_state = StateConnected;
			m_pbase->DoCallBack(State2String(m_state),true,false);
		}
		else if( !m_strPack.compare( 9, 3, "407" ) )
		{
			m_socket->Disconnect(m_state,0,0,NULL);
			m_state = StateDisconnected;
			m_pbase->DoCallBack(State2String((HttpState)407),false,false);
		}
		else if( !m_strPack.compare( 9, 3, "403" ) )
		{
			m_socket->Disconnect(m_state,0,0,NULL);
			m_state = StateDisconnected;
			m_pbase->DoCallBack(State2String((HttpState)403),false,false);
		}
		else if( !m_strPack.compare( 9, 3, "404" ) )
		{
			m_socket->Disconnect(m_state,0,0,NULL);
			m_state = StateDisconnected;
			m_pbase->DoCallBack(State2String((HttpState)404),false,false);
		}
	}
}
void ProxyHttp::onSend(PSEND pSend,DWORD &dwID)
{
	if( m_socket )
	{
		DWORD	dwFlags = 0,dwRet = 0;
		dwRet = m_socket->Receive(m_state,0xFF,dwFlags,NULL);
		if( dwRet != 997 )	//出现错误
		{
			m_state = StateError;
			m_socket->Disconnect(m_state,0,0,NULL);
		}
	}
}
void ProxyHttp::onConnect(PCONNECTEX pConnect,DWORD &dwID)
{
	CAtlString sport;
	CheckAlive::Service *pSvr = m_pbase->GetService();
	sport.Format(TEXT("%d"),pSvr->unPort);
	std::string os = "CONNECT ";
				os += CT2CA(pSvr->SvrIp);
				os += ":";
				os += CT2CA(sport);
				os += " HTTP/1.0\r\n";
				os += "Host: ";
				os += CT2CA(pSvr->SvrIp);
				os += "\r\n"
					  "Content-Length: 0\r\n"
					  "Proxy-Connection: Keep-Alive\r\n"
					  "Pragma: no-cache\r\n"
					  "User-Agent: sitong/ 0.0.1 \r\n";
	if( !m_proxy.name.IsEmpty() && !m_proxy.pass.IsEmpty() )
	{
		std::string strTmp;
		strTmp += CT2CA(m_proxy.name) ;
		strTmp += ":";
		strTmp += CT2CA(m_proxy.pass);

		os += "Proxy-Authorization: Basic ";
		os += base::encode64( strTmp );
		os += "\r\n";
	}
	os += "\r\n";
	if( !SendIo( (char*)os.c_str(),os.size()) )
		m_state = StateDisconnected;
	else
		m_state = StateConnecting;
}
void ProxyHttp::onDisconnect(DWORD &dwID)
{
	if( m_state != StateConnected )
	{
		m_state = StateDisconnected;
		m_pbase->DoCallBack(State2String(m_state),false,true);
	}
}
CAtlString	ProxyHttp::State2String(HttpState n)
{
	CAtlString	strRet;
	if( n == StateConnected )
		strRet = TEXT("已连接");
	if( n == StateDisconnected )
		strRet = TEXT("连接被断开");
	if( n == StateConnecting )
		strRet = TEXT("正在连接...");
	if( n == 407 )
		strRet = TEXT("代理服务需要身份验证");
	if( n == 403 )
		strRet = TEXT("403 forbidden");
	if( n == 404 )
		strRet = TEXT("not found");
	if( n == 10053 )
		strRet = TEXT("连接被拒绝");
	return strRet;
}

bool  ProxyHttp::SendIo(char *pBuf,UINT unLen)
{
	DWORD dwRet = 0;
	if( m_socket )
		dwRet = m_socket->Send(m_state,pBuf,unLen,0,0,NULL);
	if( dwRet != 997 )
	{
		m_socket->Disconnect(m_state,0,0,NULL);
		return false;
	}
	return true;
}