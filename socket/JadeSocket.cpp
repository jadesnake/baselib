#include "stdafx.h"
#include "winsock.h"
#include "time.h"
#include "JadeSocket.h"
#pragma comment(lib,"Ws2_32")

#undef FD_SIZE
#define FD_SIZE 1024
//重新分配select所支持的套接字数目

JadeSocket::JadeSocket(UINT unlow,UINT unHigh,BOOL bAutoClean)
	: m_socket(NULL),m_Event(0),m_bSorM(false)
{
	WSADATA	wsaData = { 0 };
	m_bAutoClean = bAutoClean;
	WSAStartup(MAKEWORD(unlow,unHigh),&wsaData);
}

JadeSocket::JadeSocket()
	: m_socket(NULL),m_Event(0),m_bAutoClean(0),m_bSorM(false)
{


}

JadeSocket::~JadeSocket()
{
	if(m_socket != NULL)
		Clear();
	if(m_bAutoClean)
		WSACleanup();
}

DWORD	JadeSocket::Create(UINT nSocketPort,const char* lpszSocketAddress,int nSocketType,
						   BOOL bBind,UINT unAf,UINT unProtocol)
{
	if(m_socket)	return STILL_ACTIVE;	//仍然存活
	m_socket = socket(unAf,nSocketType,unProtocol);
	if(m_socket == INVALID_SOCKET)
		return WSAGetLastError();
	sockaddr_in	 tcpaddr = { 0 };
	if(lpszSocketAddress != NULL)
		tcpaddr.sin_addr.s_addr = htonl(inet_addr(lpszSocketAddress));
	else
		tcpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	tcpaddr.sin_family = unAf;
	tcpaddr.sin_port = htons(nSocketPort);
	if(!bBind)
		return 0;
	else
	{
		if( SOCKET_ERROR == bind(m_socket,(sockaddr*)&tcpaddr,sizeof(sockaddr_in)) )
			return WSAGetLastError();
	}
	return 0;
}

DWORD	JadeSocket::CreateEx(SOCKADDR_IN *pSockAddr,DWORD dwSockAddrLen,int nSocketType,BOOL bBind
						   ,UINT unAf,UINT unProtocol)
{
	if(m_socket)	return STILL_ACTIVE;	//仍然存活
	if( IsBadReadPtr(pSockAddr,dwSockAddrLen) )
		return -1;
	m_socket = socket(unAf,nSocketType,unProtocol);
	if(m_socket == INVALID_SOCKET)
		return WSAGetLastError();
	pSockAddr->sin_family = unAf;
	if(!bBind)
		return 0;
	else
	{
		if( SOCKET_ERROR == bind(m_socket,(sockaddr*)pSockAddr,dwSockAddrLen) )
			return WSAGetLastError();
	}
	return 0;
}

DWORD JadeSocket::GetSockName(char* rSocketAddress,UINT& rSocketPort,UINT &unAdrSize)
{
	SOCKADDR_IN	OutBuf = { 0 };
	int		Buflen = sizeof(SOCKADDR_IN);
	if(rSocketAddress == NULL)
		return 0x00001000;
	try
	{
		if(SOCKET_ERROR == getsockname(m_socket,(SOCKADDR*)&OutBuf,&Buflen))
			return WSAGetLastError();
		char	*lpAdr = inet_ntoa(OutBuf.sin_addr);
		Buflen = strlen(lpAdr);
		if(unAdrSize == 0 || (int)unAdrSize < Buflen)
			return Buflen;
		rSocketPort = ntohs(OutBuf.sin_port);
		strcpy_s(rSocketAddress,unAdrSize,lpAdr);
		return 0;
	}
	catch(...)
	{
		return 0x00001000;
	}
	return 0;
}

DWORD JadeSocket::GetSockName(SOCKADDR* lpSockAddr,int* lpSockAddrLen )
{
	if( (lpSockAddr == NULL)||(lpSockAddrLen == NULL) )
		return 0x00001000;
	try
	{

		if(SOCKET_ERROR == getsockname(m_socket,lpSockAddr,lpSockAddrLen))
			return WSAGetLastError();
	}
	catch(...)
	{
		return 0x00001000;
	}
	return 0;
}

