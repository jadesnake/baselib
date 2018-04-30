#include "stdafx.h"
#include "proxysock5.h"
#include "JadeSocket.h"

ProxySock5::ProxySock5(const Proxy& proxy)
	:m_socket(NULL),m_pbase(NULL),m_unRetry(0)
{
	m_proxy = proxy;
	m_state	= S5StateDisconnected;
	
	m_udpSock = new JadeSocket;
	m_udpSock->Create(0,NULL,SOCK_DGRAM);
	m_udpSock->m_bAutoClean = false;
	m_udpSock->m_bSorM = false;
}
ProxySock5::~ProxySock5()
{
	if( m_udpSock )
		delete m_udpSock;
}
bool  ProxySock5::SendUdpPack(int ip,u_short unPort)
{
	/* rfc1928
		+----+------+------+----------+----------+----------+
		|RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
		+----+------+------+----------+----------+----------+
		| 2  |  1   |  1   | Variable |    2     | Variable |
		+----+------+------+----------+----------+----------+
		o  RSV  Reserved X'0000'
		o  FRAG    Current fragment number
		o  ATYP    address type of following addresses:
		o  IP V4 address: X'01'
		o  DOMAINNAME: X'03'
		o  IP V6 address: X'04'
		o  DST.ADDR       desired destination address
		o  DST.PORT       desired destination port
		o  DATA     user data
	*/	
	CheckAlive::Service* svr = m_pbase->GetService();
	
	sockaddr_in proxyAddr;
	memset( &proxyAddr, 0, sizeof( proxyAddr ) );
	proxyAddr.sin_port = unPort;
	proxyAddr.sin_addr.s_addr = ip;
	proxyAddr.sin_family = AF_INET;

	m_state = S5StateUpdConfirm;
	DWORD	dwRet = 0;
	int	 nHasSend = 0;	
	bool bIp= IsIp(svr->SvrIp); 
	int  pos= 4+(bIp?4:svr->SvrIp.GetLength()+1)+2+strlen(svr->TestPack);
	char *d = new char[pos];
	pos = 0;
	d[pos++] = 0;
	d[pos++] = 0;
	d[pos++] = 0;
	if( bIp )
	{
		d[pos++] = 0x01;
		DWORD dwAdr  = inet_addr( CT2CA(svr->SvrIp) );
		memcpy(d+pos,&dwAdr,sizeof(DWORD));
		pos += sizeof(DWORD);
	}
	else
	{
		std::string server( CT2CA(svr->SvrIp) );
		d[pos++] = 0x03; // hostname
		d[pos++] = (char)svr->SvrIp.GetLength();
		memcpy(d + pos,server.c_str(), server.length());
		pos += server.length();
	}
	UINT unP = htons(svr->unPort);
	memcpy(d+pos,&unP,2);
	pos += 2;
	memcpy(d+pos,svr->TestPack,strlen(svr->TestPack));
	pos += strlen(svr->TestPack);
	dwRet = m_udpSock->SendTo(d,pos,nHasSend,m_pbase->GetService()->unTimout,
						   (SOCKADDR*)&proxyAddr,sizeof(proxyAddr));
	if( dwRet )
	{
		delete []d;
		m_state = S5StateError;
		//关闭连接
		m_socket->Disconnect(0,0,0,NULL);
		return false;
	}
	delete []d;
	
	char chTmp[128];
	int  adrLen = sizeof(proxyAddr);
	int  nTm = m_pbase->GetService()->unTimout;
	if( nTm == -1 || nTm == 0 )
		nTm = 1500;
	nTm = nTm/100;		//每次等待10毫秒，那么大概就是设置的描述的1/10次循环
	pos = 0;			//pos用于存储接收到的数据
	for(int nI=0;nI < nTm;nI++)
	{
		dwRet = m_udpSock->ReceiveFrom(chTmp,sizeof(chTmp),pos,(SOCKADDR*)&proxyAddr,
									  &adrLen,nTm);
		if( pos )
			break;
	}
	if( pos )
	{
		m_state = S5StateConnected;
		m_pbase->DoCallBack(State2String(m_state),false,false);
	}
	else
	{
		//关闭连接
		m_socket->Disconnect(0,0,0,NULL);
	}
	return true;
}
bool  ProxySock5::IsIp(const CAtlString& strV)
{
	//通过.字符判断是ip还是域名
	int nPos = 0,nCount = 0;	//nCount用于计数
	while( nPos=strV.Find('.',nPos) )
	{
		if( nPos == -1 )
			break;
		nPos++;
		nCount++;
	}
	return (nCount>=3?true:false);
}
void  ProxySock5::negotiate(void)
{
	CheckAlive::Service *pSvr = m_pbase->GetService();
	bool bIp = IsIp(pSvr->SvrIp);
	m_state = S5StateNegotiating;
	char* d = new char[bIp ? 10 : 6 + pSvr->SvrIp.GetLength() + 1];
	size_t pos = 0;
	d[pos++] = 0x05;		     // SOCKS version 5
	if( pSvr->unType )
		d[pos++] = 0x03;		 // command CONNECT udp
	else
		d[pos++] = 0x01;		 // command CONNECT tcp
	d[pos++] = 0x00;		     // reserved
	
	std::string server;
	int nport = 0;
	if( pSvr->unType )
	{
		sockaddr_in localAddr;
		int nSize = sizeof( localAddr );
		server = "0.0.0.0";
		::getsockname(m_udpSock->m_socket, (sockaddr*)&localAddr, &nSize );
		nport  = localAddr.sin_port;
	}
	else
	{
		server = CT2CA(pSvr->SvrIp);
		nport = htons( pSvr->unPort );
	}
	if( bIp )				// IP address
	{
		d[pos++] = 0x01;	// IPv4 address
		DWORD addr = inet_addr(server.c_str());
		memcpy(d+pos,&addr,4);
		pos += 4;
	}
	else // hostname
	{
		d[pos++] = 0x03; // hostname
		d[pos++] = (char)server.length();
		memcpy(d + pos,server.c_str(), server.length());
		pos += server.length();
	}
	memcpy(d+pos,&nport,2);
	pos+=2;
	//d[pos++] = static_cast<char>( nport );
	//d[pos++] = static_cast<char>( nport >> 8 );	
	SendIo(d,pos);

	delete[] d;
}
DWORD ProxySock5::onStart(JadeSocketIocp<> *socket,CheckAlive *pbase)
{
	DWORD	dwRet = 0;
	m_socket = socket;
	m_pbase  = pbase;
	dwRet = m_socket->ConnectEx(0,CT2CA(m_proxy.SvrIp.GetString()),m_proxy.unPort,NULL,0);
	if( dwRet != 997 )
	{
		m_socket->Disconnect(0,0,0,0);
	}
	return dwRet;
}
bool  ProxySock5::IsConnected(void)
{
	return (m_state == S5StateConnected);
}
void  ProxySock5::onRecv(PRECV pRecv,DWORD &dwID)
{
	char *pBuf = (char*)pRecv->pBuf;
	if( m_state == S5StateConnecting )
	{
		if( pRecv->dwLen != 2 || pBuf[0] != 0x05 )
		{
			m_state = S5StateError;
			m_socket->Disconnect(0,0,0,0);
			return ;
		}
		if( pBuf[1] == 0x00 ) // no auth
		{
			negotiate();
			m_pbase->DoCallBack(State2String(m_state),false,false);
			return ;
		}
		else if( pBuf[1] == 0x02 && !m_proxy.name.IsEmpty() && !m_proxy.pass.IsEmpty() ) 
		{
			// user/password auth
			m_state = S5StateAuthenticating;
			std::string strName(CT2CA(m_proxy.name.GetString()));
			std::string strPass(CT2CA(m_proxy.pass.GetString()));
			char* d = new char[3+m_proxy.name.GetLength()+m_proxy.pass.GetLength()];
			size_t pos = 0;
			d[pos++] = 0x01;
			d[pos++] = (char)strName.length();
			memcpy(d+pos,strName.c_str(),strName.length());
			pos += strName.length();
			d[pos++] = (char)strPass.length();
			memcpy(d+pos,strPass.c_str(), strPass.length() );
			pos += strPass.length();

			SendIo(d,pos);
			delete[] d;
			m_pbase->DoCallBack(State2String(m_state),false,false);
			return	  ;
		}
		else
		{
			if( pBuf[1] == (char)(unsigned char)0xFF && 
				!m_proxy.name.IsEmpty() && !m_proxy.pass.IsEmpty() )
			{
				m_state = S5StateProxyNoSupportedAuth;

			}
			else
			{
				m_state = S5StateProxyAuthRequired;
			}
			m_pbase->DoCallBack(State2String(m_state),false,false);
			m_socket->Disconnect(0,0,0,0);
			return ;
		}
	}	
	if( m_state == S5StateAuthenticating )
	{
		if( pRecv->dwLen == 2 && pBuf[0] == 0x01 && pBuf[1] == 0x00 )
		{
			negotiate();
		}
		else
		{
			m_state = S5StateProxyAuthFailed;
			m_pbase->DoCallBack(State2String(m_state),false,false);
			m_socket->Disconnect(0,0,0,0);
		}
		return ;
	}
	if( m_state == S5StateNegotiating )
	{
		if( pRecv->dwLen >= 6 && pBuf[0] == 0x05 )
		{
			if( pBuf[1] == 0x00 )
			{
				if( m_pbase->GetService()->unType )
				{
					if( 0x01 == pBuf[3] ) 
					{
						m_udpIp.clear();
						m_udpPort.clear();

						m_udpIp.append(pBuf+3,5);
						m_udpPort.append(pBuf+8,2);

						in_addr addr;
						u_short port;
						memcpy(&addr,pBuf+4,4);
						memcpy(&port,pBuf+8,2);

						SendUdpPack(addr.S_un.S_addr,port);	
						return ;
					}
					else if( 0x03 == pBuf[3] )
					{
						m_udpIp.clear();
						m_udpPort.clear();

						m_udpIp.append(pBuf+4,pBuf[4]);
						m_udpPort.append(pBuf+4+pBuf[4],2);
					
						u_short port;
						memcpy(&port,pBuf+4+pBuf[4],2);
						SendUdpPack(htonl(inet_addr(m_udpIp.c_str())),port);	
						return ;
					}
				}
				else
				{
					m_state = S5StateConnected;
				}
			}
			else
			{
				m_state = S5StateConnectionRefused;
			}
		}
		else
		{
			m_state = S5StateError;
		}
		m_pbase->DoCallBack(State2String(m_state),
						    m_state==S5StateConnected,false);
		return ;
	}
	return ;
}
CAtlString	ProxySock5::State2String(Socks5State n)
{
	CAtlString	strRet;
	if( n == S5StateDisconnected )
	{
		strRet = TEXT("连接断开");
		return strRet;
	}
	if( n == S5StateConnecting )
	{
		strRet = TEXT("正在连接");
		return strRet;
	}
	if( n == S5StateNegotiating )
	{
		strRet = TEXT("正在握手协商");
		return strRet;
	}
	if( n == S5StateAuthenticating )
	{
		strRet = TEXT("正咋进行身份验证");
		return strRet;
	}
	if( n == S5StateConnected )
	{
		strRet = TEXT("已连接");
		return strRet;
	}
	if( n == S5StateConnectionRefused )
	{
		strRet = TEXT("连接被拒绝");
		return strRet;
	}
	if( n == S5StateProxyNoSupportedAuth )
	{
		strRet = TEXT("代理服务不支持身份验证");
		return strRet;
	}
	if( n == S5StateProxyAuthRequired )
	{
		strRet = TEXT("代理服务需要身份验证");
		return strRet;
	}
	if( n == S5StateProxyAuthFailed )
	{
		strRet = TEXT("代理服务身份认证错误");
		return strRet;
	}
	if( n == S5StateError )
	{
		strRet = TEXT("发生错误");
		return strRet;
	}
	if( n == 10053 )
	{
		strRet = TEXT("连接被拒绝");
		return strRet;
	}
	if( n == S5StateUpdConfirm )
	{
		strRet = TEXT("UDP连接验证失败");
		return strRet;
	}
	return strRet;
}
void  ProxySock5::onSend(PSEND pSend,DWORD &dwID)
{
	if( m_socket )
	{
		DWORD	dwFlags = 0,dwRet = 0;
		if( m_state == S5StateConnecting )
			dwRet = m_socket->Receive(m_state,0x02,dwFlags,NULL);
		else if( m_state == S5StateNegotiating )
			dwRet = m_socket->Receive(m_state,0xFF,dwFlags,NULL);
		else if( m_state == S5StateAuthenticating )
			dwRet = m_socket->Receive(m_state,0xFF,dwFlags,NULL);
		else if( m_state == S5StateUpdConfirm )
			dwRet = m_socket->Receive(m_state,0xFF,dwFlags,NULL);
		if( dwRet != 997 )	//出现错误
			m_socket->Disconnect(0,0,0,NULL);
	}
}
void  ProxySock5::onConnect(PCONNECTEX pConnect,DWORD &dwID)
{
	const bool auth = !m_proxy.name.IsEmpty() && !m_proxy.pass.IsEmpty();
	const char d[4] = {
		0x05,                             // SOCKS version 5
		static_cast<char>( auth ? 0x02    // two methods
		: 0x01 ),						  // one method
		0x00,                             // method: no auth
		0x02                              // method: username/password auth
	};
	std::string pack(d, auth ? 4 : 3 );
	if( !SendIo((char*)pack.c_str(),pack.length()) )
	{
		m_socket->Disconnect(0,0,0,0);
	}
	else
		m_state = S5StateConnecting;
}
void  ProxySock5::onDisconnect(DWORD &dwID)
{
	if( m_state != S5StateConnected )
		m_pbase->DoCallBack(State2String(m_state),false,true);
}
bool  ProxySock5::SendIo(char *pBuf,UINT unLen)
{
	DWORD dwRet = 0;
	if( m_socket )
	{
		dwRet = m_socket->Send(m_state,pBuf,unLen,0,0,NULL);
	}
	if( dwRet != 997 )
	{
		m_socket->Disconnect(0,0,0,0);
		return false;
	}
	return true;
}