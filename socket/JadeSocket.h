#ifndef JADE_SOCKET_H
#define JADE_SOCKET_H

// This class is exported from the LsRoof_EmailOS.dll
#define ERROR_SOCK_TIMEOUT	0x0000412b	//表示超时
#define ERROR_SOCK			0x0000589c	//表示socket操作错误
#define ERROR_PROTO_NETERR	0x01009020	//非网络协议
#define ERROR_SOCK_DROP		0x0000A00A	//连接被断开

class JadeSocket
{
public:
	JadeSocket(UINT unlow,UINT unHigh,BOOL bAutoClean);
	JadeSocket();
	virtual ~JadeSocket();
public:
	DWORD Startup(WORD wVersion,WSADATA* lpWsaData,BOOL bAutoClean = TRUE);
	
	DWORD Create(UINT nSocketPort = 0,const char* lpszSocketAddress = NULL,int nSocketType = SOCK_STREAM,
				 BOOL bBind = TRUE,UINT unAf = AF_INET,UINT unProtocol = IPPROTO_IP);

	DWORD CreateEx(SOCKADDR_IN *pSockAddr,DWORD dwSockAddrLen,int nSocketType = SOCK_STREAM,BOOL bBind = TRUE
				   ,UINT unAf = AF_INET,UINT unProtocol = IPPROTO_IP);

	DWORD GetSockName(char* rSocketAddress,UINT& rSocketPort,UINT &unAdrSize);
	DWORD GetSockName(SOCKADDR* lpSockAddr,int* lpSockAddrLen );
	DWORD GetSockOpt(int nOptionName,void* lpOptionValue,int* lpOptionLen,int nLevel = SOL_SOCKET);
	DWORD SetSockOpt(int nOptionName,const void* lpOptionValue,int nOptionLen,int nLevel = SOL_SOCKET);
	
	DWORD Accept(SOCKET &ConnectedSocket,SOCKADDR* lpSockAddr = NULL,int* lpSockAddrLen = NULL);
	
	DWORD AcceptByTimeout(SOCKET &ConnectedSocket,UINT unTimeout,int nRestart,SOCKADDR* lpSockAddr = NULL,
						  int* lpSockAddrLen = NULL);
	
	DWORD Bind(UINT nSocketPort,const char* lpszSocketAddress = NULL,UINT unAf = AF_INET);
	DWORD Bind(const SOCKADDR* lpSockAddr,int nSockAddrLen );
	DWORD Close();
	DWORD Connect(const char* lpszHostAddress,UINT nHostPort,UINT unTimeOut,UINT unAf = AF_INET);
	DWORD Connect(const SOCKADDR* lpSockAddr,int nSockAddrLen,UINT unTimeOut);
	DWORD IOCtl(long lCommand,DWORD* lpArgument );
	DWORD Listen(int nConnectionBacklog = 5 );
	DWORD Receive(void* lpBuf,int nBufLen,int &nRevLen,long unTimeOut,int nFlags = 0 );
	DWORD ReceiveFrom(void* lpBuf,int nBufLen,int &nRevLen,SOCKADDR* lpSockAddr, int* lpSockAddrLen,UINT unTimeOut,int nFlags = 0 );
	DWORD Send( const void* lpBuf, int nBufLen,int &nSedLen,UINT unTimeOut,int nFlags = 0);
	DWORD SendTo( const void* lpBuf, int nBufLen,int &nSedLen,UINT unTimeOut,UINT nHostPort,const char* lpszHostAddress = NULL,int nFlags = 0,UINT unAf = AF_INET);
	DWORD SendTo( const void* lpBuf, int nBufLen,int &nSedLen,UINT unTimeOut, const SOCKADDR* lpSockAddr, int nSockAddrLen, int nFlags = 0 );
	DWORD ShutDown( int nHow);
	
	DWORD GetPeerName(char* rPeerAddress,UINT& rPeerPort,UINT unAdrSize);
	DWORD GetPeerName(SOCKADDR* lpSockAddr,int* lpSockAddrLen);

	DWORD ReadLine(std::string &tgLine,UINT &unBufLen,long lOvertime);

	DWORD Clear();
	
	DWORD FlushIORecv(void);
public:
	char *	Ipv4AddrToString(SOCKADDR_IN *SockAddr);
	inline void	SetSocket(SOCKET sock){ m_socket = sock; }
	
	inline SOCKET GetSocket(void)
	{
		return m_socket;
	}

	inline	DWORD Detach(SOCKET &lpSock)
	{
		if(m_socket == NULL)
			return -1;
		lpSock = m_socket;
		m_socket = NULL;
		return 0;
	}
	
	inline	DWORD Attach(SOCKET tgSock)
	{
		m_socket = tgSock;
		return 0;
	}

	inline DWORD IsHasData(void)
	{
		char chBuf = 0;
		DWORD dwRetVal =	recv(m_socket,&chBuf,1,MSG_PEEK);
		return dwRetVal != 0?-1:0;
	}
	DWORD	IsConnected(long lusec);
protected:
	enum WaitOpt{WAITREAD=0,WAITWRITE,WAITCONNECT,};
	DWORD EnteryWait(UINT unOpt,long lSec,long luSec = 0);
	DWORD LeaveWait();
	DWORD ConnectComplete(UINT unTimeOut);
	
public:
	SOCKET	m_socket;
	DWORD	m_Event;
	BOOL	m_bAutoClean;
	bool	m_bSorM;	//S --- 秒,M --- 毫秒 true 表示秒，false 表示毫秒
};

#endif/*JADE_SOCKET_H*/