DWORD JadeSocket::GetSockOpt(int nOptionName,void* lpOptionValue,int* lpOptionLen,int nLevel)
{
	if(lpOptionValue==NULL||lpOptionLen==NULL)
		return 0x00001000;
	try
	{
		if(SOCKET_ERROR==getsockopt(m_socket,nLevel,nOptionName,(char*)lpOptionValue,lpOptionLen))
			return WSAGetLastError();
	}
	catch(...)
	{
		return 0x00001000;
	}
	return 0;
}

DWORD JadeSocket::SetSockOpt(int nOptionName,const void* lpOptionValue,int nOptionLen,int nLevel)
{
	if(lpOptionValue == NULL)
		return 0x00001000;
	try
	{
		if(SOCKET_ERROR==setsockopt(m_socket,nLevel,nOptionName,(const char*)lpOptionValue,nOptionLen))
			return WSAGetLastError();
	}
	catch(...)
	{
		return 0x00001000;
	}
	return 0;
}

DWORD JadeSocket::Accept(SOCKET &ConnectedSocket,SOCKADDR* lpSockAddr,int* lpSockAddrLen)
{
	if(lpSockAddr == NULL||lpSockAddrLen==NULL)
		return 0x00001000;
	try
	{
		if(INVALID_SOCKET == (ConnectedSocket=accept(m_socket,lpSockAddr,lpSockAddrLen)))
			return WSAGetLastError();
		return 0;
	}
	catch(...)
	{
		return 0x00001000;	
	}
}

DWORD JadeSocket::Bind(UINT nSocketPort,const char* lpszSocketAddress,UINT unAf)
{
	sockaddr_in	 tcpaddr = { 0 };
	if(lpszSocketAddress != NULL)
		tcpaddr.sin_addr.s_addr = htonl(inet_addr(lpszSocketAddress));
	else
		tcpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	tcpaddr.sin_family = unAf;
	tcpaddr.sin_port = htons(nSocketPort);
	if( SOCKET_ERROR == bind(m_socket,(sockaddr*)&tcpaddr,sizeof(sockaddr_in)) )
		return WSAGetLastError();
	return 0;
}

DWORD JadeSocket::Bind(const SOCKADDR* lpSockAddr,int nSockAddrLen )
{
	if(lpSockAddr == NULL)
		return 0x00001000;
	try
	{
		if( SOCKET_ERROR==bind(m_socket,lpSockAddr,nSockAddrLen))
			return WSAGetLastError();
	}
	catch(...)
	{
		return 0x00001000;
	}
	return 0;
}
DWORD JadeSocket::Close()
{
	if( SOCKET_ERROR  == closesocket(m_socket) )
		return WSAGetLastError();
	m_socket = 0;	//释放资源
	return 0;
}

DWORD JadeSocket::IsConnected(long lusec)
{
	DWORD	dwRet = 0;
	char	chBuf[2] = { 0 };
	int		nLen = 2,nBuf = 0;
	fd_set	fdopt,fdEx;
	FD_ZERO(&fdopt);FD_ZERO(&fdEx);
	FD_SET(m_socket,&fdopt);FD_SET(m_socket,&fdEx);
	timeval	tval = { 0,0 };
	if( m_bSorM )
		tval.tv_usec = lusec;
	else
		tval.tv_usec = lusec;
	dwRet = select(1,&fdopt,NULL,&fdEx,&tval);
	if(FD_ISSET(m_socket,&fdopt))
	{
		FD_CLR(m_socket,&fdopt);
		FD_CLR(m_socket,&fdEx);
		dwRet = recv(m_socket,chBuf,nLen,MSG_PEEK);
		if(dwRet == SOCKET_ERROR)
			return WSAGetLastError();
		return dwRet == 0?0:-1;		//Read Event 触发并且读取缓冲区数据长度为0
									//说明链接已断开， -1表示已链接
	}
	if(FD_ISSET(m_socket,&fdEx))
	{
		FD_CLR(m_socket,&fdopt);
		FD_CLR(m_socket,&fdEx);
		nLen = sizeof(int);
		GetSockOpt(SO_ERROR,&nBuf,&nLen,SOL_SOCKET);
		return nBuf;
	}
	return 0;
}

