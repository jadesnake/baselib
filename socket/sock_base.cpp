#include "stdafx.h"
#include "sock_base.h"

UINT RET_THREAD = JadeSocketIocp<>::UNKNOWN_USERDATA+1;
DWORD CheckAlive::BackService(LPVOID lpParameter)
{
	CheckAlive *pT = static_cast<CheckAlive*>(lpParameter);
	DWORD dwId   = 0;
	UINT  unType = 0;
	void  *pWhat = NULL;
	DWORD dwRet  = 0;
	UINT  unRetry= 0;
	int	  nCode  = 0;
	while( 1 )
	{
		dwRet = pT->m_socket.WaitSocketComplete(dwId,unType,&pWhat,pT->m_svr.unTimout);
		if( dwRet )
		{
			if( pT->m_state == Connected )
				pT->m_socket.Disconnect(0,0,0,NULL);				
			if( dwRet == 258 )
			{
				//等待超时断开连接				
				pT->DoCallBack(pT->State2String((CheckAlive::State)dwRet),false,true);
				pT->m_bEnd = true;
			}
			else
			{
				pT->DoCallBack(pT->State2String((CheckAlive::State)dwRet),false,true);
				pT->m_bEnd = true;
			}
			pT->m_bEnd = true;
			break;
		}
		else
		{
			//收到正确的数据包
			if( JadeSocketIocp<>::UNKNOWN_USERDATA == unType )
			{
				UINT *unW = static_cast<UINT*>(pWhat);
				if( (*unW)==RET_THREAD )	 
					pT->m_bEnd = true;
			}
			if( unType == JadeSocketIocp<>::SOCKET_CONNECTEX )
			{
				PCONNECTEX pOptData = (PCONNECTEX)pWhat;
				if( pT->m_pProxy )
					pT->m_pProxy->onConnect(pOptData,dwId);
				else
				{
					//已连接成功
					pT->m_state = Connected;
					nCode = pT->DoCallBack(pT->State2String(pT->m_state),true,false);
					if( nCode == -1 )
						pT->m_bEnd = true;
				}
			}
			if( unType == JadeSocketIocp<>::SOCKET_RECV )
			{
				PRECV pOptData = (PRECV)pWhat;
				if( pOptData->dwLen == 0 )
				{
					pT->m_state = Disconnectd;

					if( pT->m_pProxy )
						pT->m_pProxy->onDisconnect(dwId);	//连接被断开
					else
						pT->DoCallBack(pT->State2String(pT->m_state),false,true);
					pT->m_bEnd = true;
				}
				else
				{
					if( pT->m_pProxy )
					{
						pT->m_pProxy->onRecv(pOptData,dwId);
						if( pT->m_pProxy->IsConnected() )
							pT->m_bEnd = true;
					}
					else if( pT->m_svr.unType==1 )
					{
						pT->m_state = Connected;
						pT->DoCallBack(pT->State2String(pT->m_state),true,false);
						pT->m_bEnd = true;
					}
				}
			}
			if( unType == JadeSocketIocp<>::SOCKET_SENDTO )
			{
				PSENDTO pOptData = (PSENDTO)pWhat;
				if( pOptData->dwPos == 0 )
				{
					if( pOptData->dwBufLen == pOptData->dwNumBytes )
					{
						pT->OnSendCmd(pOptData,dwId);
					}
					else if( pOptData->dwBufLen > pOptData->dwNumBytes )
					{
						pT->m_socket.SendTo(dwId,pOptData->pBuf,pOptData->dwBufLen,
							pOptData->dwNumBytes,0,CT2CA(pT->m_svr.SvrIp),
							pT->m_svr.unPort,NULL);
					}
				}
				else if( pOptData->dwBufLen = (pOptData->dwNumBytes+pOptData->dwPos) )
				{
					pT->OnSendCmd(pOptData,dwId);
				}
				else
				{
					pT->m_socket.SendTo(dwId,pOptData->pBuf,pOptData->dwBufLen,
						pOptData->dwNumBytes,0,CT2CA(pT->m_svr.SvrIp),
						pT->m_svr.unPort,NULL);
				}
			}
			if( unType == JadeSocketIocp<>::SOCKET_SEND )
			{
				PSEND pOptData = (PSEND)pWhat;
				if( pOptData->dwPos == 0 )
				{
					if( pOptData->dwBufLen == pOptData->dwNumBytes )
					{
						pT->OnSendCmd(pOptData,dwId);
					}
					else if( pOptData->dwBufLen > pOptData->dwNumBytes )
					{
						pT->m_socket.Send(dwId,pOptData->pBuf,pOptData->dwBufLen,
							pOptData->dwNumBytes,0,NULL);
					}
				}
				else if( pOptData->dwBufLen = (pOptData->dwNumBytes+pOptData->dwPos) )
				{
					pT->OnSendCmd(pOptData,dwId);
				}
				else
				{
					pT->m_socket.Send(dwId,pOptData->pBuf,pOptData->dwBufLen,
						pOptData->dwPos+pOptData->dwNumBytes,0,NULL);
				}
			}
			if( unType == JadeSocketIocp<>::SOCKET_DISCONNECTEX )
			{
				pT->m_state = Disconnectd;
				if( pT->m_pProxy )
					pT->m_pProxy->onDisconnect(dwId);
				else
					pT->DoCallBack(pT->State2String(pT->m_state),false,true);
				pT->m_bEnd = true;
			}
			if( unType == JadeSocketIocp<>::SOCKET_RECVFROM )
			{
				PRECVFROM pOptData = (PRECVFROM)pWhat;
				if( pOptData->dwLen )
				{
					pT->m_state = Connected;
					nCode=pT->DoCallBack(pT->State2String(pT->m_state),true,false);
					if( nCode == -1 )
						pT->m_bEnd = true;
				}
				else
				{
					pT->m_state = Disconnectd;
					nCode=pT->DoCallBack(pT->State2String(pT->m_state),false,true);
					if( nCode == -1 )
						pT->m_bEnd = true;
				}
				pT->m_bEnd = true;
			}
		}
		pT->m_socket.FreeMemory(pWhat,unType);		  //释放内存
		if( pT->m_bEnd )
			break;
	}
	return 0xdead;
}
CheckAlive::CheckAlive(const Service& svr,DWORD dwUser,PNETCHECK_CALLBACK fn)
	:m_socket(2,2,FALSE),m_hThread(NULL),m_fn(fn),m_dwUser(dwUser),
	m_suspended(0),m_pProxy(NULL),m_bEnd(false),m_state(Error)
{
	m_svr       = svr;
	m_hThread   = ST_CreateThread(NULL,0,BackService,(void*)this,CREATE_SUSPENDED,&m_threadId);
	m_suspended = 1;
}
CheckAlive::~CheckAlive()
{
	//如果是1表示该线程从未启动
	PostEndService();
	if( m_pProxy )
	{
		m_pProxy->RelRef();
		m_pProxy = 0;
	}
}
void   CheckAlive::OnSendCmd(PSENDTO pOpt,DWORD dwId)
{
	DWORD dwFlags=0,dwRet=0;
	dwRet = m_socket.ReceiveFrom((DWORD)(void*)this,sizeof(m_svr.TestPack),dwFlags,
		CT2CA(m_svr.SvrIp),m_svr.unPort,NULL);		
	if( dwRet != 997 )
	{
		m_state = (CheckAlive::State)dwRet;
		if( m_svr.unType==0 )
			m_socket.Disconnect((DWORD)(void*)this,0,0,NULL);
		else
			DoCallBack(State2String(m_state),false,false);
	}
}
void   CheckAlive::OnSendCmd(PSEND pOpt,DWORD dwId)
{
	if( m_pProxy )
		m_pProxy->onSend(pOpt,dwId);
}
int   CheckAlive::DoCallBack(LPCTSTR pMsg,bool bSuc,bool bDis)
{
	DWORD nCode = 0;
	if( m_fn )
		nCode = m_fn(this,pMsg,bSuc,bDis,m_dwUser);
	if( nCode == 1 )
		return -1;
	return 0;
}
CheckAlive::Service* CheckAlive::GetService(void)
{
	return &m_svr;
}
DWORD	CheckAlive::PostEndService(void)
{
	DWORD dwRet = 0;
	if( m_hThread == NULL)
		dwRet = -1;			//操作异常
	if( m_suspended == 1 )
	{
		m_suspended = 0;
		TerminateThread(m_hThread,0xdead);
		CloseHandle(m_hThread);
		m_hThread = NULL;
		dwRet = 0;
	}
	else
	{
		if( GetCurrentThreadId() == m_threadId )
		{
			m_bEnd = true;
			dwRet  = 0;
		}
		else
		{
			DWORD dwExit = 0;
			if( GetExitCodeThread(m_hThread,&dwExit) )
			{
				if( dwExit == 0xdead )
				{
					dwRet = 0;
				}
				else
				{
					dwRet =m_socket.PostUserDefined(0,RET_THREAD,NULL);
					WaitForSingleObject(m_hThread,INFINITE);
					dwRet = 0;
				}
			}
		}
	}	
	return dwRet;
}
DWORD	CheckAlive::Startup(Proxy *proxy)
{	
	DWORD dwRet = 0;
	if( proxy == NULL )
		dwRet = m_socket.Create(0,NULL,m_svr.unType?SOCK_DGRAM:SOCK_STREAM);
	else
		dwRet = m_socket.Create(0,NULL,SOCK_STREAM);
	if( dwRet )
		return dwRet;
	if( proxy )
	{
		m_pProxy = proxy;
		proxy->AddRef();
	}
	dwRet = Connect();
	m_suspended = ResumeThread(m_hThread);	//Thread启动
	m_suspended = 0;
	return dwRet;
}
DWORD  CheckAlive::Connect(void)
{
	DWORD dwRet = 0;
	if( m_pProxy == NULL )
	{
		if( m_svr.unType == 1 )
		{
			dwRet = m_socket.SendTo((DWORD)(void*)this,m_svr.TestPack,sizeof(m_svr.TestPack),
									0,0,CT2CA(m_svr.SvrIp.GetString()),m_svr.unPort,AF_INET);
		}
		else
		{
			dwRet = m_socket.ConnectEx((DWORD)(void*)this,CT2CA(m_svr.SvrIp.GetString()),m_svr.unPort,NULL,0);
		}
	}
	else
		dwRet = m_pProxy->onStart(&m_socket,this);
	if( (dwRet!=0)&&(dwRet!=997) )
		return dwRet;
	if( dwRet == 997 )
		dwRet = 0;
	return dwRet;
}
CAtlString	CheckAlive::State2String(State nD)
{
	CAtlString	strRet;
	if( nD == Connecting )
	{
		strRet = TEXT("正在连接...");
		return strRet;
	}
	if( nD == Connected )
	{
		strRet = TEXT("已经连接");
		return strRet;
	}
	if( nD == Disconnectd )
	{
		strRet = TEXT("已断开");
		return strRet;
	}	
	if( nD == 258 )
	{
		strRet = TEXT("连接超时");
		return strRet;
	}
	strRet = TEXT("error");
	return strRet;
}