#pragma once

#include <map>
#include <sstream>

//#define CURL_STATICLIB
#include "curl/curl.h"
#pragma comment(lib,"Wldap32.lib")
#pragma comment(lib,"Ws2_32.lib")


namespace curl
{
	class CDebug
	{
	public:
		virtual ~CDebug(){	}
		virtual void OnCurlDbgRelease() = 0;
		virtual void OnCurlDbgTrace(const std::stringstream& ss) = 0;
	};
	class CNotify
	{
	public:
		virtual ~CNotify(){	}
		virtual void OnCurlNotifyRelease() = 0;
		//总长度，接受的数据长度
		virtual void OnProgress(__int64 total,__int64 rcvSize) = 0;
		virtual void OnComplete(bool bSuc,const char* msg) = 0;
	};
	class CHttpClient
	{
	public:
		typedef struct 
		{
			typedef enum
			{
				HTTP = 0,		/* added in 7.10, new in 7.19.4 default is to use CONNECT HTTP/1.1 */
				SOCKS5 = 5,	/* added in 7.10 */
				NONE
			}TYPE;
			CAtlString strName;
			CAtlString strPass;
			TYPE	   nType;
			CAtlString strServer;
			long	   nPort;
		}Proxy;
		typedef enum
		{
			ParamNormal,
			ParamFile,
			ParamFileData
		}ParamAttr;
		CHttpClient();
		virtual ~CHttpClient();
		//设置agent
		void	SetAgent(const CAtlString &val);
		//添加参数
		void	AddBoundary(const CAtlString &szName,const CAtlString &szValue,ParamAttr dwParamAttr=ParamNormal);
		void	AddBoundary(const std::string& ,const std::string& szValue, ParamAttr dwParamAttr=ParamNormal);

		void	AddFile(const CAtlString  &szName,const CAtlString  &szFileName,const CAtlString &szValue);
		void	AddFile(const std::string &szName,const std::string &szFileName,const std::string& szValue);
		//清除参数
		void	ClearBoundary();
		//自定义协议头
		void	AddHeader(const CAtlString &szName,const CAtlString &szValue);
		void	AddHeader(const std::string &szName,const std::string &szValue);
		//自定义参数
		void	AddParam(const CAtlString &szName,const CAtlString &szValue);
		void	AddParam(const std::string &szName,const std::string &szValue);
		//处理参数
		void	PerformParam(const CAtlString& url);
		void	PerformParam(const std::string& url);
		//获取curl句柄
		CURL*	GetCURL();
		//设置代理信息
		void	SetProxy(const Proxy &tgProxy);
		//超时单位毫秒
		void	SetTimeout(long out);
		//提交的cotent
		void	SetContent(const std::string& data);
		const   std::stringstream& getContent() const;
		void	EnableWriteHeader(bool b);
		//设置数据存储
		void	BodySaveFile(FILE *f);
		void	SetCookie(const std::string &val);
		const std::string& GetCookie();
		//
		std::string	RequestPost(const CAtlString& url,bool cHeader=true,bool cParam=true,bool perform=true);
		std::string RequestPost(const std::string& url,bool cHeader=true,bool cParam=true,bool perform=true);

		std::string RequestGet(const CAtlString& url,bool cHeader=true,bool cParam=true,bool perform=true);
		std::string RequestGet(const std::string& url,bool cHeader=true,bool cParam=true,bool perform=true);
		void EnableFollowLocation(bool b);
		bool IsResponseChunk();

		std::string GetStream();
		std::string GetRpHeader(const char* key);

		std::vector<std::string> GetChunks();
		std::string MakeChunks();
		//
		void		EnableDecode(bool bDecode);
		static void GlobalSetup();
		static void GlobalClean();
		static long PerformUrl(CURL* url);
		std::string EncodeUrl(const std::string &v);
		std::string DecodeUrl(const std::string &v);
		void		SetEncodeUrl(bool e);
		long		ReqeustCode();
		void		ClearAll();
		void		SetNotify(CNotify *notify);
		void		SetDebug(CDebug *dbg);
		CDebug*		GetDebug();
		void HandleCookie();
		void HandleHeader();
		//内部调用逻辑
		enum Proc{
			SaveFile,
			SaveStream,
			Upload,
			SaveHeader,
			Progress,
		};
		size_t InsideProc(char *ptr, size_t size, size_t nmemb,Proc proc);
		size_t InsideProgress(__int64 dltotal,__int64 dlnow,__int64 ultotal, __int64 ulnow);
		std::string m_rqUrl;
		FILE		*m_Save2File;
	protected:
		std::string encodeParam();
	protected:
		typedef std::multimap<std::string, std::string>	mapStrings;
		//应答头部信息
		std::map<std::string,std::string> m_rpHeaders;
		long		m_tmOut;
		CAtlString	m_agent;
		Proxy		m_tgProxy;
		mapStrings	m_header;
		mapStrings  m_params;
		bool		m_bEncodeUrl;
		bool		m_bFollowLocation;

		std::stringstream	m_wbuf;
		std::stringstream	m_rbuf;
		std::stringstream	m_headbuf;

		CDebug *m_dbg;
		CNotify	*m_notify;
		bool bHttps;
		CURLcode pfmCode;	//performcode
	private:
		CURL		*m_url;
		struct curl_slist	 *m_headerlist;
		struct curl_httppost *m_postBoundary;
		struct curl_httppost *m_lastBoundary;
		bool		m_bWriteHeader;
		bool		m_bDecodeBody;
		std::string m_cookie;
	};
}