DWORD JadeSocket::FlushIORecv(void)
{
	DWORD	dwRet = 0;
	char	chBuf[2] = { 0 };
	int		nLen = 2,nBuf = 0;
	fd_set	fdopt,fdEx;
	FD_ZERO(&fdopt);FD_ZERO(&fdEx);
	FD_SET(m_socket,&fdopt);FD_SET(m_socket,&fdEx);
	timeval	tval = { 0,0 };
	if( m_bSorM )
		tval.tv_usec = 2;
	else
		tval.tv_usec = 200;	
	dwRet = select(1,&fdopt,NULL,&fdEx,&tval);
	if(FD_ISSET(m_socket,&fdopt))
	{
		FD_CLR(m_socket,&fdopt);
		FD_CLR(m_socket,&fdEx);
		dwRet = recv(m_socket,chBuf,nLen,MSG_PEEK);
		switch(dwRet)
		{
		case SOCKET_ERROR:
			return WSAGetLastError();
		case 0:
			return 10053;			//链接已经被断开
		default:
			//有数据进来吃掉
			do
			{
				recv(m_socket,chBuf,dwRet,0);
			}while((dwRet = recv(m_socket,chBuf,nLen,MSG_PEEK)));
			return 0;	//OK已经全部吃掉
		}
	}
	if(FD_ISSET(m_socket,&fdEx))
	{
		FD_CLR(m_socket,&fdopt);
		FD_CLR(m_socket,&fdEx);
		nLen = sizeof(int);
		GetSockOpt(SO_ERROR,&nBuf,&nLen,SOL_SOCKET);
		return nBuf;
	}
	return -1;
}

DWORD JadeSocket::EnteryWait(UINT unOpt,long lSec,long luSec)
{
	DWORD	dwRet = 0;
	fd_set	fdopt,fdEx;	//connect
	FD_ZERO(&fdopt);
	FD_ZERO(&fdEx);
	FD_SET(m_socket,&fdopt);
	FD_SET(m_socket,&fdEx);
	timeval tval = { 0,0 };
	tval.tv_sec		=	lSec;
	tval.tv_usec	=	luSec;
	switch(unOpt)
	{
	case JadeSocket::WAITREAD:
		dwRet = select(1,&fdopt,NULL,&fdEx,&tval);
		break;
	case JadeSocket::WAITWRITE:
		dwRet = select(1,NULL,&fdopt,&fdEx,&tval);
		break;
	}
	if(FD_ISSET(m_socket,&fdEx))
	{
		FD_CLR(m_socket,&fdEx);
		dwRet = SOCKET_ERROR;
	}
	if(FD_ISSET(m_socket,&fdopt))
	{
		FD_CLR(m_socket,&fdopt);
		dwRet = 1;	//设置为非阻塞
		return IOCtl(FIONBIO,&dwRet);
	}
	else
	{
		switch(dwRet)
		{
		case SOCKET_ERROR:
			return SOCKET_ERROR;
		case 0:
			return ERROR_SOCK_TIMEOUT;
		default:
			return -1;
		}
	}
}

DWORD JadeSocket::LeaveWait()
{
	DWORD	dwRet = 0;	//恢复为阻塞模式
	return IOCtl(FIONBIO,&dwRet);
}

DWORD	JadeSocket::ConnectComplete(UINT unTimeOut)
{
	DWORD	dwRet = 0,dwLen = sizeof(DWORD);
	fd_set	Except_ready,Write_ready,Read_ready;	//connect
	FD_ZERO(&Except_ready);
	FD_ZERO(&Write_ready);
	FD_ZERO(&Read_ready);
	FD_SET(m_socket,&Except_ready);
	FD_SET(m_socket,&Write_ready);
	FD_SET(m_socket,&Read_ready);
	timeval tval = { 0,0 };
	if( m_bSorM )
		tval.tv_sec = unTimeOut;
	else
		tval.tv_usec = unTimeOut;
	if(tval.tv_sec < 0)
		tval.tv_sec = 60*60*60*60;
	if(tval.tv_usec < 0)
		tval.tv_usec = 60*60*60*60;
	dwRet = WSAGetLastError();
	if(dwRet == 10035 || dwRet == 10036)
	{
		//说明操作未完成
		dwRet = select(1,&Read_ready,&Write_ready,&Except_ready,&tval);
		if(FD_ISSET(m_socket,&Write_ready))
		{
			FD_CLR(m_socket,&Write_ready);
			if( SOCKET_ERROR == GetSockOpt(SO_ERROR,&dwRet,(int*)&dwLen,SOL_SOCKET) )
				return SOCKET_ERROR;
			if(dwRet != 0)
				return SOCKET_ERROR;
			return 0;		
		}
		if( SOCKET_ERROR == GetSockOpt(SO_ERROR,&dwRet,(int*)&dwLen,SOL_SOCKET) )
			return SOCKET_ERROR;
		switch(dwRet)
		{
		case SOCKET_ERROR:
			return SOCKET_ERROR;
		case 0:
			return ERROR_SOCK_TIMEOUT;
		}
	}
	return dwRet;
}

