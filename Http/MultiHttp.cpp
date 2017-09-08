#include "stdafx.h"
#include <Winsock2.h>
#include "MultiHttp.h"
#include "HttpClient.h"

#define CMD_CANCEL "cancel curl"
#define CMD_USER_FLAG "&&"
int sockpair(SOCKET *fds,int nNum,int nStart)
{
	int nBindPort = nStart;
	if( nNum < 2 )
		return nBindPort;
	int	 nWsaError=0;
	struct sockaddr_in inaddr;
	memset(&inaddr, 0, sizeof(inaddr));

	fds[0] =::socket(AF_INET, SOCK_DGRAM,0);
	fds[1] =::socket(AF_INET, SOCK_DGRAM,0);

	//int yes=1;
	//::setsockopt(fds[0],SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(yes));
	//::setsockopt(fds[1],SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(yes));
	//找到一个可绑定的命令端口
	int nbind=0;
	do 
	{
		inaddr.sin_family = AF_INET;
		inaddr.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
		inaddr.sin_port = htons(nBindPort++);
		nbind = ::bind(fds[1],(struct sockaddr *)&inaddr,sizeof(inaddr));
	}while(nbind==SOCKET_ERROR);
	return nBindPort;
}
bool sendcmd(SOCKET s,char *ch,int nlen,int nCmdPort)
{
	struct sockaddr_in inaddr;
	inaddr.sin_family = AF_INET;
	inaddr.sin_addr.s_addr = ::htonl(INADDR_LOOPBACK);
	inaddr.sin_port = htons(nCmdPort);
	int nHaveSend=0;
	do 
	{
		int nTmp = nHaveSend;
		nHaveSend += ::sendto(s,ch+nHaveSend,nlen-nHaveSend,0,(sockaddr*)&inaddr,sizeof(inaddr));
		if(nHaveSend==SOCKET_ERROR)
		{
			int wE = WSAGetLastError();
			return false;
		}
		if(nHaveSend==0)
		{
			return false;
		}
		if(nTmp==nHaveSend)
		{
			return false;
		}
	}while(nHaveSend!=nlen);
	return true;
}
bool recvcmd(SOCKET s,std::string &cmd)
{
	struct sockaddr addr;
	int addlen = sizeof(sockaddr_in);
	char order[1024];
	memset(order,0,sizeof(order));
	int nCode = ::recvfrom(s,order,sizeof(order),0,&addr,&addlen);
	if( nCode==0 || nCode==SOCKET_ERROR )
	{
		int nCode = WSAGetLastError();
		return false;
	}
	cmd.append(order,nCode);
	return true;
}
DWORD WINAPI ThreadProc(void *param)
{
	::CoInitialize(NULL);
	curl::CMultiHttp *pService = static_cast<curl::CMultiHttp*>(param);
	while( pService->Run() )
	{

	}
	::CoUninitialize();
	return 0;
}
namespace curl
{
	CMultiHttp::CMultiHttp()
	{
		m_cmdPort = CMultiHttp::CmdPortStart;
		m_state = CMultiHttp::Initialize;
		m_cmdPort = sockpair(m_fds,2,CMultiHttp::CmdPortStart);
		m_murl = curl_multi_init();
		m_hThread = ::CreateThread(NULL,0,ThreadProc,this,CREATE_SUSPENDED,NULL);
	}
	CMultiHttp::~CMultiHttp()
	{
		if( m_hThread )
		{
			::CloseHandle(m_hThread);
			m_hThread = NULL;
		}
		//释放curl资源
		if( m_murl )
		{
			for(size_t nI=0;nI < m_lstUrl.size();nI++)
			{
				curl_multi_remove_handle(m_murl,m_lstUrl[nI]);
				curl_multi_cleanup(m_lstUrl[nI]);
			}
			m_lstUrl.clear();
			curl_multi_cleanup(m_murl);
			m_murl = NULL;
		}
		closesocket(m_fds[0]);
		closesocket(m_fds[1]);
	}
	void CMultiHttp::AddUrl(curl::CHttpClient* client)
	{
		if( client==NULL )
			return ;
		curl_easy_setopt(client->GetCURL(),CURLOPT_NOSIGNAL,1);
		if( CURLM_OK==curl_multi_add_handle(m_murl,client->GetCURL()) )
			m_lstUrl.push_back(client->GetCURL());
	}
	bool CMultiHttp::Start()
	{
		if( m_state==CMultiHttp::Initialize )
		{
			::ResumeThread(m_hThread);
			return true;
		}
		if( m_state==CMultiHttp::Running )
			Cancel();
		if( m_state==CMultiHttp::Terminate )
		{
			if( m_hThread )
				::CloseHandle(m_hThread);
			m_hThread = ::CreateThread(NULL,0,ThreadProc,this,CREATE_SUSPENDED,NULL);
			::ResumeThread(m_hThread);
			 m_state=CMultiHttp::Initialize;
		}
		return false;
	}
	void CMultiHttp::PostCmd(const char *c,UserCmd *cmd)
	{
		if( m_murl && m_fds[1] && m_state==CMultiHttp::Running )
		{
			std::string userCmd(c);
			char chNo[20] = {0};
			if(cmd)
			{
				m_lock.Lock();
				m_cmds.push_back(cmd);
				m_lock.UnLock();
				size_t nSZ = m_cmds.size();
				sprintf_s(chNo,100,"%d",nSZ);
				userCmd += CMD_USER_FLAG;
				userCmd += chNo;
			}
			sendcmd(m_fds[0],(char*)userCmd.c_str(),userCmd.size(),m_cmdPort);
		}
	}
	bool CMultiHttp::Cancel()
	{
		bool ncode=0;
		DWORD dwCode=3;
		if( m_hThread )
		{
			::GetExitCodeThread(m_hThread,&dwCode);
			if( dwCode==STILL_ACTIVE )
			{
				m_hThread=NULL;
				return true;
			}
		}
		else
			return true;
		if( m_murl && m_fds[1] && m_state==CMultiHttp::Running )
		{
			//重试三次
			dwCode=3;
			do 
			{
				if(!sendcmd(m_fds[0],CMD_CANCEL,sizeof(CMD_CANCEL),m_cmdPort))
				{
					dwCode--;
					::Sleep(100);
				}
				else
					break;
			}while( dwCode );
			dwCode=0;
			while( ::GetExitCodeThread(m_hThread,&dwCode) )
			{
				if( dwCode!=STILL_ACTIVE )
					break;
				::Sleep(100);
			}
			m_hThread=NULL;
			m_state = CMultiHttp::Terminate;
			return true;
		}
		else if( m_state==CMultiHttp::Terminate )
			return true;
		return false;
	}
	bool CMultiHttp::Run()
	{
		if( m_murl==NULL )
			return false;
		int still_running=0;
		if( m_state==CMultiHttp::Initialize )
		{
			m_state=CMultiHttp::Running;
			while( CURLM_CALL_MULTI_PERFORM==curl_multi_perform(m_murl,&still_running) )
				;
		}
		return Perform();
	}
	bool	CMultiHttp::Perform()
	{
		struct timeval timeout;
		int still_running=0;
		int rc=0;			/* select() return code */ 
		CURLMcode mc;		/* curl_multi_fdset() return code */ 
		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd = -1;
		long curl_timeo = -1;
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);
		/* set a suitable timeout to play around with */ 
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		curl_multi_timeout(m_murl, &curl_timeo);
		if( curl_timeo==0 )
			curl_timeo=50;
		if(curl_timeo >= 0)
		{
			timeout.tv_sec = curl_timeo/1000;
			if(timeout.tv_sec > 1)
				timeout.tv_sec = 1;
			else
				timeout.tv_usec = (curl_timeo % 1000) * 1000;
		}
		/* get file descriptors from the transfers */ 
		mc = curl_multi_fdset(m_murl, &fdread, &fdwrite, &fdexcep, &maxfd);
		if(mc != CURLM_OK)
		{
			return false;
		}
		/* On success the value of maxfd is guaranteed to be >= -1. We call
         select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
         no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
         to sleep 100ms, which is the minimum suggested value in the
         curl_multi_fdset() doc. */
		if(maxfd == -1)
		{
			Sleep(100);
			rc = 0;
		}
		else
		{
			FD_SET(m_fds[1],&fdread);
			FD_SET(m_fds[1],&fdwrite);
			FD_SET(m_fds[1],&fdexcep);
			rc = ::select(maxfd+2, &fdread,&fdwrite, &fdexcep, &timeout);
			if(rc>0 && FD_ISSET(m_fds[1],&fdread))
			{
				std::string cmd;
				if( !recvcmd(m_fds[1],cmd) )
				{
					m_state = CMultiHttp::Terminate;
					return false;
				}
				if( cmd==CMD_CANCEL )
				{
					m_state = CMultiHttp::Terminate;
					return false;
				}
				else
				{
					std::string userPost(cmd);
					UserCmd *userCmd=NULL;
					size_t nFlag = cmd.find_last_of(CMD_USER_FLAG);
					int    cmdIndex=-1;
					if(nFlag!=std::string::npos)
					{
						std::string index = userPost.substr(nFlag+1);
						cmdIndex = atol(index.c_str());
						userPost = userPost.substr(0,nFlag-1);						
						m_lock.Lock();
						if( m_cmds.size() && m_cmds.size()>cmdIndex )
							userCmd = m_cmds.at(cmdIndex);
						m_lock.UnLock();
					}
					OnUserCmd(userPost.c_str(),userCmd);
					if(userCmd)
					{
						m_lock.Lock();
						std::vector<UserCmd*>::iterator it = std::find(m_cmds.begin(),m_cmds.end(),userCmd);
						if(it!=m_cmds.end())
							m_cmds.erase(it);
						m_lock.UnLock();
					}
				}
			}
		}
		switch(rc)
		{
		case -1:
			/* select error */ 
			break;
		case 0:
		default:
			while( CURLM_CALL_MULTI_PERFORM==curl_multi_perform(m_murl,&still_running) )
				;
			{
				struct CURLMsg *m;
				int msgQ=0;
				m = curl_multi_info_read(m_murl,&msgQ);
				if(m && (m->msg == CURLMSG_DONE))
				{
					CURL *e = m->easy_handle;
					if( !OneComplete(e,m->data.result) )
						still_running=0;
				}
			}
			break;
		}
		if( still_running )
			return true;
		m_state = CMultiHttp::Terminate;
		return false;
	}
	bool CMultiHttp::OneComplete(CURL* url,CURLcode code)
	{

		return true;
	}
	void CMultiHttp::OnUserCmd(const char *,UserCmd *cmd)
	{

	}
}
