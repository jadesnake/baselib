#include "stdafx.h"
#include "HttpClient.h"
#include <algorithm>
namespace curl {
	static int dbg_trace(CURL *handle, curl_infotype type,char *data, size_t size,void *userp)
	{
		std::stringstream buf;
		switch (type) {
		case CURLINFO_TEXT:
			buf << data;
		default: /* in case a new one is introduced to shock us */
			return 0;
		case CURLINFO_HEADER_OUT:
			buf <<std::endl<<"=> Send header";
			break;
		case CURLINFO_DATA_OUT:
			buf <<std::endl<< "=> Send data";
			break;
		case CURLINFO_SSL_DATA_OUT:
			buf <<std::endl<< "=> Send SSL data";
			break;
		case CURLINFO_HEADER_IN:
			buf <<std::endl<< "<= Recv header";
			break;
		case CURLINFO_DATA_IN:
			buf <<std::endl<< "<= Recv data";
			break;
		case CURLINFO_SSL_DATA_IN:
			buf <<std::endl<< "<= Recv SSL data";
			break;
		}
		buf.write((char *)data, size);
		buf.flush();
		buf << "\r\n";
		::OutputDebugStringA(buf.str().c_str());
		CDebug *dbg = static_cast<CDebug*>(userp);	
		if(dbg)
			dbg->OnCurlDbgTrace(buf);
		return 0;
	}
	int StreamSeek(void *userp, curl_off_t offset, int origin) 
	{
		//if( offset==1 && origin== SEEK_SET)
		//	return CURL_SEEKFUNC_CANTSEEK;
		//std::stringstream *buf = static_cast<std::stringstream*>(userp);
		//buf->seekg(offset);
		return CURL_SEEKFUNC_CANTSEEK;
	}
	size_t StreamHeader(char *ptr, size_t size, size_t nmemb, void *userdata) 
	{
		CHttpClient::Proc proc = CHttpClient::SaveHeader;
		CHttpClient *client = static_cast<CHttpClient*>(userdata);
		client->InsideProc((char*)ptr,size,nmemb,proc);
		return (size*nmemb);
	}
	int StreamProgress(void *clientp,curl_off_t dltotal,curl_off_t dlnow,curl_off_t ultotal,curl_off_t ulnow)
	{
		CHttpClient *client = static_cast<CHttpClient*>(clientp);
		client->InsideProgress(dltotal,dlnow,ultotal,ulnow);
		return 0;
	}
	size_t StreamSave(char *ptr, size_t size, size_t nmemb, void *userdata)
	{
		CHttpClient::Proc proc = CHttpClient::SaveStream;
		CHttpClient *client = static_cast<CHttpClient*>(userdata);
		client->InsideProc((char*)ptr,size,nmemb,proc);
		return (size*nmemb);
	}
	size_t StreamSaveFile(char *ptr, size_t size, size_t nmemb, void *userdata)
	{
		CHttpClient::Proc proc = CHttpClient::SaveFile;
		CHttpClient *client = static_cast<CHttpClient*>(userdata);
		client->InsideProc((char*)ptr,size,nmemb,proc);
		return (size*nmemb);
	}
	size_t StreamUpdate(void *ptr, size_t size, size_t nmemb, void *userdata)
	{
		CHttpClient::Proc proc = CHttpClient::Upload;
		CHttpClient *client = static_cast<CHttpClient*>(userdata);
		return client->InsideProc((char*)ptr,size,nmemb,proc);
	}
	//有些服务chuck长度和实际数据不一致
	void readChuckFromStream(std::stringstream &ss,std::string &out,long nlen)
	{
		if(nlen==0)	return ;
		long nowPos = ss.tellg();
		char *buf = new char[nlen];
		ss.read(buf,nlen);
		nowPos = ss.tellg() - nowPos;
		out.clear();
		out.append(buf,nowPos);
		delete []buf;
	}
	//有些服务chuck长度和实际数据不一致
	void readChuckFromStream(std::stringstream &ss,std::string &out)
	{
		char crlf[2]={0,0};
		char chTmp=0;
		std::string chuck;
		while(ss.read(&chTmp,1))
		{
			if(chTmp=='\r' && ss.peek()=='\n')
			{
				//跳过末尾
				ss.seekg(1,std::ios::cur);
				break ;
			}
			chuck += chTmp;
		}
		out = chuck;
		return ;
	}