DWORD JadeSocket::Connect(const char* lpszHostAddress,UINT nHostPort,UINT unTimeOut,UINT unAf)
{
	SOCKADDR_IN sockadr = { 0 };
	sockadr.sin_port = htons(nHostPort);
	sockadr.sin_family = unAf;
	sockadr.sin_addr.s_addr = inet_addr(lpszHostAddress); 	
	if(sockadr.sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
		lphost = gethostbyname(lpszHostAddress);
		if (lphost != NULL)
			sockadr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		else
			return WSAEINVAL;
	}
	DWORD dwRet = 0,nCmd = 1;
	if(SOCKET_ERROR == ioctlsocket(m_socket,FIONBIO,&nCmd) )
		return WSAGetLastError();
	dwRet = SetSockOpt(SO_REUSEADDR,&nCmd,sizeof(int),SOL_SOCKET);
	if(dwRet != 0)
	{
		nCmd = 0;
		ioctlsocket(m_socket,FIONBIO,&nCmd);
		return dwRet;
	}
	if(SOCKET_ERROR==connect(m_socket,(SOCKADDR*)&sockadr,sizeof(SOCKADDR_IN)))
	{
		dwRet = ConnectComplete(unTimeOut);
		if(dwRet == 0)
		{
			//表示操作成功完成
			nCmd = 0;
			ioctlsocket(m_socket,FIONBIO,&nCmd);
			return 0;
		}
		//操作有错误
		return dwRet == ERROR_SOCK_TIMEOUT?ERROR_SOCK_TIMEOUT:WSAGetLastError();	
	}
	return 0;
}

DWORD JadeSocket::Connect(const SOCKADDR* lpSockAddr,int nSockAddrLen,UINT unTimeOut)
{
	if(lpSockAddr == NULL)
		return 0x00001000;
	//IO设置为非阻塞
	DWORD dwRet = 0,nCmd = 1;
	if(SOCKET_ERROR == ioctlsocket(m_socket,FIONBIO,&nCmd) )
		return WSAGetLastError();
	dwRet = SetSockOpt(SO_REUSEADDR,&nCmd,sizeof(int),SOL_SOCKET);
	if(dwRet != 0)
	{
		nCmd = 0;
		ioctlsocket(m_socket,FIONBIO,&nCmd);
		return dwRet;
	}
	if(SOCKET_ERROR==connect(m_socket,lpSockAddr,nSockAddrLen))
	{
		dwRet = ConnectComplete(unTimeOut);
		if(dwRet == 0)
		{
			//表示操作成功完成
			nCmd = 0;
			ioctlsocket(m_socket,FIONBIO,&nCmd);
			return 0;
		}
		//操作有错误
		return dwRet== ERROR_SOCK_TIMEOUT?ERROR_SOCK_TIMEOUT:WSAGetLastError();	
	}
	return 0;
}

DWORD JadeSocket::IOCtl(long lCommand,DWORD* lpArgument)
{
	if(lpArgument == NULL)
		return 0x00001000;
	if(SOCKET_ERROR == ioctlsocket(m_socket,lCommand,lpArgument))
		return WSAGetLastError();
	return 0;
}

DWORD JadeSocket::Listen(int nConnectionBacklog)
{
	return SOCKET_ERROR ==listen(m_socket,nConnectionBacklog)?
		WSAGetLastError():0;
}

