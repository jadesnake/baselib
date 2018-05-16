
template< typename BlockX >
JadeSocketIocp< BlockX >::JadeSocketIocp():m_lpfnAcceptEx(NULL),m_hIocp(NULL),m_socket(0),m_bAutoClean(FALSE)
										  ,m_hHeap(NULL),m_lpfnGetAcceptExSockaddrs(NULL),m_lpfnConnectEx(NULL)
										  ,m_lpfnDisconnectEx(NULL) 
{
	srand( (unsigned)time( NULL ) );
	m_dwKey = rand();
}

template< typename BlockX>
JadeSocketIocp< BlockX >::JadeSocketIocp(UINT unlow,UINT unHigh,BOOL bAutoClean):m_lpfnAcceptEx(NULL),m_hIocp(NULL),
										m_socket(0),m_bAutoClean(bAutoClean),m_hHeap(NULL)
										,m_lpfnGetAcceptExSockaddrs(NULL),m_lpfnConnectEx(NULL)
										,m_lpfnDisconnectEx(NULL)
{
	WSADATA	wsaData = { 0 };
	SYSTEM_INFO	tgSysInfo = { 0 };
	GetSystemInfo(&tgSysInfo);
	WSAStartup(MAKEWORD(unlow,unHigh),&wsaData);
	srand( (unsigned)time( NULL ) );
	m_dwKey = rand();
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,m_dwKey,tgSysInfo.dwNumberOfProcessors*2+2);
	m_hHeap = HeapCreate(0,tgSysInfo.dwPageSize,0);
}

template< typename BlockX>
JadeSocketIocp< BlockX >::~JadeSocketIocp()
{
	CancelIo((HANDLE)m_socket);
	if(m_bAutoClean)
		WSACleanup();
	if(m_socket)
	{
		closesocket(m_socket);
		m_socket = NULL;
	}
	if(m_hHeap)
	{
		HeapDestroy(m_hHeap);
		m_hHeap = NULL;
	}
	if(m_hIocp)
	{
		CloseHandle(m_hIocp);
		m_hIocp = NULL;
	}
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::Associate(HANDLE hIocp,DWORD dwKey)
{
	SYSTEM_INFO	tgSysInfo = { 0 };
	GetSystemInfo(&tgSysInfo);
	if(m_hIocp)
	{
		CloseHandle(m_hIocp);
		m_hIocp = NULL; 
	}
	return CreateIoCompletionPort((HANDLE)m_socket,hIocp,dwKey,tgSysInfo.dwNumberOfProcessors*2+2) != NULL ? 0 : GetLastError();
}

template< typename BlockX>
DWORD	JadeSocketIocp< BlockX >::Associate(HANDLE hIocp,SOCKET nSock)
{
	SYSTEM_INFO	tgSysInfo = { 0 };
	GetSystemInfo(&tgSysInfo);
	return CreateIoCompletionPort((HANDLE)nSock,hIocp,m_dwKey,tgSysInfo.dwNumberOfProcessors*2+2) != NULL ? 0 : GetLastError();
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::PostUserDefined(DWORD dwID,const BlockX &pUserX,DWORD *dwIdentify)
{
	PSOCKETKEY	pKey = NULL;
	if(m_hHeap == NULL)
		return -1;
	try
	{
		pKey = (PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!pKey)
			throw (DWORD)-1;
		pKey->lpValue	=	HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(BlockX));
		if(!pKey->lpValue)
			throw (DWORD)-1;
		pKey->nOperateType	=	UNKNOWN_USERDATA;
		pKey->dwID			=	dwID;
		memcpy(pKey->lpValue,&pUserX,sizeof(BlockX));
		if(!PostQueuedCompletionStatus(m_hIocp,0,m_dwKey,&pKey->tgOverlap))
			throw (DWORD)GetLastError();
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = (DWORD)pKey;
	}
	catch (DWORD dwCode)
	{
		if(pKey && !IsBadReadPtr(pKey,sizeof(SOCKETKEY)))
		{
			if(pKey->lpValue && !IsBadReadPtr(pKey->lpValue,sizeof(BlockX)))
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey->lpValue);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey);
		}
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return dwCode;
	}
	catch ( ... )
	{
		if(pKey && !IsBadReadPtr(pKey,sizeof(SOCKETKEY)))
		{
			if(pKey->lpValue && !IsBadReadPtr(pKey->lpValue,sizeof(BlockX)))
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey->lpValue);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey);
		}
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return -1;
	}
	return 0;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::Listen(int nBacklog /* = SOMAXCONN */)
{
	return  listen(m_socket,nBacklog) == SOCKET_ERROR ? WSAGetLastError() : 0;
}

template< typename BlockX>
char *	JadeSocketIocp< BlockX >::Ipv4AddrToString(SOCKADDR_IN *SockAddr)
{
	if( SockAddr == NULL || IsBadReadPtr(SockAddr,sizeof(SOCKADDR_IN)) )
		return NULL;
	return inet_ntoa(SockAddr->sin_addr);
}

