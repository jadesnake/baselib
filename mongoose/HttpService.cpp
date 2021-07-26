#include "StdAfx.h"
#include "HttpService.h"
#include "StringUtils/StringUtils.h"
#include "dir/Dir.h"
#include "mongoose.h"
#include <Cryptuiapi.h>
#pragma comment(lib,"Cryptui.lib")
#pragma comment(lib,"Crypt32.lib")

namespace HttpService {
	unsigned char ToHex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	unsigned char FromHex(unsigned char x)
	{
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		return y;
	}

	std::string UriEncode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum((unsigned char)str[i]) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += "+";
			else
			{
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4);
				strTemp += ToHex((unsigned char)str[i] % 16);
			}
		}
		return strTemp;
	}

	std::string UriDecode(const std::string& str)
	{
		std::string strTemp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') strTemp += ' ';
			else if (str[i] == '%')
			{
				unsigned char high = FromHex((unsigned char)str[++i]);
				unsigned char low = FromHex((unsigned char)str[++i]);
				strTemp += high * 16 + low;
			}
			else strTemp += str[i];
		}
		return strTemp;
	}

	bool CmdImportCert(CAtlString certfile)
	{
		CAtlString cmd;
		cmd.Format(L"/c certutil -addstore -f -enterprise -user root \"%s\"", certfile.GetString());
		HINSTANCE hInstance = ShellExecute(NULL, L"open", L"cmd", cmd, NULL, SW_HIDE);
		if ((DWORD)hInstance <= SE_ERR_DLLNOTFOUND)
			return false;
		return true;
	}
	bool AutoImportCertFile(CAtlString certfile,CAtlString name)
	{
		HCERTSTORE hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,0,NULL,
			CERT_STORE_OPEN_EXISTING_FLAG|CERT_SYSTEM_STORE_LOCAL_MACHINE,L"ROOT");
		if(hCertStore==NULL)
			hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,0,NULL,
			CERT_STORE_OPEN_EXISTING_FLAG|CERT_SYSTEM_STORE_LOCAL_MACHINE,L"TrustedPublisher" );
		CRYPTUI_WIZ_IMPORT_SRC_INFO importSrc;
		memset(&importSrc,   0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
		importSrc.dwSize =   sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
		importSrc.dwSubjectChoice =   CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
		importSrc.pwszFileName = certfile;
		importSrc.dwFlags = CRYPT_EXPORTABLE |   CRYPT_USER_PROTECTED;
		if(hCertStore)
		{
			PCCERT_CONTEXT hCert = CertFindCertificateInStore(
				hCertStore,
				X509_ASN_ENCODING|PKCS_7_ASN_ENCODING,
				0,
				CERT_FIND_SUBJECT_STR,name,NULL);
			if(hCert)
			{
				CertFreeCertificateContext(hCert);
				return true;
			}
		}
		if(CryptUIWizImport(CRYPTUI_WIZ_NO_UI|CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE,NULL,NULL,&importSrc,hCertStore)==0)
		{
			if(hCertStore)
			{
				CertCloseStore(hCertStore,NULL);
				hCertStore = NULL;
			}

			if(!CmdImportCert(certfile))
			{
				CAtlString   strErr;
				strErr.Format(_T("证书导入失败,请手动安装 0x%x \n"),GetLastError());
				MessageBox(NULL,strErr,NULL,0);
				return false;
			}
		}
		if(hCertStore)
		{
			CertCloseStore(hCertStore,NULL);
			hCertStore = NULL;
		}
		return true;
	}
	//为了把mongoose藏起来搞得扩展结构避免类型声明冲突
	class ExternMG
	{
	public:
		ExternMG()
		{
			mInited = false;
			nc = NULL;
			mUsr = NULL;
			s_http_port = "26000";
			memset(&bind_opts, 0, sizeof(bind_opts));
			memset(&s_http_server_opts, 0, sizeof(s_http_server_opts));
			s_http_server_opts.enable_directory_listing = "yes";
		}
		~ExternMG()
		{
			if(mInited)
				mg_mgr_free(&mgr);
		}
		void SetSSL(CAtlString certfile,CAtlString keyfile)
		{
			this->certfile = (char*)CT2CA(certfile);
			this->keyfile = (char*)CT2CA(keyfile);
		#if MG_ENABLE_SSL
			bind_opts.ssl_cert = this->certfile.c_str();
			bind_opts.ssl_key =  this->keyfile.c_str();
		#endif
		}
		bool Init(MG_CB(mg_event_handler_t callback,void *user_data),void *usr)
		{
			if(mInited==false)
			{
				mg_mgr_init(&mgr, usr);
				mInited = true;
			}
			mUsr = usr;
			nc = mg_bind_opt(&mgr, s_http_port.c_str(),callback,bind_opts);
			if(nc==NULL)
				return false;
			mg_set_protocol_http_websocket(nc);
			return true;
		}
		int pool(int milli)
		{
			return mg_mgr_poll(&mgr,milli);
		}
		void *mUsr;
		bool mInited;
		mg_mgr mgr;
		mg_connection *nc;
		mg_bind_opts bind_opts;
		std::string s_http_port; //监听端口
		mg_serve_http_opts s_http_server_opts;
		std::string errorTip;
		std::string certfile;
		std::string keyfile;
	};

	RunParam::RunParam()
	{
		mMG = new ExternMG;
		evReady = NULL;
 		evRun = ::CreateEvent(NULL,TRUE,FALSE,NULL);
		mThread = NULL;
		mStop = false;
	}
	RunParam::~RunParam()
	{
		if(evRun)
		{
			::CloseHandle(evRun);
			evRun = NULL;
		}
	}
	void RunParam::SetSSL(CAtlString name,CAtlString pem,CAtlString key,CAtlString cert)
	{
		if(mMG && IsPathFind(pem) && IsPathFind(key) && AutoImportCertFile(cert,name))
		{
			mMG->SetSSL(pem,key);
		}		
	}
	void RunParam::SetPort(CAtlString port)
	{
		if(mMG)
			mMG->s_http_port = (char*)CT2CA(port);
	}
	void RunParam::Stop()
	{
		mStop = true;
		if(evRun)
		{
			::CloseHandle(evRun);
			evRun = NULL;
		}
	}
	ExternMG* RunParam::GetMG()
	{
		return mMG;
	}
	void RunParam::CloseMG()
	{
		if(mMG)
		{
			delete mMG;
			mMG = NULL;
		}
	}
 	//---------------------------------------------------------------------------------
	struct OneParam
	{
		std::string key;
		std::string val;
	};
	OneParam	 ParseForm(const std::string& form)
	{
		OneParam ret;
		std::vector<std::string> q1;
		if(base::SplitBy(form,'=',q1)==0)
			ret.key = form;
		else if(q1.size()==2)
		{
			ret.key = q1[0];
			ret.val = UriDecode(q1[1]);
		}
		return ret;
	}
	ReqHead ParseHead(mg_connection *nc,http_message *hm)
	{
		ReqHead ret;
		for(int n=0;n < MG_MAX_HTTP_HEADERS;n++)
		{
			CAtlStringA name(hm->header_names[n].p,hm->header_names[n].len);
			CAtlStringA tV(hm->header_values[n].p,hm->header_values[n].len);
			name = name.MakeLower();
			if(name == "content-type")
			{
				ret.type = tV.GetString();
				continue;
			}
			if(name == "charset")
			{
 				ret.charset = tV.GetString();
				continue;
			}
			if(name == "accept")
			{
 				ret.accept = tV.GetString();
				continue;
			}
			if(name == "origin")
			{
 				ret.origin = tV.GetString();
				ret.xValue.insert(std::make_pair(name,tV));
				continue;
			}
			if(tV.GetLength())
				ret.xValue.insert(std::make_pair(name,tV));
		}
		if(hm->method.len)
		{
			CAtlStringA tV(hm->method.p,hm->method.len);
			tV.MakeLower();
			ret.method = tV.GetString();
		}
		return ret;
	}
	ReqParam ParseParam(mg_connection *nc,http_message *hm)
	{
		std::string sQuery;
		ReqParam p;
		p.head = ParseHead(nc,hm);		 
		if(hm->body.len)
			p.body.append(hm->body.p,hm->body.len);
		if(hm->query_string.len)
			sQuery.append(hm->query_string.p,hm->query_string.len);
		std::vector<std::string> params;
		if(!sQuery.empty())
		{
			base::SplitBy(sQuery,'&',params);
			for(size_t t=0;t<params.size();t++)
			{
				OneParam op = ParseForm(params[t]);
				p.query[op.key] = op.val; 
			}
		}
		params.clear();
		if(p.head.method=="get" || p.head.type.find("form") != std::string::npos)
		{
			if(p.body.size() && p.body.find('&')==std::string::npos)
			{
				OneParam op = ParseForm(p.body);
				p.params[op.key] = op.val; 
			}
			else
				base::SplitBy(p.body,'&',params);
		}
		else if(p.body.size())
		{
			for(size_t t=0;t<params.size();t++)
			{
				OneParam op = ParseForm(params[t]);
				p.params[op.key] = op.val; 
			}
		}
		return p;
	}
	//---------------------------------------------------------------------------------
	void ResponseContent(mg_connection *nc,http_message *hm,const Result& result,const std::string& origin)
	{
		size_t nSize = result.response.size();
		std::stringstream ss;
		ss << "HTTP/1.1 200 OK\r\n";
		if(result.rpType.empty())
			ss << "Content-Type:text/html;charset=UTF-8\r\n";
		else
			ss << "Content-Type:" << result.rpType.c_str() << ";charset=UTF-8\r\n";
		ss << "Access-Control-Allow-Origin: *\r\n";
		ss << "Access-Control-Allow-Methods:*\r\n";
		ss << "Access-Control-Allow-Credentials:true\r\n";
		ss << "Access-Control-Max-Age:3600\r\n";
		ss << "Access-Control-Allow-Headers:x-requested-with,Authorization,content-type\r\n";
		ss << "Content-Length:"<<nSize<<"\r\n"
			<< "\r\n";
		mg_printf(nc, "%s", ss.str().c_str());
		mg_send(nc,result.response.c_str(),result.response.size());
	}
	std::string MakeDefaultJson(bool ok)
	{
		std::stringstream ss;
		if(!ok)
			ss << "{\"suc\":"<<"false"<<",\"msg\":"<<"\"no handler process request\""<<",\"result\":"<<"{}}";
		else
			ss << "{\"suc\":"<<"true"<<",\"msg\":"<<"\"\""<<",\"result\":"<<"{}}";
		return ss.str();
	}
	void ev_handler(mg_connection *nc, int ev, void *ev_data)
	{
		Handler *usrHandle = reinterpret_cast<Handler*>(nc->mgr->user_data);
		struct http_message *hm = (struct http_message *) ev_data;
		std::string uri;
		Result result;
		switch (ev) 
		{
		case MG_EV_HTTP_REQUEST:
			{
				if(hm->uri.p && hm->uri.p[0]=='/')
					uri.append(hm->uri.p+1,hm->uri.len-1);
				else
					uri.append(hm->uri.p,hm->uri.len);
				ReqParam reqP = ParseParam(nc,hm);
				if(reqP.head.method == "options")
				{
					//跨域请求时浏览器会多发options协议
					result.bNext = false;
					result.response = MakeDefaultJson(true);
					ResponseContent(nc,hm,result,"");
				}
				else if(usrHandle)
				{
					std::string origin;
					result = usrHandle->OnHeader(uri,reqP.head);
					if(result.bNext)
						result = usrHandle->OnRequest(uri,reqP);
					if(result.response.empty())
						result.response = MakeDefaultJson(false);
					ResponseContent(nc,hm,result,origin);
				}
			}
			break;
		default:
			break;
		}
	}
	/*------------------------------------------------------------------------------------------------*/
	bool Service::BindHander(Handler *handler)
	{
		if(!gRunParam.GetMG())
			return false;
		if(!gRunParam.GetMG()->Init(ev_handler,reinterpret_cast<void*>(handler))) 
		{
			if(gRunParam.evReady)
			{
				::SetEvent(gRunParam.evReady);
				::CloseHandle(gRunParam.evReady);
			}
			if(gRunParam.evRun)
				::SetEvent(gRunParam.evRun);
			return false;
		}
		handler->OnInit(&gRunParam);
		return true;
	}
	void Service::Stop()
	{
		gRunParam.Stop();
	}
	bool Service::Run(void)
	{
		if(!gRunParam.GetMG())
			return false;
		bool bFire = false;
		if(bFire==false && gRunParam.evReady)
		{
			::SetEvent(gRunParam.evReady);
			bFire = true;
		}
		::SetEvent(gRunParam.evRun);
		Handler *handler = NULL;
		if(gRunParam.GetMG()&&gRunParam.GetMG()->mUsr)
 			handler = reinterpret_cast<Handler*>(gRunParam.GetMG()->mUsr);
 		for (;;) 
		{
			if(gRunParam.GetMG())
				gRunParam.GetMG()->pool(2000);
			if(bFire==false && gRunParam.evReady)
			{
				::SetEvent(gRunParam.evReady);
				bFire = true;
			}
			if(handler)
				handler->OnIdle();
			if(gRunParam.mStop)
				break;
		}
		gRunParam.CloseMG();
		return true;
	}
}