DWORD JadeSocket::Receive(void* lpBuf,int nBufLen,int &nRevLen,long lTimeOut,int nFlags)
{
	DWORD nRet = 0,dwBytes = 0;
	if( m_bSorM )
		nRet = EnteryWait(JadeSocket::WAITREAD,lTimeOut);
	else
		nRet = EnteryWait(JadeSocket::WAITREAD,0,lTimeOut);
	if(nRet == 0)
	{
		//读取数据
		dwBytes = recv(m_socket,(char*)lpBuf,nBufLen,nFlags);
		if(dwBytes == SOCKET_ERROR)
		{
			nRevLen = 0;
			nRet = WSAGetLastError();
			LeaveWait();
			return nRet;
		}
		nRevLen = dwBytes;
		LeaveWait();
		return 0;
	}
	nRevLen = 0;
	return nRet;	//没有数据需要读取
}

DWORD JadeSocket::Send( const void* lpBuf, int nBufLen,int &nSedLen,UINT unTimeOut,int nFlags)
{
	if(lpBuf == NULL)
		return 0x00001000;
	DWORD nRet = 0,dwBytes = 0;
	if( m_bSorM )
		nRet = EnteryWait(JadeSocket::WAITWRITE,unTimeOut);
	else
		nRet = EnteryWait(JadeSocket::WAITWRITE,0,unTimeOut);
	if(nRet == 0)
	{
		dwBytes = send(m_socket,(const char*)lpBuf,nBufLen,nFlags);
		if(dwBytes == SOCKET_ERROR)
		{
			nSedLen = 0;
			nRet = WSAGetLastError();
			LeaveWait();
			return nRet;
		}
		nSedLen = dwBytes;
		LeaveWait();
		return 0;
	}
	nSedLen = 0;
	return nRet;
}

DWORD JadeSocket::SendTo(const void* lpBuf,int nBufLen,int &nSedLen,UINT unTimeOut,
						 UINT nHostPort,const char* lpszHostAddress,int nFlags,UINT unAf)
{
	if(lpBuf == NULL||lpszHostAddress==NULL)
		return 0x00001000;
	SOCKADDR_IN	sockadr = { 0 };
	sockadr.sin_port = (u_short)htonl(nHostPort);
	sockadr.sin_family = unAf;
	sockadr.sin_addr.s_addr = inet_addr(lpszHostAddress); 	
	if(sockadr.sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
		lphost = gethostbyname(lpszHostAddress);
		if (lphost != NULL)
			sockadr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		else
			return WSAEINVAL;
	}
	DWORD nRet = 0,dwBytes = 0;
	if( m_bSorM )
		nRet = EnteryWait(JadeSocket::WAITWRITE,unTimeOut);
	else
		nRet = EnteryWait(JadeSocket::WAITWRITE,0,unTimeOut);
	if(nRet == 0)
	{
		dwBytes = sendto(m_socket,(const char*)lpBuf,nBufLen,nFlags,(SOCKADDR*)&sockadr,
					sizeof(SOCKADDR_IN));
		if(dwBytes == SOCKET_ERROR)
		{
			nSedLen = 0;
			nRet = WSAGetLastError();
			LeaveWait();
			return nRet;
		}
		nSedLen = dwBytes;
		LeaveWait();
		return 0;
	}
	nSedLen = 0;
	return nRet;
}

DWORD JadeSocket::SendTo(const void* lpBuf,int nBufLen,int &nSedLen,UINT unTimeOut,const SOCKADDR* lpSockAddr, int nSockAddrLen, int nFlags)
{
	if(lpBuf == NULL)
		return 0x00001000;
	DWORD nRet = 0,dwBytes = 0;
	if( m_bSorM )
		nRet = EnteryWait(JadeSocket::WAITWRITE,unTimeOut);
	else
		nRet = EnteryWait(JadeSocket::WAITWRITE,0,unTimeOut);
	if(nRet == 0)
	{
		dwBytes = sendto(m_socket,(const char*)lpBuf,nBufLen,nFlags,lpSockAddr,nSockAddrLen);	
		if(dwBytes == SOCKET_ERROR)
		{
			nSedLen = 0;
			nRet = WSAGetLastError();
			LeaveWait();
			return nRet;
		}
		nSedLen = dwBytes;
		LeaveWait();
		return 0;
	}
	nSedLen = 0;
	return nRet;
}