	Chunk::Chunk()
		:data(std::stringstream::in|std::stringstream::out|std::stringstream::binary)
	{
		databyte = 0;
		savebyte = 0;
		bDataEnd = false;
		bSizeEnd = false;
		offset = 0;
		databyte = 0;
 	}
	bool Chunk::IsSizeEnd()
	{
		return bSizeEnd;
	}
	bool Chunk::IsDataEnd()
	{
		return bDataEnd;
	}
	void Chunk::clear()
	{
		databyte = 0;
		savebyte = 0;
		bDataEnd = false;
		bSizeEnd = false;
		offset = 0;
		inSize.clear();
		data.str("");
		data.clear();
		data.seekp(0);
		data.seekg(0);	 
	}
	void Chunk::write(char *p,long szmem)
	{
		offset = 0;
		if(!bSizeEnd)
		{
			for(long nI=0;nI<szmem;)
			{
				inSize += *(p+nI);
				nI += 1;
				size_t szlen = inSize.size();
				char chN = inSize[szlen-1];
				if((szlen-1)==0)
					continue;
				char chR = inSize[szlen-2];
				if(chN=='\n'&&chR=='\r')
				{
					bSizeEnd = true;
					offset = nI;
					//移除末尾两个字符
					inSize.erase(inSize.size()-1);
					inSize.erase(inSize.size()-1);
					sscanf(inSize.c_str(),"%x",&databyte);
					break;
				}
			}
		}
		if(bSizeEnd)
		{
			if(databyte==0)
			{
				bDataEnd = true;
 				return ;
			}
			long syBytes = szmem-offset;
			long wantsave= databyte-savebyte;
			if((syBytes-wantsave)<0)
			{
				savebyte += syBytes;
				data.write(p+offset,syBytes);
				offset = 0;
				return ;
			}
			savebyte += wantsave;
			data.write(p+offset,wantsave);
			offset = wantsave+2; //跳过结束标记
			bDataEnd = true;
		}		
		return ;
	}
	size_t CHttpClient::InsideProgress(__int64 dltotal,__int64 dlnow,__int64 ultotal, __int64 ulnow)
	{
		if(m_notify)
		{
			if(m_rbuf.str().size() && ulnow)
			{
				m_notify->OnProgress(ultotal,ulnow);
			}
			else if( dlnow )
			{
				m_notify->OnProgress(dltotal,dlnow);
			}
		}
		return 1;
	}
	size_t CHttpClient::InsideProc(char *ptr, size_t size, size_t nmemb,Proc proc)
	{
		if(proc==SaveHeader&&size)
		{
			m_headbuf.write(ptr, size*nmemb);
			m_headbuf.flush();
			return m_headbuf.tellp();
		}
		if((proc==SaveStream||proc==SaveFile)&&size&&m_contentType.empty())
		{
			HandleHeader();
			m_contentType = GetRpHeader("content-type");
			m_chunked = IsResponseChunk();
			//转小写
			std::transform(m_contentType.begin(), m_contentType.end(), m_contentType.begin(), ::tolower);
		}
		if(proc==SaveStream&&size)
		{
			m_wbuf.write(ptr,size*nmemb);
			m_wbuf.flush();
			return m_wbuf.tellp();
		}
		if(proc==SaveFile&&size&&m_Save2File)
		{
			//app/json数据
			if(-1!=m_contentType.find("application/json"))
			{
				m_wbuf.write(ptr,size*nmemb);
				m_wbuf.flush();
			}
			if(m_chunked)
			{
				m_nowChunk.write(ptr,size*nmemb);
				if(m_nowChunk.IsSizeEnd()&&m_nowChunk.inSize=="0")
				{
					return (size*nmemb);
				}
				if(m_nowChunk.IsDataEnd())
				{
 					size_t tp = m_nowChunk.data.tellp();
					long offset = m_nowChunk.offset;
					fwrite(m_nowChunk.data.str().c_str(),1, tp, m_Save2File);
					m_nowChunk.clear();
					m_nowChunk.write(ptr+offset,size*nmemb-offset);
				}
				return (size*nmemb);
			}
			return fwrite(ptr, size, nmemb, m_Save2File);
		}
		if(proc==Upload&&size)
		{
			std::string test = m_rbuf.str();
			m_rbuf.read((char*)ptr,size*nmemb);
			return m_rbuf.gcount();
		}
		return 0;
	}
	void CHttpClient::GlobalSetup() 
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}
	void CHttpClient::GlobalClean()
	{
		curl_global_cleanup();
	}
	long CHttpClient::PerformUrl(CURL* url)
	{
		CURLcode ret = curl_easy_perform(url);
		if (ret != CURLE_OK) {
			return ret;
		}
		long retcode = 0;
		ret = curl_easy_getinfo(url, CURLINFO_RESPONSE_CODE,&retcode);
		if (ret != CURLE_OK) {
			return ret;
		}
		return retcode;
	}
	CHttpClient::CHttpClient()
		:m_url(NULL)
		,m_headerlist(NULL)
		,m_postBoundary(NULL)
		,m_lastBoundary(NULL)
		,m_bWriteHeader(true)
		,m_Save2File(NULL)
		,m_bDecodeBody(false)
		,m_bEncodeUrl(true)
		,m_dbg(NULL)
		,m_notify(NULL)
		,m_bFollowLocation(true)
	{
		bHttps = false;
		m_tmOut = 30;
		m_tgProxy.nType = Proxy::NONE;
		m_url   = curl_easy_init();
		m_chunked = false;
	}
	void CHttpClient::SetNotify(CNotify *notify)
	{
		if(m_notify)
			return ;
		m_notify = notify;
	}
	void CHttpClient::SetDebug(CDebug *dbg)
	{
		if(m_dbg)
			return ;
		m_dbg = dbg;
	}
	CDebug*	CHttpClient::GetDebug()
	{
		return m_dbg;
	}
	void CHttpClient::SetContent(const std::string& data) 
	{
		m_rbuf << data;
		m_rbuf.flush();
	}
	const std::stringstream& CHttpClient::getContent() const {
		return m_rbuf;
	}
	CHttpClient::~CHttpClient()
	{
		ClearAll();
		if(m_dbg)
		{
			m_dbg->OnCurlDbgRelease();
			m_dbg = NULL;
		}
		if(m_notify)
		{
			m_notify->OnCurlNotifyRelease();
			m_notify = NULL;
		}
	}
	void	CHttpClient::ClearAll()
	{
		if( m_url )
		{
			curl_easy_cleanup(m_url);
			m_url = NULL;
		}
		if( m_headerlist )
		{
			curl_slist_free_all(m_headerlist);
			m_headerlist=NULL;
		}
		ClearBoundary();
		m_header.clear();
		m_params.clear();
	}
	void	CHttpClient::SetTimeout(long out)
	{
		m_tmOut = out;
	}
	void	CHttpClient::SetAgent(const CAtlString &val)
	{
		m_agent = val;
	}
	
	CURL*	CHttpClient::GetCURL()
	{
		return m_url;
	}
	void	CHttpClient::SetProxy(const Proxy &tgProxy)
	{
		m_tgProxy = tgProxy;
	}
	void	CHttpClient::ClearBoundary()
	{
		if( m_postBoundary)
		{
			curl_formfree(m_postBoundary);
			m_postBoundary = NULL;
		}
	}
	void	CHttpClient::EnableWriteHeader(bool b) 
	{
		m_bWriteHeader = b;
	}
	void	CHttpClient::EnableFollowLocation(bool b)
	{
		m_bFollowLocation = b;
	}
	void	CHttpClient::AddBoundary(const std::string& szName, const std::string& szValue, ParamAttr dwParamAttr)
	{
		CURLFORMcode code = CURL_FORMADD_OK;
		if (dwParamAttr == ParamNormal)
		{
			code = curl_formadd(&m_postBoundary, &m_lastBoundary,
				CURLFORM_COPYNAME, szName.c_str(),
				CURLFORM_COPYCONTENTS, szValue.c_str(),
				CURLFORM_END);
		}
		else if (dwParamAttr == ParamFile)
		{
			code = curl_formadd(&m_postBoundary, &m_lastBoundary,
				CURLFORM_COPYNAME, szName.c_str(),
				CURLFORM_FILE, szValue.c_str(),
				CURLFORM_FILENAME, szName.c_str(),
				CURLFORM_END);
		}
		else if (dwParamAttr == ParamFileData) {
			code = curl_formadd(&m_postBoundary, &m_lastBoundary,
				CURLFORM_COPYNAME, "file",
				CURLFORM_FILENAME, szName.c_str(),
				CURLFORM_COPYCONTENTS, szValue.c_str(),
				CURLFORM_CONTENTSLENGTH,szValue.size(),
				CURLFORM_END);
		}
	}
	void	CHttpClient::AddBoundary(const CAtlString &szName,const CAtlString &szValue,ParamAttr dwParamAttr)
	{
		if( dwParamAttr==ParamNormal )
		{
			curl_formadd(&m_postBoundary,&m_lastBoundary,
						 CURLFORM_COPYNAME,(char*)CT2CA(szName,CP_UTF8),
						 CURLFORM_COPYCONTENTS,(char*)CT2CA(szValue),
						 CURLFORM_END);
		}
		else if( dwParamAttr==ParamFile )
		{
			std::string file;
			std::string name = (char*)CT2CA(szName, CP_UTF8);
			file = (char*)CT2CA(szValue, CP_UTF8);
			curl_formadd(&m_postBoundary, &m_lastBoundary,
				CURLFORM_COPYNAME, name.c_str(),
				CURLFORM_FILE, file.c_str(),
				CURLFORM_FILENAME, name.c_str(),
				CURLFORM_END);
		}
		else if (dwParamAttr == ParamFileData) {
			std::string val = (char*)CT2CA(szValue, CP_UTF8);
			std::string name = (char*)CT2CA(szName, CP_UTF8);
			curl_formadd(&m_postBoundary, &m_lastBoundary,
				CURLFORM_COPYNAME, "file",
				CURLFORM_FILENAME, name.c_str(),
				CURLFORM_COPYCONTENTS, val.c_str(),
				CURLFORM_END);
		}
	}
	void	CHttpClient::AddHeader(const CAtlString &szName, const CAtlString &szValue) {
		AddHeader((const char*)CT2CA(szName), (const char*)CT2CA(szValue));
	}
	void	CHttpClient::AddHeader(const std::string &szName, const std::string &szValue) {
		m_header.insert(std::make_pair(szName, szValue));
	}

	void	CHttpClient::AddParam(const CAtlString &szName, const CAtlString &szValue)
	{
		AddParam((const char*)CT2CA(szName),(const char*)CT2CA(szValue));
	}
	void	CHttpClient::AddParam(const std::string &szName, const std::string &szValue)
	{
		m_params.insert(std::make_pair(szName, szValue));
	}
	void	CHttpClient::AddFile(const CAtlString  &szName,const CAtlString  &szFileName,const CAtlString &szValue)
	{
		AddFile((char*)CT2CA(szName,CP_UTF8),(char*)CT2CA(szFileName,CP_UTF8),(char*)CT2CA(szValue));
	}
	void	CHttpClient::AddFile(const std::string &szName,const std::string &szFileName,const std::string& szValue)
	{
		CURLFORMcode code = CURL_FORMADD_OK;
		code = curl_formadd(&m_postBoundary, &m_lastBoundary,
			CURLFORM_COPYNAME, szName.c_str(),
			CURLFORM_FILE, szValue.c_str(),
			CURLFORM_FILENAME, szFileName.c_str(),
			CURLFORM_END);
	}
	void	CHttpClient::SetEncodeUrl(bool e)
	{
		m_bEncodeUrl = e;
	}
	void	CHttpClient::BodySaveFile(FILE *f) 
	{
		m_Save2File = f;
	}
	void	CHttpClient::PerformParam(const std::string& url)
	{
		m_chunked = false;
		if(m_rqUrl!=url && m_url)
			curl_easy_reset(m_url);
		m_wbuf.clear();	//清除操作标记，如 输入输出流指针
		m_wbuf.str("");
		m_wbuf.flush();

		m_rbuf.clear();	//清除操作标记，如 输入输出流指针

		m_headbuf.clear();
		m_headbuf.str("");
		{	//处理协议头
			if (m_headerlist)
			{
				curl_slist_free_all(m_headerlist);
				m_headerlist = NULL;
			}
			mapStrings::iterator itPos = m_header.begin();
			for (itPos; itPos != m_header.end(); itPos++)
			{
				std::string strVal;
				if (!itPos->first.empty() && !itPos->second.empty()) {
					strVal = itPos->first + ":" + itPos->second;
					m_headerlist = curl_slist_append(m_headerlist,strVal.c_str());
				}				
			}
		}
		{
			//char *ret = curl_easy_escape(m_url,strTmp,strTmp.GetLength());
			//strTmp = ret;
			//curl_free(ret);
			curl_easy_setopt(m_url, CURLOPT_URL, url.c_str());
		}
		if( std::string::npos!=url.find("https") )
		{
			bHttps = true;
			curl_easy_setopt(m_url,CURLOPT_SSL_VERIFYPEER,false);
			curl_easy_setopt(m_url,CURLOPT_SSL_VERIFYHOST,false);
		}
		else
		{
			bHttps = false;
		}
		curl_easy_setopt(m_url, CURLOPT_CONNECTTIMEOUT, 30);		//链接超时
		if (m_headerlist)
			curl_easy_setopt(m_url, CURLOPT_HTTPHEADER, m_headerlist);
		if(!m_agent.IsEmpty())
			curl_easy_setopt(m_url, CURLOPT_USERAGENT, (char*)CT2CA(m_agent, CP_UTF8));
		
		curl_easy_setopt(m_url, CURLOPT_TIMEOUT, m_tmOut);
		//curl_easy_setopt(m_url, CURLOPT_TIMEOUT, 0);	//永不超时

		if(m_bFollowLocation)
			curl_easy_setopt(m_url, CURLOPT_FOLLOWLOCATION, 1L);
		else
			curl_easy_setopt(m_url, CURLOPT_FOLLOWLOCATION, 0L);
		 
		curl_easy_setopt(m_url, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(m_url, CURLOPT_FORBID_REUSE, 1L);

		if (!m_cookie.empty()) {
			curl_easy_setopt(m_url, CURLOPT_COOKIE,m_cookie.c_str());
		}
		if (m_bDecodeBody) {
			curl_easy_setopt(m_url, CURLOPT_HTTP_CONTENT_DECODING,1L);
			curl_easy_setopt(m_url, CURLOPT_HTTP_TRANSFER_DECODING,1L);
		}
		else {
			curl_easy_setopt(m_url, CURLOPT_HTTP_CONTENT_DECODING, 0L);
			curl_easy_setopt(m_url, CURLOPT_HTTP_TRANSFER_DECODING,0L);
		}
		//curl_easy_setopt(m_url, CURLOPT_COOKIESESSION, 1L);
		//curl_easy_setopt(m_url, CURLOPT_COOKIEFILE, "curl.cookie");
		//curl_easy_setopt(m_url, CURLOPT_ACCEPT_ENCODING, "zh-CN,zh;q=0.8");
		//keep alive
		curl_easy_setopt(m_url, CURLOPT_TCP_KEEPALIVE, 1L);
		curl_easy_setopt(m_url, CURLOPT_TCP_KEEPIDLE, 120L);
		curl_easy_setopt(m_url, CURLOPT_TCP_KEEPINTVL, 60L);
		if (m_Save2File)
		{
			curl_easy_setopt(m_url, CURLOPT_WRITEFUNCTION, StreamSaveFile);
			curl_easy_setopt(m_url, CURLOPT_WRITEDATA, (void*)this);
		}
		else
		{
			curl_easy_setopt(m_url, CURLOPT_WRITEFUNCTION, StreamSave);
			curl_easy_setopt(m_url, CURLOPT_WRITEDATA, (void*)this);
		}
		if (m_notify)
		{
			curl_easy_setopt(m_url,CURLOPT_XFERINFODATA,(void*)this);
			curl_easy_setopt(m_url,CURLOPT_XFERINFOFUNCTION,StreamProgress);
			curl_easy_setopt(m_url,CURLOPT_NOPROGRESS, 0L);
		}
		if (m_rbuf.tellp()) {
			curl_off_t nLen = (curl_off_t)m_rbuf.tellp();
			curl_easy_setopt(m_url, CURLOPT_READFUNCTION, StreamUpdate);
			curl_easy_setopt(m_url, CURLOPT_READDATA, (void*)this);
		}
		if (m_tgProxy.nType != Proxy::NONE)
		{
			bool bValidPoxy=false;
			if(m_tgProxy.nType==Proxy::HTTP)
			{
				curl_easy_setopt(m_url, CURLOPT_PROXYTYPE,CURLPROXY_HTTP);
				bValidPoxy=true;
			}
			else if(m_tgProxy.nType==Proxy::SOCKS5)
			{
				curl_easy_setopt(m_url, CURLOPT_PROXYTYPE,CURLPROXY_SOCKS5);
				bValidPoxy=true;
			}
			if(bValidPoxy)
			{
				if( std::string::npos!=url.find("https") )
					curl_easy_setopt(m_url, CURLOPT_HTTPPROXYTUNNEL, 1L);
				curl_easy_setopt(m_url, CURLOPT_PROXYPORT, m_tgProxy.nPort);
				curl_easy_setopt(m_url, CURLOPT_PROXY, (char*)CT2CA(m_tgProxy.strServer));
				std::string up;
				up += (char*)CT2CA(m_tgProxy.strName);
				up += ":";
				up += (char*)CT2CA(m_tgProxy.strPass);
				curl_easy_setopt(m_url, CURLOPT_PROXYUSERPWD, up.c_str());
			}	
		}
		if (m_bWriteHeader) {
			curl_easy_setopt(m_url, CURLOPT_HEADERDATA, (void*)this);
			curl_easy_setopt(m_url, CURLOPT_HEADERFUNCTION, StreamHeader);
		}
		//curl_easy_setopt(m_url, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
		if(m_dbg)
		{
			curl_easy_setopt(m_url, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(m_url, CURLOPT_DEBUGFUNCTION, dbg_trace);
			if(bHttps==false)
				curl_easy_setopt(m_url, CURLOPT_DEBUGDATA, (void*)m_dbg);
		}
#if defined(_DEBUG)
		curl_easy_setopt(m_url, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(m_url, CURLOPT_DEBUGFUNCTION, dbg_trace);
#endif
	}
	void	CHttpClient::EnableDecode(bool bDecode) 
	{
		m_bDecodeBody = bDecode;
	}
	void	CHttpClient::PerformParam(const CAtlString& url)
	{
		PerformParam((char*)CT2CA(url));		
	}
	std::string CHttpClient::RequestGet(const CAtlString& url,bool cHeader,bool cParam,bool perform)
	{
		return RequestGet((char*)CT2CA(url),perform);
	}
	std::string CHttpClient::RequestGet(const std::string& url,bool cHeader,bool cParam,bool perform)
	{
		m_rqUrl = url;
		pfmCode = CURL_LAST;
		if (m_url)
		{
			std::string rqFull(url);
			std::string rqParams = encodeParam();
			if(!rqParams.empty())
			{
				rqFull += "?";
				rqFull += rqParams;
			}
			PerformParam(rqFull);
			curl_easy_setopt(m_url, CURLOPT_POST, 0L);			
			curl_easy_setopt(m_url, CURLOPT_HTTPGET, 1L);
			curl_easy_setopt(m_url, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
			if(cParam)
				m_params.clear();
			if(cHeader)
				m_header.clear();
			m_cookie.clear();
			if(perform)
			{
				pfmCode = curl_easy_perform(m_url);
				if(m_notify)
				{
					if(pfmCode != CURLE_OK)
						m_notify->OnComplete(false,curl_easy_strerror(pfmCode));
					else
						m_notify->OnComplete(true,"success");
				}
			}			
		}
		return GetStream();
	}

	std::string	CHttpClient::RequestPost(const CAtlString& url,bool cHeader,bool cParam,bool perform)
	{
		return RequestPost((char*)CT2CA(url),perform,cHeader,cParam);
	}
	std::string CHttpClient::RequestPost(const std::string& url,bool cHeader,bool cParam,bool perform) 
	{
		m_rqUrl = url;
		pfmCode = CURL_LAST;
		if (m_url)
		{
			std::stringstream dbgSS;
			std::string encode = encodeParam();
			PerformParam(url);
			curl_easy_setopt(m_url, CURLOPT_POST, 1L);
			curl_easy_setopt(m_url, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
			if(bHttps && m_dbg)
				dbgSS<<"url "<< url <<std::endl;
			if (!m_params.empty())
			{
				std::string encode = encodeParam();
				curl_easy_setopt(m_url, CURLOPT_POSTFIELDSIZE, encode.length());
				curl_easy_setopt(m_url, CURLOPT_COPYPOSTFIELDS,encode.c_str());
				if(bHttps && m_dbg)
					dbgSS<<"param "<< encode <<std::endl;
			}
			else if (m_rbuf.tellp())
			{
				curl_easy_setopt(m_url, CURLOPT_POSTFIELDSIZE, m_rbuf.tellp());
			}
			if (m_postBoundary)
			{
				curl_easy_setopt(m_url, CURLOPT_HTTPPOST, m_postBoundary);
			}
			if(cParam)
				m_params.clear();
			if(cHeader)
				m_header.clear();
			if(bHttps && m_dbg)
				m_dbg->OnCurlDbgTrace(dbgSS);
			if(perform)
			{
				pfmCode = curl_easy_perform(m_url);
				if(m_notify)
				{
					if(pfmCode != CURLE_OK)
						m_notify->OnComplete(false,curl_easy_strerror(pfmCode));
					else
						m_notify->OnComplete(true,"success");
				}
			}
			if(m_dbg)
			{
				m_dbg->OnCurlDone(m_wbuf,encode.c_str(),url.c_str(),ReqeustCode());
			}
		}
		return GetStream();
	}
	long		CHttpClient::ReqeustCode()
	{
		if(m_url==NULL)
			return -1;
		long code = 0;
		if( CURLE_OK==curl_easy_getinfo(m_url, CURLINFO_RESPONSE_CODE, &code) )
		{
			if(code==0&&pfmCode)
				return pfmCode;
			return code;
		}
		return -1;
	}
	bool CHttpClient::IsResponseChunk()
	{
		bool chunked = false;
		std::string line1 = GetRpHeader("transfer-encoding");
		std::string line2 = GetRpHeader("headertransfer-encoding");
		std::transform(line1.begin(),line1.end(),line1.begin(),::tolower);
		std::transform(line2.begin(),line2.end(),line2.begin(),::tolower);
		if(-1!=line1.find("chunked")||-1!=line2.find("chunked"))
		{
			chunked = true;
		}
		return chunked;
	}
	std::vector<std::string> CHttpClient::GetChunks()
	{
		std::vector<std::string>  ret;
		if(!IsResponseChunk())
		{
			ret.push_back(GetStream());			
			return ret;
		}
		if(pfmCode==CURLE_OPERATION_TIMEDOUT)
			return ret;
		bool error = false;
		long nChuck = -1;
		std::string line;
		m_wbuf.clear();
		m_wbuf.seekp(0);
		m_wbuf.seekg(0);
		while(std::getline(m_wbuf,line))
		{
			sscanf(line.c_str(), "%x", &nChuck);
			if(nChuck==0)
				break;
			if(nChuck<0)
			{
				error = true;
				break;
			}
			std::string elem;
			readChuckFromStream(m_wbuf,elem,nChuck);
			ret.push_back(elem);
			//读取两个字符判断当前chuck末尾
			line[0] = m_wbuf.peek();
			if(line[0]=='\r')
				m_wbuf.seekg(2,std::ios::cur);		
		}
		if(error)
		{
			ret.push_back(GetStream());			
			return ret;
		}
		return ret;
	}
	std::string CHttpClient::MakeChunks()
	{
		std::string ret("");
		if(pfmCode==CURLE_OPERATION_TIMEDOUT)
			return ret;
		std::vector<std::string> vtSome;
		vtSome = GetChunks();
		if(vtSome.size()==1)
			return vtSome[0];
		for(size_t n=0;n<vtSome.size();n++)
		{
			ret += vtSome[n];
		}
		return ret;
	}
	std::string	CHttpClient::GetStream()
	{
		std::string strRet("");
		if(bHttps && m_dbg)
		{
			std::stringstream trace;
			trace<<"code "<<ReqeustCode()<<" data "<< m_wbuf.str();
			trace.flush();
			m_dbg->OnCurlDbgTrace(trace);
		}
		//超时
		if(pfmCode==CURLE_OPERATION_TIMEDOUT)
			return strRet;

		HandleHeader();
		HandleCookie();
		if(200!=ReqeustCode())
			return strRet;	//非200返回空
		if( m_wbuf.tellp() < 0 ) 
			return strRet;
		return m_wbuf.str();
	}
	std::string	CHttpClient::GetRpHeader(const char* key)
	{
		std::string strRet("");
		if (m_headbuf.str().length()< 0)
			return strRet;
		if(key==NULL)
			return m_headbuf.str();
		std::string fdKey(key);
		std::transform(fdKey.begin(), fdKey.end(), fdKey.begin(), ::tolower);
		std::map<std::string,std::string>::iterator it = m_rpHeaders.find(fdKey);
		if(it==m_rpHeaders.end())
			return strRet;
		strRet = it->second;
		return strRet;
	}
	std::string CHttpClient::DecodeUrl(const std::string &v)
	{
		std::string ret;
		char *result = curl_easy_unescape(m_url, v.c_str(), v.length(),0);
		if (result) {
			ret = result;
			curl_free(result);
		}
		return ret;
	}
	std::string CHttpClient::EncodeUrl(const std::string &v) 
	{
		std::string ret;
		char *result = curl_easy_escape(m_url, v.c_str(), v.length());
		if (result) {
			ret = result;
			curl_free(result);
		}
		return ret;
	}
	void	CHttpClient::SetCookie(const std::string &val) {
		m_cookie = val;
	}
	const std::string& CHttpClient::GetCookie(){
		return m_cookie;
	}
	void CHttpClient::HandleHeader()
	{
		std::string line;
		m_headbuf.clear();
		m_headbuf.seekp(0);
		m_headbuf.seekg(0);
		m_rpHeaders.clear();
		while(std::getline(m_headbuf,line))
		{
			size_t pos = line.find(':');
			if(pos==std::string::npos)
				continue;
			std::string key = line.substr(0,pos);
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			m_rpHeaders[key] = line.substr(pos+1);
		}
	}
	std::string CHttpClient::GetContentType()
	{
		return m_contentType;
	}
	void CHttpClient::HandleCookie()
	{
		std::string setCookie = GetRpHeader("set-cookie");
		std::string cookie = GetRpHeader("cookie");
		if(setCookie.size()||cookie.size())
		{
			//肯能存在多个setCookie
			if(m_cookie.size())
			{
				if(m_cookie[m_cookie.length()-1]!='\n')
					m_cookie.erase(m_cookie.length()-1,1);

				if(m_cookie[m_cookie.length()-1]!=';')
					m_cookie += ';';
			}
			if(setCookie.size())
			{
				m_cookie += "\n";
				m_cookie += setCookie;
			}
			else if(cookie.size())
			{
				m_cookie = cookie;
			}
		}
	}
	std::string CHttpClient::encodeParam() 
	{
		std::string ret;
		mapStrings::iterator itPos = m_params.begin();
		for (itPos; itPos != m_params.end(); itPos++)
		{
			std::string strVal;
			if (!itPos->first.empty()) {
				char *key = NULL;
				char *val = NULL;
				if(m_bEncodeUrl)
				{
					key = curl_easy_escape(m_url,itPos->first.c_str(), itPos->first.length());
					if(!itPos->second.empty())
						val = curl_easy_escape(m_url,itPos->second.c_str(),itPos->second.length());
				}
				if (key) {
					ret.append(key);
					curl_free(key);
				}
				else
					ret.append(itPos->first);
				ret.append("=");
				if (val) {
					ret.append(val);
					curl_free(val);
				}
				else
					ret.append(itPos->second);
				ret += "&";
			}
		}
		if (ret.empty()|| m_url==NULL)
			return ret;
		ret.erase( ret.begin()+(ret.length()-1) ); //去掉最后的&
		return ret;
	}
}