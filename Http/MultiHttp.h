#pragma once

#include <map>
#include <sstream>
#include <algorithm>
#include "HttpClient.h"
#include "curl/curl.h"
#include "../AsyncUtils/LockGuard.h"
namespace curl
{
	class CMultiHttp
	{
	public:
		typedef enum
		{
			Initialize,		//初始
			Running,		//运行
			Terminate		//结束
		}STATUS;
		class UserCmd
		{
		public:
			virtual ~UserCmd(){	}
			virtual void Release() = 0;
		};
		const static int CmdPortStart = 8181;
		CMultiHttp();
		virtual ~CMultiHttp();
		void	AddUrl(curl::CHttpClient* client);
		virtual bool	Cancel();
		virtual bool	Start();
		virtual bool	Run();
		virtual void    PostCmd(const char *,UserCmd *cmd);
	protected:
		virtual bool	OneComplete(CURL* url,CURLcode code);
		virtual void    OnUserCmd(const char *,UserCmd *cmd);
		virtual bool	Perform();
	protected:
		CLock		m_lock;
		std::vector<UserCmd*>	m_cmds;

		STATUS		m_state;		//运行状态
		SOCKET		m_fds[2];		//0-c,1-s
		HANDLE		m_hThread;		//
		std::vector<CURL*>	m_lstUrl;
		CURLM		*m_murl;		//mulit curl
		int			m_cmdPort;
	};
}