template< typename BlockX >
int		JadeSocketIocp< BlockX >::Startup(WORD wVersion,WSADATA* lpWsaData,BOOL bAutoClean)
{
	SYSTEM_INFO	tgSysInfo = { 0 };
	DWORD		dwRetVal = 0;
	if( lpWsaData == NULL)
		return -1;
	if( IsBadReadPtr(lpWsaData,sizeof(WSADATA)) )
		return -1;
	if(m_hIocp)
		CloseHandle(m_hIocp);
	GetSystemInfo(&tgSysInfo);
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,m_dwKey,tgSysInfo.dwNumberOfProcessors*2+2);
	if(!m_hIocp)
		return GetLastError();
	m_hHeap = HeapCreate(0,tgSysInfo.dwPageSize,0);
	if(!m_hHeap)
	{
		HeapDestroy(m_hHeap);
		m_hHeap		= NULL;
		dwRetVal	= GetLastError();
		CloseHandle(m_hIocp);
		return dwRetVal;
	}
	m_bAutoClean = bAutoClean;
	return WSAStartup(wVersion,lpWsaData);
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::InitResource(void)
{
	SYSTEM_INFO	tgSysInfo = { 0 };
	DWORD		dwRetVal = 0;
	if(m_hIocp)
		CloseHandle(m_hIocp);
	GetSystemInfo(&tgSysInfo);
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,m_dwKey,tgSysInfo.dwNumberOfProcessors*2+2);
	if(!m_hIocp)
		return GetLastError();
	if(m_hHeap)
		HeapDestroy(m_hHeap);
	m_hHeap = HeapCreate(0,tgSysInfo.dwPageSize,0);
	if(!m_hHeap)
	{
		HeapDestroy(m_hHeap);
		m_hHeap		= NULL;
		dwRetVal	= GetLastError();
		CloseHandle(m_hIocp);
		return dwRetVal;
	}
	return 0;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::Create(UINT nSocketPort,const char* lpszSocketAddress,
										 int nSocketType,DWORD	dwFlags,
										 LPWSAPROTOCOL_INFO lpProtocolInfo,BOOL bBind,
										 UINT unAf,UINT unProtocol)
{
	sockaddr_in	tcpaddr		 = { 0 };
	GUID		GuidAcceptEx			 = WSAID_ACCEPTEX;
	GUID		GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	GUID		GuidConnectEx			 = WSAID_CONNECTEX;
	GUID		GuidDisconnectEx		 = WSAID_DISCONNECTEX;
	DWORD		dwBytes		 = 0;
	int			nRetVal		 = 0; 
	try
	{
		if(m_socket)	return STILL_ACTIVE;	//仍然存活
		m_socket = WSASocket(unAf,nSocketType,unProtocol,lpProtocolInfo,0,dwFlags);
		if( m_socket == INVALID_SOCKET)
			return  WSAGetLastError();
		if(lpszSocketAddress != NULL)
			tcpaddr.sin_addr.s_addr = htonl(inet_addr(lpszSocketAddress));
		else
			tcpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		tcpaddr.sin_family = unAf;
		tcpaddr.sin_port = htons(nSocketPort);
		//取函数指针
		nRetVal =	::WSAIoctl(m_socket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidAcceptEx,sizeof(GuidAcceptEx)
							,&m_lpfnAcceptEx,sizeof(m_lpfnAcceptEx),&dwBytes,NULL,NULL);
		if(nRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		nRetVal	=	::WSAIoctl(m_socket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidGetAcceptExSockaddrs
							,sizeof(GuidGetAcceptExSockaddrs),&m_lpfnGetAcceptExSockaddrs
							,sizeof(m_lpfnGetAcceptExSockaddrs),&dwBytes,NULL,NULL);
		if(nRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		nRetVal =	::WSAIoctl(m_socket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidConnectEx,sizeof(GuidConnectEx)
							,&m_lpfnConnectEx,sizeof(m_lpfnConnectEx),&dwBytes,NULL,NULL);
		if(nRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		nRetVal = ::WSAIoctl(m_socket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidDisconnectEx,sizeof(GuidDisconnectEx)
							,&m_lpfnDisconnectEx,sizeof(m_lpfnDisconnectEx),&dwBytes,NULL,NULL);
		if(nRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		dwBytes = Associate(m_hIocp,m_socket);
		if(dwBytes)	throw dwBytes;
		if(!bBind)
			return 0;
		else
		{
			if( SOCKET_ERROR == bind(m_socket,(sockaddr*)&tcpaddr,sizeof(sockaddr_in)) )
				throw (DWORD)WSAGetLastError();
		}
	}
	catch (DWORD dwCode)
	{
		closesocket(m_socket);
		m_socket = NULL;
		return dwCode;
	}
	return 0;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::CreateEx(SOCKADDR_IN *pSockAddr,DWORD dwSockAddrLen,
										LPWSAPROTOCOL_INFO lpProtocolInfo,
										int nSocketType,DWORD dwFlags,BOOL bBind,UINT unAf,UINT unProtocol)
{
	INT		nRetVal = 0;
	DWORD	dwBytes = 0;
	GUID		GuidAcceptEx			 = WSAID_ACCEPTEX;
	GUID		GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	GUID		GuidConnectEx			 = WSAID_CONNECTEX;
	GUID		GuidDisconnectEx		 = WSAID_DISCONNECTEX;
	if(m_socket)	return STILL_ACTIVE;	//仍然存活
	if( IsBadReadPtr(pSockAddr,dwSockAddrLen) )
		return -1;
	m_socket = WSASocket(unAf,nSocketType,unProtocol,lpProtocolInfo,0,dwFlags);
	if(m_socket == INVALID_SOCKET)
		return WSAGetLastError();
	pSockAddr->sin_family = unAf;
	//取函数指针
	try
	{
		nRetVal =	::WSAIoctl(m_socket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidAcceptEx,sizeof(GuidAcceptEx)
								,&m_lpfnAcceptEx,sizeof(m_lpfnAcceptEx),&dwBytes,NULL,NULL);
		if(nRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		nRetVal	=	::WSAIoctl(m_socket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidGetAcceptExSockaddrs
								,sizeof(GuidGetAcceptExSockaddrs),&m_lpfnGetAcceptExSockaddrs
								,sizeof(m_lpfnGetAcceptExSockaddrs),&dwBytes,NULL,NULL);
		if(nRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		nRetVal =	::WSAIoctl(m_socket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidConnectEx,sizeof(GuidConnectEx)
								,&m_lpfnConnectEx,sizeof(m_lpfnConnectEx),&dwBytes,NULL,NULL);
		if(nRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		nRetVal = ::WSAIoctl(m_socket,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidDisconnectEx,sizeof(GuidDisconnectEx)
								,&m_lpfnDisconnectEx,sizeof(m_lpfnDisconnectEx),&dwBytes,NULL,NULL);
		if(nRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		dwBytes = Associate(m_hIocp,m_socket);
		if(dwBytes)	throw dwBytes;
		if(!bBind)
			return 0;
		else
		{
			if( SOCKET_ERROR == bind(m_socket,(sockaddr*)pSockAddr,dwSockAddrLen) )
				return WSAGetLastError();
		}
	}
	catch (DWORD dwCode)
	{
		closesocket(m_socket);
		m_socket = NULL;
		return dwCode;
	}
	return 0;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::GetSockName(char* rSocketAddress,UINT& rSocketPort,UINT &unAdrSize)
{
	SOCKADDR_IN	OutBuf = { 0 };
	int		Buflen = sizeof(SOCKADDR_IN);
	if(rSocketAddress == NULL)
		return	-1;
	try
	{
		if(SOCKET_ERROR == getsockname(m_socket,(SOCKADDR*)&OutBuf,&Buflen))
			return WSAGetLastError();
		char	*lpAdr = inet_ntoa(OutBuf.sin_addr);
		Buflen = strlen(lpAdr);
		if(unAdrSize == 0 || unAdrSize < Buflen)
			return Buflen;
		rSocketPort = ntohs(OutBuf.sin_port);
		strcpy(rSocketAddress,lpAdr);
	}
	catch(...)
	{
		return -1;
	}
	return 0;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::GetSockName(SOCKADDR* lpSockAddr,int* lpSockAddrLen )
{
	if( (lpSockAddr == NULL)||(lpSockAddrLen == NULL) )
		return	-1;
	try
	{
		if(SOCKET_ERROR == getsockname(m_socket,lpSockAddr,lpSockAddrLen))
			return WSAGetLastError();
	}
	catch(...)
	{
		return	-1;
	}
	return 0;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::GetSockOpt(int nOptionName,void* lpOptionValue,int* lpOptionLen,int nLevel)
{
	if(lpOptionValue==NULL||lpOptionLen==NULL)
		return	-1;
	try
	{
		if(SOCKET_ERROR==getsockopt(m_socket,nLevel,nOptionName,(char*)lpOptionValue,lpOptionLen))
			return WSAGetLastError();
	}
	catch(...)
	{
		return -1;
	}
	return 0;	
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::SetSockOpt(int nOptionName,const void* lpOptionValue,int nOptionLen,int nLevel)
{
	if(lpOptionValue == NULL)
		return	-1;
	try
	{
		if(SOCKET_ERROR==setsockopt(m_socket,nLevel,nOptionName,(const char*)lpOptionValue,nOptionLen))
			return WSAGetLastError();
	}
	catch(...)
	{
		return -1;
	}
	return 0;
}
/************************************************************************\
					函数名称：AcceptEx
					函数功能：异步接收网络链接
					函数参数：
					Input:
							SOCKET		ConnectedSocket		Socket连接
							DWORD		dwDataBufLen		pOutputBuffer长度
					Output：
							DWORD		&dwIdentify			标示该操作
					返 回 值：
							0		——		成功
							非0		——		失败
					备注：
							DWORD	dwBufferLen 变量表示当接收连接时所接受的数据长度。
							该数据应为SOCKADDR+16个字节并乘2（因为有远程和本地之分）其余的数据为网络
							IO交换数据。
\************************************************************************/
template< typename BlockX>
DWORD	JadeSocketIocp< BlockX >::AcceptEx(DWORD dwID,SOCKET ConnectedSocket,DWORD dwDataBufLen,BOOL bInherit,DWORD *dwIdentify)
{
	DWORD		dwReceiveDataLen = 0,dwRetVal = 0;
	PSOCKETKEY	pSockKey = NULL;
	BOOL		bSuc = FALSE;
	PACCEPTEX	pAcceptOpt = NULL;
	INT			nLocalRecvLen = 0,nRemoteRecvLen = 0;
	if(m_lpfnAcceptEx == NULL || IsBadReadPtr(m_lpfnAcceptEx,sizeof(LPFN_ACCEPTEX)))
		return -1;
	if( m_lpfnGetAcceptExSockaddrs == NULL || IsBadReadPtr(m_lpfnGetAcceptExSockaddrs,sizeof(LPFN_GETACCEPTEXSOCKADDRS)))
		return -1;
	pAcceptOpt = (PACCEPTEX)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(ACCEPTEX));
	if(!pAcceptOpt)
		return -1;
	try
	{
		pAcceptOpt->dwBufLen  = ADDRRESS_LENGTH*2+dwDataBufLen;
		pAcceptOpt->wSocket	  = ConnectedSocket;
		pAcceptOpt->bInherit  = bInherit;
		pAcceptOpt->lpDataBuf =	HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,pAcceptOpt->dwBufLen);
		if(!pAcceptOpt->lpDataBuf)
			throw (DWORD)-1;
		
		pSockKey = (PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!pSockKey)
			throw (DWORD)-1;
		
		pSockKey->nOperateType	=	SOCKET_ACCEPTEX;
		pSockKey->lpValue		=	(void*)pAcceptOpt;
		pSockKey->dwID			=	dwID;
		bSuc = m_lpfnAcceptEx(m_socket,ConnectedSocket,pAcceptOpt->lpDataBuf,dwDataBufLen,ADDRRESS_LENGTH
							 ,ADDRRESS_LENGTH,&dwReceiveDataLen,&pSockKey->tgOverlap);
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = (DWORD)pSockKey;
		dwRetVal = WSAGetLastError();
		return (dwRetVal != ERROR_IO_PENDING) && (dwRetVal != 0) ? throw dwRetVal : ERROR_IO_PENDING;
	}
	catch (DWORD dwCode)
	{
		if(pAcceptOpt && !IsBadReadPtr(pAcceptOpt,sizeof(ACCEPTEX)))
		{
			if(pAcceptOpt->lpDataBuf && !IsBadReadPtr(pAcceptOpt->lpDataBuf,pAcceptOpt->dwBufLen))
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pAcceptOpt->lpDataBuf);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pAcceptOpt);
		}
		if(pSockKey && !IsBadReadPtr(pSockKey,sizeof(SOCKETKEY)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return dwCode;
	}
}

template< typename BlockX>
DWORD	JadeSocketIocp< BlockX >::WaitSocketComplete(DWORD &dwID,UINT &nOperateType,LPVOID *lpOptVal,DWORD dwMilliseconds)
{
	DWORD			dwNumOfTransfer = 0,dwKey = 0,dwRetVal = 0;
	LPOVERLAPPED	lpOverlapped	= NULL;
	PSOCKETKEY		pSockKey		= NULL;
	int				nType			= 0;
	int				nLocalRecvLen	= 0,nRemoteRecvLen = 0,nRecvLen = 0;
	PACCEPTEX		pAcceptEx		= NULL;
	PACCEPTEXINFO	pAcceptInfo		= NULL;
	PCONNECTEX		pConnectEx		= NULL;
	PRECV			pRecv			= NULL;
	PSEND			pSend			= NULL;
	LPRECVFROM		pRecvFrom		= NULL;
	PSENDTO			pSendTo			= NULL;
	BlockX			*tgBlockX		= NULL;
	BOOL bSuc = GetQueuedCompletionStatus(m_hIocp,&dwNumOfTransfer,&dwKey,&lpOverlapped,dwMilliseconds);
	if(!bSuc)
	{
		dwRetVal = GetLastError();
		pSockKey = CONTAINING_RECORD(lpOverlapped,SOCKETKEY,tgOverlap);
		if( dwRetVal != ERROR_MORE_DATA )
		{
			if(!IsBadReadPtr(pSockKey,sizeof(SOCKETKEY)))
			{
				OnFailedFreeMemory(pSockKey->lpValue,nOperateType);
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
			}
			return dwRetVal;
		}
	}
	pSockKey = CONTAINING_RECORD(lpOverlapped,SOCKETKEY,tgOverlap);
	if(pSockKey == NULL || IsBadReadPtr(pSockKey,sizeof(SOCKETKEY)) || IsBadWritePtr(pSockKey,sizeof(SOCKETKEY)))
	{
		HeapFree(m_hHeap,HEAP_ZERO_MEMORY,lpOverlapped);
		return -1;
	}
	nOperateType = (UINT)pSockKey->nOperateType;
	dwID		 = pSockKey->dwID;
	try
	{
		switch(nOperateType)
		{
		case SOCKET_ACCEPTEX:
			{
				pAcceptEx	= (PACCEPTEX)pSockKey->lpValue;
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
				dwRetVal = OnAcceptEx(m_hHeap,pAcceptEx,(PACCEPTEXINFO*)lpOptVal,dwNumOfTransfer);
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pAcceptEx->lpDataBuf);
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pAcceptEx);
				return dwRetVal;
			}
		case SOCKET_CONNECTEX:
			pConnectEx = (PCONNECTEX)pSockKey->lpValue;
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
			if(dwNumOfTransfer)
			{
				OnConnectEx(m_hHeap,pConnectEx,dwNumOfTransfer);
				*lpOptVal = pConnectEx;
			}
			else
			{
				pConnectEx->dwBytesTranslate?dwRetVal = WSAEHOSTDOWN:dwRetVal =  0;			//连接已断开
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pConnectEx);
				return	dwRetVal;
			}
			break;
		case SOCKET_RECV:
			pRecv = (PRECV)pSockKey->lpValue;
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
			pRecv->dwLen = dwNumOfTransfer;
			*lpOptVal	 = (PVOID)pRecv;
			break;
		case SOCKET_RECVFROM:
			pRecvFrom = (PRECVFROM)pSockKey->lpValue;
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
			pRecvFrom->dwLen = dwNumOfTransfer;
			*lpOptVal	= (PVOID)pRecvFrom;			
			break;
		case SOCKET_SEND:
			pSend = (PSEND)pSockKey->lpValue;
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
			pSend->dwNumBytes = dwNumOfTransfer;
			*lpOptVal = (PVOID)pSend;
			break;
		case SOCKET_SENDTO:
			pSendTo=(PSENDTO)pSockKey->lpValue;
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
			pSendTo->dwNumBytes = dwNumOfTransfer;
			*lpOptVal = (PVOID)pSendTo;
			break;
		case SOCKET_DISCONNECTEX:
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
			*lpOptVal = NULL;
			break;
		case UNKNOWN_USERDATA:
			tgBlockX = (BlockX*)pSockKey->lpValue;
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
			*lpOptVal = (PVOID)tgBlockX;
			break;
		case 100:	//表示资源释放标记
			dwKey = (DWORD)pSockKey->lpValue;
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSockKey);
			return dwKey;
		default:
			return -1;
		}
	}
	catch (DWORD dwCode)
	{
		return dwCode;
	}
	return 0;
}

template< typename BlockX>
DWORD	JadeSocketIocp< BlockX >::OnFailedFreeMemory(PVOID lpData,UINT nOperateType)
{
	PACCEPTEX	pAcceptEx	= NULL;
	PCONNECTEX	pConnectEx	= NULL;
	PRECV		pRecv		= NULL;
	PRECVFROM	pRecvFrom	= NULL;
	PSEND		pSend		= NULL;
	PSENDTO		pSendTo		= NULL;
	BlockX*		pBlockX		= NULL;
	switch(nOperateType)
	{
	case SOCKET_ACCEPTEX:
		pAcceptEx = static_cast<PACCEPTEX>(lpData);	
		if( pAcceptEx->lpDataBuf != NULL && !IsBadReadPtr(pAcceptEx->lpDataBuf,pAcceptEx->dwBufLen) )
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pAcceptEx->lpDataBuf);
		break;
	case SOCKET_CONNECTEX:
		pConnectEx = static_cast<PCONNECTEX>(lpData);
		break;
	case SOCKET_RECV:
		pRecv	=	static_cast<PRECV>(lpData);
		if( pRecv->pBuf != NULL && !IsBadReadPtr(pRecv->pBuf,pRecv->dwLen))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRecv->pBuf);	
		break;
	case SOCKET_RECVFROM:
		pRecvFrom = static_cast<PRECVFROM>(lpData);
		if( pRecvFrom->pBuf != NULL && !IsBadReadPtr(pRecvFrom->pBuf,pRecvFrom->dwLen))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRecvFrom->pBuf);
		break;
	case SOCKET_SEND:
		pSend = static_cast<PSEND>(lpData);
		break;
	case SOCKET_SENDTO:
		pSendTo = static_cast<PSENDTO>(lpData);
		break;
	case UNKNOWN_USERDATA:
		pBlockX	= static_cast<BlockX*>(lpData);
		break;
	case 100:
		break;
	default:
		return -1;	//未知格式
	}
	return HeapFree(m_hHeap,HEAP_ZERO_MEMORY,lpData)?0:GetLastError();

}

template< typename BlockX>
DWORD	JadeSocketIocp< BlockX >::OnAcceptEx(HANDLE hHeap,PACCEPTEX pAcceptEx,PACCEPTEXINFO *lpInfo,DWORD dwNumOfBytesTransfer)
{
	DWORD	dwRetVal = 0;
	try
	{
		*lpInfo = (PACCEPTEXINFO)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,sizeof(ACCEPTEXINFO));
		if(*lpInfo == NULL)
			throw (DWORD)-1;
		if(pAcceptEx->bInherit)
		{
			dwRetVal = setsockopt( pAcceptEx->wSocket,SOL_SOCKET,SO_UPDATE_ACCEPT_CONTEXT
				,(char *)&m_socket,sizeof(m_socket) );
			if(dwRetVal == SOCKET_ERROR )
				return WSAGetLastError();
		}
		ParserAcceptEx(hHeap,pAcceptEx,*lpInfo);
	}
	catch (DWORD dwCode)
	{
		if( *lpInfo && !IsBadReadPtr(*lpInfo,sizeof(ACCEPTEXINFO)))
		{
			if( (*lpInfo)->lpBuf && !IsBadReadPtr((*lpInfo)->lpBuf,(*lpInfo)->dwBufLen) )
				HeapFree(hHeap,0,(*lpInfo)->lpBuf);
			HeapFree(hHeap,0,*lpInfo);
		}
		return dwCode;
	}
	return 0;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::OnConnectEx(HANDLE hHeap,PCONNECTEX pConnectEx,DWORD dwNumOfBytesTransfer)
{
	DWORD	dwRetVal = 0;
	if(pConnectEx == NULL || IsBadReadPtr(pConnectEx,sizeof(CONNECTEX)))
		return -1;
	pConnectEx->dwBytesTranslate = dwNumOfBytesTransfer;
	return 0;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::FreeMemory(LPVOID lpOptVal,UINT nOperateType)
{
	DWORD dwRetVal = 0;
	PACCEPTEXINFO	pAcceptEx = NULL;
	PRECVFROM		pRcvFrm	  = NULL;
	PRECV			pRcv	  = NULL;
	switch(nOperateType)
	{
	case SOCKET_CONNECTEX:
		HeapFree(m_hHeap,HEAP_ZERO_MEMORY,lpOptVal);
		break;
	case SOCKET_ACCEPTEX:
		pAcceptEx = static_cast<PACCEPTEXINFO>(lpOptVal);
		if(pAcceptEx != NULL && !IsBadReadPtr(pAcceptEx,sizeof(ACCEPTEXINFO)))
		{
			if(pAcceptEx->lpBuf != NULL && !IsBadReadPtr(pAcceptEx->lpBuf,pAcceptEx->dwBufLen))
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pAcceptEx->lpBuf);	
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pAcceptEx);
			dwRetVal = 0;
		}
		else
			dwRetVal = -1;
		break;
	case SOCKET_RECV:
		pRcv = (PRECV)lpOptVal;
		if(pRcv != NULL && !IsBadReadPtr(pRcv,sizeof(PRECV)))
		{
			if(pRcv->pBuf != NULL && !IsBadReadPtr(pRcv->pBuf,pRcv->dwLen))
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRcv->pBuf);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,lpOptVal);
			dwRetVal = 0;
		}
		else
			dwRetVal = -1;
		break;
	case SOCKET_RECVFROM:
		pRcvFrm = (PRECVFROM)lpOptVal;
		if(pRcvFrm != NULL && !IsBadReadPtr(pRcvFrm,sizeof(RECVFROM)))
		{
			if(pRcvFrm->pBuf != NULL && !IsBadReadPtr(pRcvFrm->pBuf,sizeof(pRcvFrm->dwLen)))
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRcvFrm->pBuf);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,lpOptVal);	
			dwRetVal = 0;
		}
		else
			dwRetVal = -1;
		break;
	case SOCKET_SEND:
		{
			PSEND pSend = (PSEND)lpOptVal;
			if( pSend->pBuf )
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSend->pBuf);
		}
		HeapFree(m_hHeap,HEAP_ZERO_MEMORY,lpOptVal);
		break;
	case SOCKET_DISCONNECTEX:

		break;
	case SOCKET_SENDTO:
		{
			PSENDTO pSend = (PSENDTO)lpOptVal;
			if( pSend->pBuf )
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSend->pBuf);
		}
		HeapFree(m_hHeap,HEAP_ZERO_MEMORY,lpOptVal);	
		break;
	case UNKNOWN_USERDATA:
		HeapFree(m_hHeap,HEAP_ZERO_MEMORY,lpOptVal);
		break;
	default:
		return -1;
	}
	return dwRetVal;
}

