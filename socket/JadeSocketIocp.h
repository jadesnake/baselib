#if !defined(__JADE_SOCKET_IOCP_H_)
#define __JADE_SOCKET_IOCP_H_

#include <Winsock2.h>
#include <mswsock.h>
#include <Ws2tcpip.h>
#if !defined(BASE_CODE)
	#if ( _WIN32_WINNT >= 0x0500 )
		#define BASE_CODE	(16000L)	//对于Windows2000以后系统函数返回信息至16000之后开始
	#else
		#define BASE_CODE	(6010L)		//对于Windows2000之前系统函数返回信息至6010之后开始
	#endif
#endif

#define ERROR_OOBDATA	(BASE_CODE+100)	//

#define	ADDRRESS_LENGTH	(sizeof(SOCKADDR_IN)+16)
/************************************************************************\
					函数名称：SOCKETKEY
					函数功能：标示IO操作Key关联值
					函数参数：
						Input:
							DWORD	dwOperateType	操作类型
							LPVOID	lpValue			类型结构指针
						Output：
						
					返 回 值：
						无
\************************************************************************/
typedef struct _tgSocketKey
{
	OVERLAPPED	tgOverlap;
	INT			nOperateType;
	DWORD		dwID;			//包标签
	LPVOID		lpValue;		//
}SOCKETKEY,*PSOCKETKEY;
/************************************************************************\
					結構名稱：AcceptEx
					結構參數：
							LPVOID		lpDataBuf		接收数据缓冲区
							DWORD		dwBufLen		接收数据缓冲区
\************************************************************************/
typedef struct _tgAcceptEx
{
	LPVOID		lpDataBuf;
	DWORD		dwBufLen;
	SOCKET		wSocket;
	BOOL		bInherit;
}ACCEPTEX,*PACCEPTEX;
/************************************************************************\
					結構名稱：AcceptExInfo
					結構參數：
						SOCKADDR	tgRemoteAddr	远程地址
						SOCKADDR	tgLocalAddr		本地地址
\************************************************************************/
typedef struct _tgAcceptExInfo
{
	SOCKADDR_IN	tgRemoteAddr;
	SOCKADDR_IN	tgLocalAddr;
	char*		lpBuf;
	DWORD		dwBufLen;
	SOCKET		wSocket;
}ACCEPTEXINFO,*PACCEPTEXINFO;
/************************************************************************\
					結構名稱：ConnectEx
					結構參數：
						DWORD	dwBytesTranslate 已经发送的数据长度							
\************************************************************************/
typedef struct _tgConnectEx
{
	DWORD	dwBytesTranslate;
}CONNECTEX,*PCONNECTEX,FAR *LPCONNECTEX;
/************************************************************************\
					結構名稱：Recv
					結構參數：
						PVOID	pBuf	实际接收数据缓冲区
						DWORD	dwLen	实际接收数据长度
\************************************************************************/
typedef struct _tgRecv
{
	char	*pBuf;
	DWORD	dwLen;
}RECV,*PRECV,FAR *LPRECV;
/************************************************************************\
					結構名稱：Send
					結構參數：
						DWORD	dwNumBytes	实际发送数据长度
\************************************************************************/
typedef struct _tgSend
{
	DWORD	dwNumBytes;		//已发送字节数
	char	*pBuf;			//发送缓冲区
	DWORD	dwBufLen;		//发送缓冲区长度
	DWORD	dwPos;			//当前游标位置
}SEND,*PSEND,FAR *LPSEND;
/************************************************************************\
					結構名稱：RecvFrom
					結構參數：
						PVOID			pBuf		接收网络数据缓冲区
						DWORD			dwLen		接收数据长度
						SOCKADDR_IN		tgAddress	数据来源
\************************************************************************/
typedef struct _tgRecvFrom
{
	char*			pBuf;
	DWORD			dwLen;
	SOCKADDR_IN		tgAddress;
}RECVFROM,*PRECVFROM,FAR *LPRECVFROM;
/************************************************************************\
					結構名稱：SendTo
					結構參數：
						DWORD	dwNumBytes	实际发送数据长度
\************************************************************************/
typedef struct _tgSendTo
{
	SOCKADDR_IN	tgAddress;
	DWORD		dwNumBytes;
	char		*pBuf;			//发送缓冲区
	DWORD		dwBufLen;		//发送缓冲区长度
	DWORD		dwPos;			//当前游标位置
}SENDTO,*PSENDTO,FAR *LPSENDTO;

template < typename BlockX = int >
class JadeSocketIocp
{
public:
	JadeSocketIocp(void);
	JadeSocketIocp(UINT unlow,UINT unHigh,BOOL bAutoClean);
	virtual ~JadeSocketIocp(void);
	/************************************************************************\
								操作类型定义			
	\************************************************************************/
	static	const UINT	SOCKET_ACCEPTEX		= 0;
	static	const UINT	SOCKET_SENDTO		= 1;
	static	const UINT	SOCKET_RECVFROM		= 2;
	static	const UINT	SOCKET_SEND			= 3;
	static	const UINT	SOCKET_RECV			= 4;
	static	const UINT	SOCKET_CONNECTEX	= 5;
	static	const UINT	SOCKET_DISCONNECTEX	= 6;
	static	const UINT	UNKNOWN_USERDATA	= 7;
	DWORD	m_dwKey;
	SOCKET	m_socket;					//Socket
public:
	int		Startup(WORD wVersion,WSADATA* lpWsaData,BOOL bAutoClean = TRUE);
	
	DWORD	InitResource(void);

	DWORD	GetSockName(char* rSocketAddress,UINT& rSocketPort,UINT &unAdrSize);
	