DWORD JadeSocket::ReceiveFrom(void* lpBuf,int nBufLen,int &nRevLen,SOCKADDR* lpSockAddr,int* lpSockAddrLen,UINT unTimeOut,int nFlags)
{
	if(lpSockAddr==NULL||lpSockAddrLen==NULL)
		return 0x00001000;
	DWORD nRet = 0,dwBytes = 0;
	if( m_bSorM )
		nRet = EnteryWait(JadeSocket::WAITREAD,unTimeOut);
	else
		nRet = EnteryWait(JadeSocket::WAITREAD,0,unTimeOut);
	if(nRet == 0)
	{
		dwBytes = recvfrom(m_socket,(char*)lpBuf,nBufLen,nFlags,lpSockAddr,lpSockAddrLen);
		if(dwBytes == SOCKET_ERROR)
		{
			nRevLen = 0;
		 	nRet = WSAGetLastError();
			LeaveWait();
			return nRet;
		}
		nRevLen = dwBytes;
		LeaveWait();
		return 0;
	}
	nRevLen = 0;
	return nRet;	//没有数据需要读取
}
/*---------------------------------------------------------------------------------
		
					  
----------------------------------------------------------------------------------*/
char *	JadeSocket::Ipv4AddrToString(SOCKADDR_IN *SockAddr)
{
	if( SockAddr == NULL || IsBadReadPtr(SockAddr,sizeof(SOCKADDR_IN)) )
		return NULL;
	return inet_ntoa(SockAddr->sin_addr);
}

DWORD JadeSocket::ShutDown(int nHow)
{
	return SOCKET_ERROR == shutdown(m_socket,nHow)?WSAGetLastError():0;
}