template< typename BlockX >
void	JadeSocketIocp< BlockX >::ParserAcceptEx(HANDLE hHeap,PACCEPTEX lpData,PACCEPTEXINFO pInfo)
{

	SOCKADDR		*pLocalAddrPos	= NULL,*pRemoteAddrPos = NULL;
	int				nLocalRecvLen	= 0,nRemoteRecvLen = 0,nRecvLen = 0;
	if
	( 
		lpData == NULL || (IsBadReadPtr(lpData,sizeof(ACCEPTEX)))	 ||
		pInfo  == NULL || (IsBadReadPtr(pInfo,sizeof(ACCEPTEXINFO))) ||
		m_lpfnGetAcceptExSockaddrs == NULL || ( IsBadReadPtr(m_lpfnGetAcceptExSockaddrs,sizeof(LPFN_GETACCEPTEXSOCKADDRS)))
	)
		return ;
	pInfo->wSocket	=	lpData->wSocket;
	pInfo->dwBufLen	=	lpData->dwBufLen - ADDRRESS_LENGTH*2;
	m_lpfnGetAcceptExSockaddrs(lpData->lpDataBuf,pInfo->dwBufLen,ADDRRESS_LENGTH,ADDRRESS_LENGTH
								,&pLocalAddrPos,&nLocalRecvLen,&pRemoteAddrPos,&nRemoteRecvLen);
	memcpy(&pInfo->tgLocalAddr,pLocalAddrPos,sizeof(SOCKADDR));
	memcpy(&pInfo->tgRemoteAddr,pRemoteAddrPos,sizeof(SOCKADDR));
	pInfo->lpBuf	=	(char*)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,pInfo->dwBufLen);
	if(!pInfo->lpBuf)
		return ;
	memcpy(pInfo->lpBuf,lpData->lpDataBuf,pInfo->dwBufLen);
	return;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::ConnectEx(DWORD dwID,const SOCKADDR *cpName,PVOID pSendBuffer,DWORD dwSendDataLen,
											DWORD *dwIdentify)
{
	DWORD		dwRetVal = 0;
	PSOCKETKEY	pKey	 = NULL;
	BOOL		bSuc	 = FALSE;
	PCONNECTEX	pConnectEx = NULL;
	try
	{
		if( m_lpfnConnectEx == NULL || IsBadReadPtr(m_lpfnConnectEx,sizeof(LPFN_CONNECTEX)) )
			return -1;		//调用序列错误
		pKey = (PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!pKey)
			throw (DWORD)-1;	
		pKey->nOperateType = SOCKET_CONNECTEX;
		pKey->dwID		   = dwID;
		pConnectEx = (PCONNECTEX)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(CONNECTEX));	
		if(!pConnectEx)
			throw (DWORD)-1;
		pKey->lpValue = (PVOID)pConnectEx;
		bSuc = m_lpfnConnectEx(m_socket,cpName,sizeof(SOCKADDR),pSendBuffer,dwSendDataLen,&pConnectEx->dwBytesTranslate,&pKey->tgOverlap);
		dwRetVal = WSAGetLastError();
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = (DWORD)pKey;
		return (dwRetVal != ERROR_IO_PENDING)&&(dwRetVal != 0) ? throw dwRetVal : ERROR_IO_PENDING;
	}
	catch (DWORD dwCode)
	{
		if(pKey && !IsBadReadPtr(pKey,sizeof(SOCKETKEY)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey);	
		if(pKey->lpValue && !IsBadReadPtr(pKey->lpValue,sizeof(CONNECTEX)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey->lpValue);
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return dwCode;
	}
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::ConnectEx(DWORD dwID,const char* lpszHostAddress,UINT nHostPort,PVOID pSendBuffer,DWORD dwSendDataLen,
											DWORD *dwIdentify,UINT unAf)
{
	SOCKADDR_IN	sockadr = { 0 };
	PSOCKETKEY	pKey = NULL;
	BOOL		bSuc = FALSE;
	DWORD		dwRetVal = 0;
	PCONNECTEX	pConnectEx = NULL;
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
	if( m_lpfnConnectEx == NULL || IsBadReadPtr(m_lpfnConnectEx,sizeof(LPFN_CONNECTEX)) )
		return -1;		//调用序列错误
	try
	{
		pKey = (PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!pKey)
			throw (DWORD)-1;	
		pKey->nOperateType = SOCKET_CONNECTEX;
		pKey->dwID		   = dwID;	
		pConnectEx = (PCONNECTEX)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(CONNECTEX));	
		if(!pConnectEx)
			throw (DWORD)-1;
		pKey->lpValue = (PVOID)pConnectEx;
		bSuc = m_lpfnConnectEx(m_socket,(SOCKADDR*)&sockadr,sizeof(SOCKADDR),pSendBuffer,dwSendDataLen,&pConnectEx->dwBytesTranslate
							  ,&pKey->tgOverlap);
		dwRetVal = WSAGetLastError();
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = (DWORD)pKey;
		return (dwRetVal != ERROR_IO_PENDING)&&(dwRetVal != 0) ? throw dwRetVal : ERROR_IO_PENDING;
	}
	catch (DWORD dwCode)
	{
		if(pKey && !IsBadReadPtr(pKey,sizeof(SOCKETKEY)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey);
		if(pKey->lpValue && !IsBadReadPtr(pKey->lpValue,sizeof(CONNECTEX)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey->lpValue);
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return dwCode;
	}
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::Receive(DWORD dwID,DWORD dwBufLen,DWORD &dwFlags,DWORD *dwIdentify)
{
	DWORD		dwRetVal = 0,dwNumOfBytesRecvd = 0;
	WSABUF		tgBuf	=	{ 0 };
	PSOCKETKEY	pKey	=	NULL;
	PRECV		pRecv	=	NULL;
	if(dwFlags == MSG_PEEK)		return (DWORD)-1;
	try
	{
		pKey	=	(PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!pKey)
			throw (DWORD)-1;
		pRecv	=	(PRECV)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(RECV));
		if(!pRecv)
			throw (DWORD)-1;
		pRecv->pBuf = (char*)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,dwBufLen);
		if(!pRecv->pBuf)
			throw (DWORD)-1;
		pKey->nOperateType	=	SOCKET_RECV;
		pKey->lpValue		=	(void*)pRecv;
		pKey->dwID			=	dwID;
		tgBuf.len	=	dwBufLen;
		tgBuf.buf	=	(char*)pRecv->pBuf;
		dwRetVal = WSARecv(m_socket,&tgBuf,1,NULL,&dwFlags,&pKey->tgOverlap,NULL);
		dwRetVal = WSAGetLastError();
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))	
			*dwIdentify = (DWORD)pKey;
		return (dwRetVal != ERROR_IO_PENDING)&&(dwRetVal != 0) ? throw dwRetVal : ERROR_IO_PENDING;
	}
	catch (DWORD dwCode)
	{
		if(pKey && !IsBadReadPtr(pKey,sizeof(SOCKETKEY)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey);
		if(pRecv && !IsBadReadPtr(pKey,sizeof(RECV)))
		{
			if(pRecv->pBuf != NULL && !IsBadReadPtr(pRecv->pBuf,pRecv->dwLen))
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRecv->pBuf);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRecv);
		}
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return dwCode;
	}
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::RecvDisconnect(LPCTSTR lpBuf,DWORD dwBufLen)
{
	WSABUF	tgBuf = { 0 };
	DWORD	dwRetVal = 0;
	try
	{
		if( lpBuf == NULL || dwBufLen == 0 || IsBadReadPtr((LPVOID)lpBuf,dwBufLen) )
			throw (DWORD)-1;
		tgBuf.buf = (char*)lpBuf;
		tgBuf.len = dwBufLen;
		dwRetVal = WSARecvDisconnect(m_socket,&tgBuf);
	}
	catch (DWORD dwCode)
	{
		return dwCode;
	}
	return dwRetVal ? WSAGetLastError() : 0;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::PeekInputQueue(char* const pBuf,DWORD dwBufLen,DWORD &dwRecvLen,long lOvertime)
{
	DWORD		dwRetVal = 0,dwNumOfBytesRecvd = 0,dwFlags = MSG_PEEK;
	WSABUF		tgNetBuf = { 0 };
	fd_set		fdopt,fdEx;
	FD_ZERO(&fdopt);FD_ZERO(&fdEx);
	FD_SET(m_socket,&fdopt);FD_SET(m_socket,&fdEx);
	timeval	tval = { 0 };
	tval.tv_usec = lOvertime*10000 ;	//输入毫秒转换为微秒
	try
	{
		if(pBuf == NULL || IsBadWritePtr(pBuf,dwBufLen))
			throw (DWORD)-1;
		ZeroMemory(pBuf,dwBufLen);
		tgNetBuf.buf = (char*)pBuf;
		tgNetBuf.len = dwBufLen;
		dwRetVal = select(1,&fdopt,NULL,&fdEx,&tval);
		if(FD_ISSET(m_socket,&fdopt))
		{
			FD_CLR(m_socket,&fdopt);
			dwRetVal = WSARecv(m_socket,&tgNetBuf,1,&dwRecvLen,&dwFlags,NULL,NULL);
			if(dwRetVal == SOCKET_ERROR )
				throw	(DWORD)WSAGetLastError();
			return 0;
		}
		else	if(FD_ISSET(m_socket,&fdEx))
		{
			FD_CLR(m_socket,&fdEx);
			throw (DWORD)ERROR_OOBDATA;
		}
		switch(dwRetVal)
		{
		case 0 :
			throw (DWORD)WAIT_TIMEOUT;
		case SOCKET_ERROR:
			throw (DWORD)WSAGetLastError();
		}
	}
	catch(DWORD dwCode)
	{
		return dwCode;
	}
	catch(...)
	{

		return -1;
	}
	return dwRetVal;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::IsConnect(long lOvertime)
{
	DWORD		dwRetVal = 0,dwNumOfBytesRecvd = 0,dwFlags = MSG_PEEK;
	WSABUF		tgNetBuf = { 0 };
	TCHAR		chBuf = 0;
	fd_set		fdRecv,fdSend;
	FD_ZERO(&fdRecv);FD_ZERO(&fdSend);
	FD_SET(m_socket,&fdRecv);FD_SET(m_socket,&fdSend);
	timeval	tval = { 0 };
	tval.tv_sec = lOvertime;	//输入毫秒转换为微秒
	tgNetBuf.buf = &chBuf;
	tgNetBuf.len = sizeof(chBuf);
	try
	{
		dwRetVal = select(0,&fdRecv,&fdSend,NULL,&tval);
		if(dwRetVal == 0)	throw (DWORD)WAIT_TIMEOUT;
		if( FD_ISSET(m_socket,&fdSend) )
		{
			FD_CLR(m_socket,&fdSend);
			throw (DWORD)0;	//还处于连接状态		
		}
		else if( FD_ISSET(m_socket,&fdRecv) )
		{
			FD_CLR(m_socket,&fdRecv);
			dwRetVal = WSARecv(m_socket,&tgNetBuf,1,&dwNumOfBytesRecvd,&dwFlags,NULL,NULL);
			if(dwRetVal)	throw	(DWORD)WSAGetLastError();
			if(dwRetVal == 0 && dwNumOfBytesRecvd == 0)	throw (DWORD)-1;	//
		}
	}
	catch(DWORD dwCode)
	{

		return dwCode;
	}
	catch(...)
	{
		
		return -1;
	}
	return dwRetVal;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::Send(DWORD dwID,char* pBuf,DWORD dwBufLen,
									   DWORD dwPos,DWORD dwFlags,DWORD *dwIdentify)
{
	DWORD		dwRetVal = 0,dwNumOfBytesSend = 0;
	WSABUF		tgBuf	=	{ 0 };
	PSOCKETKEY	pKey	=	NULL;
	PSEND		pSend	=	NULL;
	if(pBuf == NULL || IsBadWritePtr(pBuf,dwBufLen))
		return (DWORD)-1;
	try
	{
		pKey	=	(PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!pKey)
			throw (DWORD)-1;
		pSend	=	(PSEND)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SEND));
		if(!pSend)
			throw (DWORD)-1;
		pSend->pBuf = (char*)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,dwBufLen);
		if( !pSend->pBuf )
			throw (DWORD)-1;
		pKey->nOperateType	=	SOCKET_SEND;
		pKey->lpValue		=	(void*)pSend;
		pKey->dwID			=	dwID;
		//偏移信息保存
		memcpy(pSend->pBuf,pBuf,dwBufLen);
		pSend->dwBufLen = dwBufLen;
		pSend->dwPos    = dwPos;
		//使用偏移量确保同一数据包第一次发送未完成时可以下次继续发送		
		tgBuf.len = dwBufLen-dwPos;
		tgBuf.buf = pBuf+dwPos;
		dwRetVal = WSASend(m_socket,&tgBuf,1,&dwNumOfBytesSend,dwFlags,&pKey->tgOverlap,NULL);
		dwRetVal = WSAGetLastError();
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = (DWORD)pKey;
		return (dwRetVal != ERROR_IO_PENDING)&&(dwRetVal != 0) ? throw dwRetVal : ERROR_IO_PENDING;
	}
	catch(DWORD dwCode)
	{
		if(pKey && !IsBadReadPtr(pKey,sizeof(SOCKETKEY)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey);
		if(pSend && !IsBadReadPtr(pSend,sizeof(SEND)))
		{
			if( pSend->pBuf )
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSend->pBuf);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSend);
		}
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return dwCode;
	}
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::ReceiveFrom(DWORD dwID,DWORD dwLength,DWORD &dwFlags,
											  const char* szAddress,int nPort,DWORD *dwIdentify)
{
	DWORD	dwRetVal = 0,dwNumBytes = 0;
	PSOCKETKEY	tgKey	=	NULL;
	WSABUF		tgBuf	=	{ 0 };
	PRECVFROM	pRecvFrom = NULL;
	int			nAddrLen = sizeof(SOCKADDR_IN);
	
	if(dwFlags == MSG_PEEK)	return (DWORD)-1;
	try
	{
		tgKey = (PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!tgKey)	throw (DWORD)-1;

		pRecvFrom = (PRECVFROM)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(RECVFROM));
		if(!pRecvFrom) throw (DWORD)-1;

		pRecvFrom->pBuf	= (char*)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,dwLength);
		if(!pRecvFrom->pBuf)	throw (DWORD)-1;

		nAddrLen  = sizeof(SOCKADDR_IN);
		GetSockName((LPSOCKADDR)(&pRecvFrom->tgAddress),&nAddrLen);
		pRecvFrom->tgAddress.sin_port		= nPort;
		pRecvFrom->tgAddress.sin_addr.s_addr= inet_addr(szAddress);
		tgBuf.buf = (char*)pRecvFrom->pBuf;
		tgBuf.len = dwLength;

		tgKey->nOperateType = SOCKET_RECVFROM;
		tgKey->lpValue		= (PVOID)pRecvFrom;
		tgKey->dwID			= dwID;
		dwRetVal =	WSARecvFrom(m_socket,&tgBuf,1,&dwNumBytes,&dwFlags,(PSOCKADDR)&pRecvFrom->tgAddress,&nAddrLen,
								&tgKey->tgOverlap,NULL);
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = (DWORD)tgKey;
		dwRetVal = WSAGetLastError();
		return (dwRetVal != ERROR_IO_PENDING)&&(dwRetVal != 0) ? throw dwRetVal : ERROR_IO_PENDING;
	}
	catch (DWORD dwCode)
	{
		if(tgKey && !IsBadReadPtr(tgKey,sizeof(SOCKETKEY)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,tgKey);
		if(pRecvFrom && !IsBadReadPtr(pRecvFrom,sizeof(RECVFROM)))
		{
			if(pRecvFrom->pBuf && !IsBadReadPtr(pRecvFrom->pBuf,pRecvFrom->dwLen))
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRecvFrom->pBuf);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRecvFrom);
		}
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return dwCode;
	}
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::ReceiveFrom(DWORD dwID,DWORD dwLength,DWORD &dwFlags,SOCKADDR *lpFrom,int &nFromLen,
											DWORD *dwIdentify)
{
	DWORD		dwRetVal = 0,dwNumBytes = 0;
	PSOCKETKEY	tgKey	=	NULL;
	WSABUF		tgBuf	=	{ 0 };
	PRECVFROM	pRecvFrom = NULL;
	if(dwFlags == MSG_PEEK)		return (DWORD)-1;
	try
	{
		tgKey		= (PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!tgKey)	throw (DWORD)-1;
		pRecvFrom	= (PRECVFROM)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(RECVFROM));
		if(!pRecvFrom) throw (DWORD)-1;
		pRecvFrom->pBuf	=	HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,dwLength);
		if(!pRecvFrom->pBuf)	throw (DWORD)-1;
		memcpy(&pRecvFrom->tgAddress,lpFrom,nFromLen);
	
		tgBuf.buf = (char*)pRecvFrom->pBuf;
		tgBuf.len = dwLength;

		tgKey->nOperateType = SOCKET_RECVFROM;
		tgKey->lpValue		= (PVOID)pRecvFrom;
		tgKey->dwID			= dwID;
		dwRetVal =	WSARecvFrom(m_socket,&tgBuf,1,&dwNumBytes,&dwFlags,(SOCKADDR*)&pRecvFrom->tgAddress,&nFromLen,&tgKey->tgOverlap,NULL);
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = (DWORD)tgKey;
		memcpy(lpFrom,&pRecvFrom->tgAddress,sizeof(pRecvFrom->tgAddress));
		dwRetVal = WSAGetLastError();
		return (dwRetVal != ERROR_IO_PENDING)&&(dwRetVal != 0) ? throw dwRetVal : ERROR_IO_PENDING;
	}
	catch (DWORD dwCode)
	{
		if(tgKey && !IsBadReadPtr(tgKey,sizeof(SOCKETKEY)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,tgKey);
		if(pRecvFrom &&!IsBadReadPtr(pRecvFrom,sizeof(RECVFROM)))
		{
			if(pRecvFrom->pBuf && !IsBadReadPtr(pRecvFrom->pBuf,pRecvFrom->dwLen))
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRecvFrom->pBuf);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pRecvFrom);
		}
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return dwCode;
	}
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::SendTo(DWORD dwID,char *pBuffer,DWORD dwLength,
										 DWORD dwPos,DWORD dwFlags,SOCKADDR *lpTo,
										 int nToLen,DWORD *dwIdentify)
{
	DWORD		dwRetVal = 0,dwNumBytes = 0;
	PSENDTO		pSendTo	= NULL;
	PSOCKETKEY	pKey	= NULL;
	WSABUF		tgBuf	= { 0 };
	if(lpTo == NULL || IsBadReadPtr(lpTo,nToLen))		return (DWORD)-1;
	try
	{
		pKey = (PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!pKey)	throw (DWORD)-1;
		pSendTo = (PSENDTO)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SENDTO));
		if(!pSendTo) throw (DWORD)-1;
		pSendTo->pBuf = (char*)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,dwLength);
		if(!pSendTo->pBuf) throw (DWORD)-1;
		//
		memcpy(&pSendTo->tgAddress,lpTo,nToLen);
		pSendTo->dwBufLen =dwLength;
		memcpy(pSendTo->pBuf,pBuffer,dwLength);
		pSendTo->dwPos = dwPos;
		//
		pKey->nOperateType = SOCKET_SENDTO;
		pKey->lpValue = (PVOID)pSendTo;
		pKey->dwID		=	dwID;

		tgBuf.buf = pBuffer+dwPos;
		tgBuf.len = dwLength-dwPos;
		dwRetVal = WSASendTo(m_socket,&tgBuf,1,&dwNumBytes,dwFlags,lpTo,nToLen,&pKey->tgOverlap,NULL);
		dwRetVal = WSAGetLastError();
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = (DWORD)pKey;
		return (dwRetVal != ERROR_IO_PENDING)&&(dwRetVal != 0) ? throw dwRetVal : ERROR_IO_PENDING;
	}
	catch (DWORD dwCode)
	{
		if(pSendTo && !IsBadReadPtr(pSendTo,sizeof(SENDTO)))
		{
			if( pSendTo->pBuf )
				HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSendTo->pBuf);
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pSendTo);
		}
		if(pKey && !IsBadReadPtr(pKey,sizeof(SOCKETKEY)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey);
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = 0;
		return dwCode;
	}
	return WSAGetLastError();
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::SendTo(DWORD dwID,char *pBuffer,DWORD dwLength,DWORD dwPos,DWORD dwFlags,
										 const char* lpAddress,int nPort,short nAf)
{
	SOCKADDR_IN	tgAddress = { 0 };
	if(lpAddress != NULL)
		tgAddress.sin_addr.s_addr = inet_addr(lpAddress);
	else
		tgAddress.sin_addr.s_addr = INADDR_ANY;
	tgAddress.sin_family = nAf;
	tgAddress.sin_port = htons(nPort);
	return SendTo(dwID,pBuffer,dwLength,dwPos,dwFlags,(SOCKADDR*)&tgAddress,sizeof(SOCKADDR_IN),NULL);
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::Disconnect(DWORD dwID,DWORD dwFlags,DWORD Reserved,DWORD *dwIdentify)
{
	DWORD		dwRetVal = 0;
	PSOCKETKEY	pKey	 = NULL;
	if(m_lpfnDisconnectEx == NULL || IsBadReadPtr(m_lpfnDisconnectEx,sizeof(LPFN_DISCONNECTEX)))
		return -1;
	try
	{
		pKey = (PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!pKey)
			throw (DWORD)-1;
		pKey->nOperateType = SOCKET_DISCONNECTEX;
		pKey->dwID		   = dwID;
		dwRetVal = m_lpfnDisconnectEx(m_socket,&pKey->tgOverlap,dwFlags,Reserved);
		dwRetVal = WSAGetLastError();
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = (DWORD)pKey;
		return (dwRetVal != ERROR_IO_PENDING)&&(dwRetVal != 0) ? throw dwRetVal : ERROR_IO_PENDING;
	}
	catch (DWORD dwCode)
	{
		if(pKey && !IsBadReadPtr(pKey,sizeof(SOCKETKEY)))
			HeapFree(m_hHeap,HEAP_ZERO_MEMORY,pKey);	
		if(!IsBadWritePtr(dwIdentify,sizeof(DWORD)))
			*dwIdentify = NULL;
		return dwCode;
	}
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::Attach(SOCKET tgSock)
{
	DWORD		dwRetVal = 0,dwBytes = 0;
	GUID		GuidAcceptEx			 = WSAID_ACCEPTEX;
	GUID		GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
	GUID		GuidConnectEx			 = WSAID_CONNECTEX;
	GUID		GuidDisconnectEx		 = WSAID_DISCONNECTEX;
	SYSTEM_INFO	tgSysInfo = { 0 };
	LPFN_CONNECTEX	lpfnConnectEx		 = NULL;
	LPFN_ACCEPTEX	lpfnAcceptEx		 = NULL;
	LPFN_GETACCEPTEXSOCKADDRS	lpfnGetAcceptExSockaddrs = NULL;
	LPFN_DISCONNECTEX	lpfnDisconnectEx = NULL;
	GetSystemInfo(&tgSysInfo);
	try
	{
		dwRetVal	=	::WSAIoctl(tgSock,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidAcceptEx,sizeof(GuidAcceptEx)
									,&lpfnAcceptEx,sizeof(lpfnAcceptEx),&dwBytes,NULL,NULL);
		if(dwRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		dwRetVal	=	::WSAIoctl(tgSock,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidGetAcceptExSockaddrs
									,sizeof(GuidGetAcceptExSockaddrs),&lpfnGetAcceptExSockaddrs
									,sizeof(lpfnGetAcceptExSockaddrs),&dwBytes,NULL,NULL);
		if(dwRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		dwRetVal =	::WSAIoctl(tgSock,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidConnectEx,sizeof(GuidConnectEx)
								,&lpfnConnectEx,sizeof(lpfnConnectEx),&dwBytes,NULL,NULL);
		if(dwRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		dwRetVal = ::WSAIoctl(tgSock,SIO_GET_EXTENSION_FUNCTION_POINTER,&GuidDisconnectEx,sizeof(GuidDisconnectEx)
								,&lpfnDisconnectEx,sizeof(lpfnDisconnectEx),&dwBytes,NULL,NULL);
		if(dwRetVal == SOCKET_ERROR)
			throw (DWORD)WSAGetLastError();
		if(m_hIocp)
			CloseHandle(m_hIocp);
		m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,m_dwKey,tgSysInfo.dwNumberOfProcessors*2+2);
		if(!m_hIocp)
			throw (DWORD)GetLastError();
		CreateIoCompletionPort((HANDLE)tgSock,m_hIocp,m_dwKey,tgSysInfo.dwNumberOfProcessors*2+2);
		if(m_hHeap)
			HeapDestroy(m_hHeap);
		m_hHeap = HeapCreate(0,tgSysInfo.dwPageSize,0);
		if(m_socket)
		{
			m_lpfnDisconnectEx(m_socket,NULL,0,0);
			closesocket(m_socket);
			m_socket = tgSock;
		}
		else
			m_socket = tgSock;
		m_lpfnConnectEx				= lpfnConnectEx;
		m_lpfnAcceptEx				= lpfnAcceptEx;
		m_lpfnGetAcceptExSockaddrs	= lpfnGetAcceptExSockaddrs;
		m_lpfnDisconnectEx			= lpfnDisconnectEx;
	}
	catch(DWORD dwCode)
	{	
		return dwCode;
	}
	return dwRetVal;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::Release(DWORD dwExitCode)
{
	PSOCKETKEY	pKey = NULL;
	DWORD		dwRetVal = 0;
	try
	{
		pKey = (PSOCKETKEY)HeapAlloc(m_hHeap,HEAP_ZERO_MEMORY,sizeof(SOCKETKEY));
		if(!pKey)	throw (DWORD)-1;
		pKey->nOperateType = 100;
		pKey->lpValue = (PVOID)dwExitCode;
		PostQueuedCompletionStatus(m_hIocp,sizeof(SOCKETKEY),m_dwKey,&pKey->tgOverlap);
		CloseHandle(m_hIocp);
		m_hIocp = 0;
		closesocket(m_socket);
		HeapDestroy(m_hHeap);
	}
	catch(DWORD dwCode)
	{
		return dwCode;
	}
	return dwRetVal;
}
/************************************************************************\
					函数名称：IsComplete
					函数功能：判断操作是否完成
					函数参数：
						Input:
							DWORD		dwIdentify	操作标识
						Output：
						
					返 回 值：
						0
						-1
			注意如果操作返回值为非ERROR_IO_PENDING请不要使用该函数
\************************************************************************/
template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::IsComplete(DWORD dwIdentify)
{
	PSOCKETKEY		ptgKey		= reinterpret_cast<PSOCKETKEY>((PVOID)dwIdentify);
	if(IsBadReadPtr(ptgKey,sizeof(SOCKETKEY)))
		return -1;
	return HasOverlappedIoCompleted(&ptgKey->tgOverlap) ? 0 : -1;
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::CancelPending(DWORD dwIdentify)
{
	PROCESS_HEAP_ENTRY	tgEntry = { 0 };
	DWORD dwRetVal = 0;
	try
	{
		if(IsComplete(dwIdentify))
			return -1;
		if(!HeapLock(m_hHeap))
			throw GetLastError(); 
		if(!HeapWalk(m_hHeap,&tgEntry))
			throw GetLastError();
		while(HeapWalk(m_hHeap,&tgEntry))
		{
			if((DWORD)tgEntry.lpData == dwIdentify)
			{
				HeapUnlock(m_hHeap);
				HeapFree(m_hHeap,0,tgEntry.lpData);
				throw (DWORD)0;
			}
		}
		HeapUnlock(m_hHeap);
		dwRetVal = GetLastError();
	}
	catch(DWORD dwCode)
	{
		if(dwCode)
			HeapUnlock(m_hHeap);
		return dwCode;
	}
	return (dwRetVal == ERROR_NO_MORE_ITEMS && dwRetVal == 0)?0:GetLastError();
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::CancelAll(void)
{
	PROCESS_HEAP_ENTRY	tgEntry = { 0 };
	DWORD dwRetVal = 0;
	try
	{
		if(CancelIo(m_socket))
		{
			if(HeapLock(m_hHeap))
				throw GetLastError(); 
			if(HeapWalk(m_hHeap,&tgEntry))
				throw GetLastError();
			while(HeapWalk(m_hHeap,&tgEntry))
				HeapFree(m_hHeap,0,tgEntry.lpData);	
			HeapUnlock(m_hHeap);
			dwRetVal = GetLastError();
		}
	}
	catch(DWORD dwCode)
	{
		HeapUnlock(m_hHeap);
		return dwCode;
	}
	return (dwRetVal == ERROR_NO_MORE_ITEMS && dwRetVal == 0)?0:GetLastError();
}

template< typename BlockX >
DWORD	JadeSocketIocp< BlockX >::ReleaseAll(void)
{
	DWORD	dwRetVal = 0;
	try
	{
		if(!CancelIo((HANDLE)m_socket))	throw (DWORD)GetLastError();
		if(m_hHeap)
		{
			HeapDestroy(m_hHeap);
			m_hHeap = NULL;
		}
		CloseHandle(m_hIocp);
		m_hIocp = NULL;
		closesocket(m_socket);
		m_socket = NULL;
		if(m_bAutoClean)
			WSACleanup();
	}
	catch(DWORD	dwCode)
	{
		return dwCode;
	}
	return dwRetVal;
}