	DWORD	GetSockName(SOCKADDR* lpSockAddr,int* lpSockAddrLen );

	DWORD	GetSockOpt(int nOptionName,void* lpOptionValue,int* lpOptionLen,int nLevel = SOL_SOCKET);
	
	DWORD	SetSockOpt(int nOptionName,const void* lpOptionValue,int nOptionLen,int nLevel = SOL_SOCKET);

	DWORD	Create(UINT nSocketPort = 0,const char* lpszSocketAddress = NULL,int nSocketType = SOCK_STREAM,
					DWORD	dwFlags = WSA_FLAG_OVERLAPPED,LPWSAPROTOCOL_INFO lpProtocolInfo = NULL,
					BOOL bBind = TRUE,UINT unAf = AF_INET,UINT unProtocol = IPPROTO_IP);

	DWORD	CreateEx(SOCKADDR_IN *pSockAddr,DWORD dwSockAddrLen,LPWSAPROTOCOL_INFO lpProtocolInfo
					,int nSocketType = SOCK_STREAM,DWORD dwFlags = WSA_FLAG_OVERLAPPED
					,BOOL bBind = TRUE,UINT unAf = AF_INET,UINT unProtocol = IPPROTO_IP);

	DWORD	Listen(int nBacklog = SOMAXCONN);
	
	DWORD	AcceptEx(IN DWORD dwID,IN SOCKET ConnectedSocket,IN DWORD dwDataBufLen,IN BOOL bInherit = TRUE,OUT DWORD *dwIdentify = NULL);

	DWORD	ConnectEx(DWORD dwID,const SOCKADDR *cpName,PVOID pSendBuffer,DWORD dwSendDataLen,DWORD *dwIdentify);

	DWORD	ConnectEx(DWORD dwID,const char* lpszHostAddress,UINT nHostPort,PVOID pSendBuffer,DWORD dwSendDataLen,
					DWORD *dwIdentify = NULL,UINT unAf = AF_INET);

	DWORD	Receive(DWORD dwID,DWORD dwBufLen,DWORD &dwFlags,DWORD *dwIdentify);

	DWORD	PeekInputQueue(char* const pBuf,DWORD dwBufLen,DWORD &dwRecvLen,long lOvertime);
	
	DWORD	IsConnect(long lOvertime);

	DWORD	Send(DWORD dwID,char* pBuf,DWORD dwBufLen,DWORD dwPos,DWORD dwFlags,DWORD *dwIdentify);

	DWORD	ReceiveFrom(DWORD dwID,DWORD dwLength,DWORD &dwFlags,const char* szAddress,int nPort,DWORD *dwIdentify);
	DWORD	ReceiveFrom(DWORD dwID,DWORD dwLength,DWORD &dwFlags,SOCKADDR *lpFrom,int &nFromLen,DWORD *dwIdentify);

	DWORD	SendTo(DWORD dwID,char *pBuffer,DWORD dwLength,DWORD dwPos,DWORD dwFlags,
				   SOCKADDR *lpTo,int nToLen,DWORD *dwIdentify);

	DWORD	SendTo(DWORD dwID,char *pBuffer,DWORD dwLength,DWORD dwPos,DWORD dwFlags,
				   const char* lpAddress,int nPort,short nAf);

	DWORD	Disconnect(DWORD dwID,DWORD dwFlags,DWORD Reserved,DWORD *dwIdentify);
	
	DWORD	WaitSocketComplete(DWORD &dwID,UINT &nOperateType,PVOID *lpOptVal,DWORD dwMilliseconds);

	void	ParserAcceptEx(HANDLE hHeap,PACCEPTEX lpData,PACCEPTEXINFO pInfo);

	char *	Ipv4AddrToString(SOCKADDR_IN *SockAddr);
	
	DWORD	Attach(SOCKET tgSock);

	DWORD	FreeMemory(PVOID lpData,UINT nOperateType);

	DWORD	Release(DWORD dwExitCode);

	DWORD	Associate(HANDLE hIocp,DWORD dwKey);

	DWORD	PostUserDefined(DWORD dwID,const BlockX &pUserX,DWORD *dwIdentify);

	DWORD	IsComplete(DWORD dwIdentify);
	
	DWORD	RecvDisconnect(LPCTSTR lpBuf,DWORD dwBufLen);

	DWORD	CancelAll(void);

	DWORD	CancelPending(DWORD dwIdentify);

	DWORD	ReleaseAll(void);
protected:
	DWORD	Associate(HANDLE hIocp,SOCKET nSock);
	DWORD	OnFailedFreeMemory(PVOID lpData,UINT nOperateType);
protected:
	DWORD	OnAcceptEx(HANDLE hHeap,PACCEPTEX pAcceptEx,PACCEPTEXINFO *lpInfo,DWORD dwNumOfBytesTransfer);
	DWORD	OnConnectEx(HANDLE hHeap,PCONNECTEX pConnectEx,DWORD dwNumOfBytesTransfer);
private:
	LPFN_ACCEPTEX				m_lpfnAcceptEx;				//AcceptEx Function Pointer
	LPFN_GETACCEPTEXSOCKADDRS	m_lpfnGetAcceptExSockaddrs;	//GetAcceptExSockaddrs Function Pointer
	LPFN_CONNECTEX				m_lpfnConnectEx;			//ConnectEx Function Pointer
	LPFN_DISCONNECTEX			m_lpfnDisconnectEx;			//DisconnectEx Function Pointer
	HANDLE						m_hIocp;					//Socket io complete Handle
	BOOL						m_bAutoClean;				//Auto Clean flag
	HANDLE						m_hHeap;					//Memory Heap	
};

#include "JadeSocketIocp.inl"

#endif /* __JADE_SOCKET_IOCP_H_ */