DWORD JadeSocket::GetPeerName(char* rPeerAddress,UINT& rPeerPort,UINT unAdrSize)
{
	SOCKADDR_IN sockAddr = { 0 };
	int nSockAddrLen = sizeof(sockAddr);
	int nRet = getpeername(m_socket,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	if(nRet != SOCKET_ERROR)
	{
		rPeerPort = ntohs(sockAddr.sin_port);
		char *lpAddr = inet_ntoa(sockAddr.sin_addr);
		if(unAdrSize == 0|| (unAdrSize < strlen(lpAddr)+1) )
		{
			unAdrSize = strlen(lpAddr)+1;
			return 0;
		}
		strcpy_s(rPeerAddress,unAdrSize,lpAddr);
	}
	return WSAGetLastError();
}

DWORD JadeSocket::GetPeerName(SOCKADDR* lpSockAddr,int* lpSockAddrLen)
{
	if( (lpSockAddr == NULL)||(lpSockAddrLen == NULL) )
		return 0x00001000;
	try
	{
		
		if(SOCKET_ERROR == getpeername(m_socket,lpSockAddr,lpSockAddrLen))
			return WSAGetLastError();
	}
	catch(...)
	{
		return 0x00001000;
	}
	return 0;
}

DWORD JadeSocket::Clear()
{
	if( SOCKET_ERROR== closesocket(m_socket) )
		return WSAGetLastError();
	m_socket = NULL;
	return 0;
}

DWORD JadeSocket::AcceptByTimeout(SOCKET &ConnectedSocket,UINT unTimeout,int nRestart,SOCKADDR* lpSockAddr,
								  int* lpSockAddrLen)
{
	if(lpSockAddr==NULL||lpSockAddrLen==NULL)
		return 0x00001000;
	DWORD nRet = 0,dwBytes = 0;
	int nStart = nRestart;		//
	//IO设置为非阻塞
	DWORD dwRet = 0,nCmd = 1;
	if(SOCKET_ERROR == ioctlsocket(m_socket,FIONBIO,&nCmd) )
		return WSAGetLastError();
	dwRet = SetSockOpt(SO_REUSEADDR,&nCmd,sizeof(int),SOL_SOCKET);
	if(dwRet != 0)
	{
		nCmd = 0;
		ioctlsocket(m_socket,FIONBIO,&nCmd);
		return dwRet;
	}
	//等待客户连接
	//Begin
	fd_set	fdAccept;
	timeval tval = { 0,0 };
	if( m_bSorM )
		tval.tv_sec = unTimeout;
	else
		tval.tv_usec= unTimeout;
	do 
	{
		FD_ZERO(&fdAccept);
		FD_SET(m_socket,&fdAccept);
		nRet = select(0,&fdAccept,NULL,NULL,&tval);
		switch(nRet)
		{
		case 0:
			//超时
			if(nStart)
			{
				//检查重启次数还是非0则继续否则
				nStart--;
				continue;
			}
			else
			{
				nCmd = 0;
				if(SOCKET_ERROR == ioctlsocket(m_socket,FIONBIO,&nCmd) )
					return WSAGetLastError();
				return ERROR_SOCK_TIMEOUT;
			}
		case -1:
			return WSAGetLastError();
		case 1:
			//有客户端接入
			ConnectedSocket = accept(m_socket,lpSockAddr,lpSockAddrLen);
			if(ConnectedSocket == INVALID_SOCKET)
			{
				nCmd = 0;
				if(SOCKET_ERROR == ioctlsocket(m_socket,FIONBIO,&nCmd) )
					return WSAGetLastError();
				return WSAGetLastError();
			}
			else
			{
				nCmd = 0;
				if(SOCKET_ERROR == ioctlsocket(m_socket,FIONBIO,&nCmd) )
					return WSAGetLastError();
				return 0;	//表示操作正常
			}
		default:
			nCmd = 0;
			if(SOCKET_ERROR == ioctlsocket(m_socket,FIONBIO,&nCmd) )
				return WSAGetLastError();
			return nRet;
		}
	}while(1);
	//End
}
DWORD JadeSocket::Startup(WORD wVersion,WSADATA* lpWsaData,BOOL	bAutoClean)
{
	m_bAutoClean = bAutoClean;
	return WSAStartup(wVersion,lpWsaData);
}

/*-----------------------------------------------------------------------------------
				函数功能：读取网络一行数据
				参	  数：
						Input:
							lOvertime	超时设置
						Output:
							tgLine		输出行数据			
				返回值：
					0		——	成功
					非0		——	错误信息
				注意：
					这里我要演示一种比较独特的内存处理方法
					行要已\r\n作为结束标记
------------------------------------------------------------------------------------*/
DWORD JadeSocket::ReadLine(std::string &tgLine,UINT &unBufLen,long lOvertime)
{
	DWORD	dwRetVal = 0,dwTmp = 0,dwTotalLen = 0;
	int		nRecLen	 = 0,nRead = 0;
	char	*lpBuf = NULL,*lpTmp = NULL;
	HANDLE	hHeap  = HeapCreate(0,unBufLen,0);
	if(hHeap == NULL)
		return GetLastError();
	lpBuf = (char*)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,unBufLen);
	if(!lpBuf)
		return -1;	//缓冲建立失败
	//为了保证最后一位为'\0'所以需要保留一位数据
	while(!(dwRetVal = Receive(lpBuf,unBufLen-1,nRecLen,lOvertime,MSG_PEEK)))
	{
		lpBuf[nRecLen+1] = '\0';
		nRead = nRecLen;
		if(nRead == 0)
			return 10053;	//读取长度为0并且触发了读取事件则为断开链接
		const char *eol = strchr(lpBuf,'\n');
	    // if we found '\n', is there a '\r' as well?
        if( eol )
        {	
			if ( eol == lpBuf )
            {
                // check for case of "\r\n" being split
				nRecLen = tgLine.length();
				if (  nRecLen || tgLine.at(nRecLen) == '\r' )
                {
					eol =  NULL;
                }
                //else: ok, got real EOL				
                // read just this '\n' and restart
                nRead = 1;
            }
            else // '\n' in the middle of the buffer
            {
                // in any case, read everything up to and including '\n'
                nRead = eol - lpBuf + 1;
                if ( eol[-1] != '\r' )
                {
                    // as above, simply ignore stray '\n'
                    eol = NULL;
                }
            }
		 }
		dwRetVal = Receive(lpBuf,unBufLen-1,nRecLen,lOvertime);
		if(nRead != nRecLen)
		{
			if(hHeap)	HeapDestroy(hHeap);
			return ERROR_PROTO_NETERR;
		}
		tgLine += lpBuf;
		ZeroMemory(lpBuf,unBufLen);
	}
	if(hHeap)	HeapDestroy(hHeap);
	return dwRetVal;
}
