#include "stdafx.h"
#include "ChangR.h"
#include "Http/HttpClient.h"
#include "json/json.h"
#include <algorithm>
#include <fstream>
#include "dir/Dir.h"
#include <atlfile.h>
#import "msscript.ocx"


typedef HRESULT (__stdcall *DllRegisterApi)(void);
typedef HRESULT (__stdcall *DllUnregisterApi)(void);


#define OPT_SUCCESS _T("操作成功")
#define OPT_DEF_ERROR _T("重复登录或请求异常")
#define OPT_BAD_DATA _T("数据异常");
#define OPT_NO_USR _T("纳税人档案信息不存在！")
template<typename T>
bool ToWhat(std::string str,T &r)
{  
	std::stringstream sin(str);
	T tmp;
	if(!(sin >> tmp))
	{
		return false;
	}
	r = tmp;
	return true;
}
BYTE TakeJson(std::string &inS)
{
	size_t bgnPos = inS.find('{');
	size_t endPos = inS.rfind('}');
	if(bgnPos==-1 || endPos==-1)
	{
		return 0x04;
	}
	inS = inS.substr(bgnPos,endPos-bgnPos+1);
	return 0x00;
}
std::string BuildPagerJson(ChangRuan::Pager *p,int columns)
{
	Json::Value jsonRq;
	std::string sColumns;
	for(int n=1;n<columns;n++)
	{
		sColumns+=',';
	}
	for(int n=0;n<5;n++)
	{
		Json::Value tmp;
		switch(n)
		{
		case 0:
			tmp["name"]="sEcho";
			tmp["value"]=1;
			break;
		case 1:
			tmp["name"]="iColumns";
			tmp["value"]=columns;
			break;
		case 2:
			tmp["name"]="sColumns";
			tmp["value"]=sColumns;
			break;
		case 3:
			tmp["name"]="iDisplayStart";
			tmp["value"]=_ttol(p->page)*_ttol(p->nMax); //0
			break;
		case 4:
			tmp["name"]="iDisplayLength";
			tmp["value"] = (char*)CT2CA(p->nMax);
			break;
		}
		jsonRq.append(tmp);
	}
	for(int n=0;n<columns;n++)
	{
		Json::Value tmp;
		CStringA name;
		name.Format("mDataProp_%d",n);
		tmp["name"]=name.GetString();
		tmp["value"]=n;
		jsonRq.append(tmp);
	}
	return jsonRq.toStyledString();
}


class ChangRuanDebug : public curl::CDebug
{
public:
	ChangRuanDebug(ChangRuan::Log *log,const std::string& method)
	{
		this->log = log;
		this->method = method;
	}
	void OnCurlDbgRelease()
	{
		delete this;
	}
	void OnCurlDbgTrace(const std::stringstream& ss)
	{
		if(log)
		{
			log->OnOperator(method);
			log->OnHttpTrace(ss);
		}
	}
private:
	ChangRuan::Log *log;
	std::string method;
};

void CosplayIE(curl::CHttpClient *http,const CString& host,ChangRuan::Log *log,const std::string& method,long overTime=150000)
{
	http->SetDebug(new ChangRuanDebug(log,method));
	http->SetTimeout(overTime);
	http->SetAgent(_T("Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko"));
	http->AddHeader(_T("x-requested-with"),_T("XMLHttpRequest"));
	http->AddHeader(_T("Accept-Language"),_T("zh-CN"));
	http->AddHeader(_T("Referer"),host.GetString());
	http->AddHeader(_T("Accept"),_T("text/javascript, application/javascript, application/ecmascript, application/x-ecmascript, */*; q=0.01"));
	http->AddHeader(_T("Content-Type"),_T("application/x-www-form-urlencoded;charset=utf-8"));
	http->AddHeader(_T("Connection"),_T("Keep-Alive"));
	http->AddHeader(_T("Cache-Control"),_T("no-cache"));
}
CAtlString CodeToError(const std::string& code)
{
	CAtlString ret;
	if(code=="00")
	{
		ret = _T("请稍后再试");
	}
	else if(code=="98")
	{
		ret = _T("网络调用异常，请重试");
	}
	else if(code=="99")
	{
		ret = _T("网络调用超时");
	}
	else if(code=="101")
	{
		ret = _T("数据库连接失败");
	}
	else if(code=="02")
	{
		ret = _T("信息不存在");
	}
	else if(code=="03")
	{
		ret = _T("系统异常");
	}
	else if(code=="04")
	{
		ret = _T("平台密码不正确");
	}
	else if(code=="05")
	{
		ret = _T("平台密码错误次数超过十次，请联系税务机关解锁或明天再试");
	}
	else if( code=="09" )
	{
		ret = _T("会话已超时，请重新登陆！");
	}
	else if(code=="11")
	{
		ret = _T("您还未登录系统，请先登录");
	}
	else if( code=="12")
	{
		ret = _T("请确认本企业是否属于取消认证政策的纳税人");
	}
	else if( code=="13")
	{
		ret = _T("特定企业不允许进行网上发票认证");
	}
	else if(code=="21")
	{
		ret = _T("本平台启用状态为：未启用,无权登录此系统，请联系主管税务机关开通权限");
	}
	else if( code=="10" || code=="20" )
	{
		ret = _T("信息出现异常，请稍后再试！");
	}
	else
	{
		ret = _T("未知异常");
	}
	return ret;
}

ChangRuan::ChangRuan()
{
	m_log = NULL;
	m_hasInitPwd  = false;
	m_Ymbb = _T("3.0.13");
	m_atuoQuit = true;
}
ChangRuan::~ChangRuan()
{
	if(m_atuoQuit)
		Quit();
}
void ChangRuan::SetLog(Log *log)
{
	if(m_log)
		m_log->Release();
	m_log = log;
}

bool ChangRuan::Init()
{
	HRESULT hr = 0;
	//注册长软控件
	//获取接口
	if(crypCtrl) return true;
	hr = crypCtrl.CoCreateInstance(__uuidof(CryptCtl));
	if(!SUCCEEDED(hr))
	{
		if(SUCCEEDED(hr))
			hr = crypCtrl.CoCreateInstance(__uuidof(CryptCtl));
		m_lastMsg = _T("组件获取失败");
		return false;
	}
	GxPt::Encrypt::Get()->AddJsCode( AppendUrl(GetAppPath(),_T("gxpt.js")) );
	m_lastMsg = OPT_SUCCESS;
	return true;
}
bool ChangRuan::CheckEnv()
{
	HRESULT hr = 0;
	CComPtr<_CryptCtl> crypTmp;
	hr = crypTmp.CoCreateInstance(__uuidof(CryptCtl));
	if(!SUCCEEDED(hr))
	{
		if(SUCCEEDED(hr))
			hr = crypCtrl.CoCreateInstance(__uuidof(CryptCtl));
		m_lastMsg = _T("组件获取失败");
		return false;
	}
	if(crypTmp->IsDeviceOpened())
		crypTmp->CloseDevice();
	_bstr_t sNull;
	CComBSTR bstrTmp;
	long code=0;
	crypTmp->OpenDeviceEx(sNull);
	code = crypTmp->ErrCode;
	bstrTmp = (wchar_t*)crypTmp->ErrMsg;
	if(code!=0&&code!=-1&&code!=87)
	{
		//异常
		if(code==167||code==187)
			m_lastMsg = _T("未安装驱动");
		crypTmp->CloseDevice();
		crypTmp.Release();
		return false;
	}
	m_lastMsg = OPT_SUCCESS;
	return true;
}
bool ChangRuan::CheckPwd(const CAtlString& pwd)
{
	if(!crypCtrl)	return false;
	long nCode = 0;
	HRESULT hr = 0;
	CComBSTR bstrTmpB;
	m_pwd = pwd;
	if(!OpenDev())
	{
		crypCtrl->get_ErrMsg(&bstrTmpB);
		m_lastMsg = bstrTmpB;
		return false;
	}
	m_tax.Empty();

	hr = crypCtrl->GetCertInfo(_bstr_t(_T("")),71); 
	bstrTmpB.Empty();
	hr = crypCtrl->get_strResult(&bstrTmpB);
	if(SUCCEEDED(hr))
	{
		m_lastMsg = OPT_SUCCESS;
		m_tax = bstrTmpB;
		m_pwd = pwd;
		m_hasInitPwd = true;
		CloseDev();
		return true;
	}
	CloseDev();
	return false;
}
CAtlString ChangRuan::GetUserInfo()
{
	if(!crypCtrl)	return false;
	HRESULT hr = 0;
	CComBSTR bstrTmp;
	if(OpenDev())
	{
		hr = crypCtrl->GetUserInfo();
		hr = crypCtrl->get_strResult(&bstrTmp);
		if(SUCCEEDED(hr))
			m_lastMsg = OPT_SUCCESS;
		CloseDev();
	}
	CAtlString strR(bstrTmp);
	return strR;
}
const CAtlString& ChangRuan::GetLastMsg()
{
	return m_lastMsg;
}
void ChangRuan::SetPwd(const CAtlString& pwd)
{
	m_pwd = pwd;
}
void ChangRuan::SetArea(AREA area)
{
	m_area = area;
}
CAtlString ChangRuan::GetTickCount()
{
	CAtlString tick;
	CAtlString randStr;
	::srand(::GetTickCount());
	for (int i = 0; i < 21; i++)
	{
		randStr.Format(_T("%d"),::rand()%9) ;
		tick += randStr;
	}
	tick += _T("_");
	::srand(::GetTickCount());
	for (int i = 0; i < 13; i++)
	{
		randStr.Format(_T("%d"),::rand()%9) ;
		tick += randStr;
	}
	return tick;
}

bool ChangRuan::Login(AREA area)
{
	CAtlString url;
	switch (area)
	{
	case BEIJING:
		url = _T("https://fpdk.bjsat.gov.cn");
		break;
	case TIANJIN:
		url = _T("https://fpdk.tjsat.gov.cn");
		break;
	case HEBEI:
		url = _T("https://fpdk.he-n-tax.gov.cn");
		break;
	case SANXI:
		url = _T("https://fpdk.tax.sx.cn");
		break;
	case NEIMENG:
		url = _T("https://fpdk.nm-n-tax.gov.cn");
		break;
	case LIAONING:
		url = _T("https://fpdk.tax.ln.cn");
		break;
	case DALIAN:
		url = _T("https://fpdk.dlntax.gov.cn");
		break;
	case JILIN:
		url = _T("https://fpdk.jl-n-tax.gov.cn:4431");
		break;
	case HEILJ:
		url = _T("https://fpdk.hl-n-tax.gov.cn");
		break;
	case SHANGHAI:
		url = _T("https://fpdk.tax.sh.gov.cn");
		break;
	case JIANGSHU:
		url = _T("https://fpdk.jsgs.gov.cn:81");
		break;
	case ZHEJIANG:
		url = _T("https://fpdk.zjtax.gov.cn");
		break;
	case NINGBO:
		url = _T("https://fpdk.nb-n-tax.gov.cn");
		break;
	case ANHUI:
		url = _T("https://fpdk.ah-n-tax.gov.cn");
		break;
	case FUJIAN:
		url = _T("https://fpdk.fj-n-tax.gov.cn");
		break;
	case XIAMEN:
		url = _T("https://fpdk.xm-n-tax.gov.cn");
		break;
	case JIANGXI:
		url = _T("https://fpdk.jxgs.gov.cn");
		break;
	case SANDONG:
		url = _T("https://fpdk.sd-n-tax.gov.cn");
		break;
	case QINGDAO:
		//青岛版本13
		url = _T("https://fpdk.qd-n-tax.gov.cn");
		m_Ymbb = _T("3.0.13");
		break;
	case HENAN:
		url = _T("https://fpdk.ha-n-tax.gov.cn");
		break;
	case HUBEI:
		url = _T("https://fpdk.hb-n-tax.gov.cn");
		break;
	case HUNAN:
		url = _T("https://fpdk.hntax.gov.cn");
		break;
	case GUANGDONG:
		url = _T("https://fpdk.gd-n-tax.gov.cn");
		break;
	case SHENZHEN:
		url = _T("https://fpdk.szgs.gov.cn");
		break;
	case GUANGXI:
		url = _T("https://fpdk.gxgs.gov.cn");
		break;
	case HAINAN:
		url = _T("https://fpdk.hitax.gov.cn");
		break;
	case CHONGQING:
		url = _T("https://fpdk.cqsw.gov.cn");
		break;
	case SHICHUAN:
		url = _T("https://fpdk.sc-n-tax.gov.cn");
		break;
	case GUIZHOU:
		url = _T("https://fpdk.gz-n-tax.gov.cn");
		break;
	case YUNNAN:
		url = _T("https://fpdk.yngs.gov.cn");
		break;
	case XIZANG:
		url = _T("https://fpdk.xztax.gov.cn");
		break;
	case SHANXI:
		url = _T("https://fprzweb.sn-n-tax.gov.cn");
		break;
	case GANSU:
		url = _T("https://fpdk.gs-n-tax.gov.cn");
		break;
	case QINGHAI:
		url = _T("https://fpdk.qh-n-tax.gov.cn");
		break;
	case NINGXIA:
		url = _T("https://fpdk.nxgs.gov.cn");
		break;
	case XINJIANG:
		url = _T("https://fpdk.xj-n-tax.gov.cn");
		break;
	default:
		m_lastMsg = _T("地区错误");
		return false;
	}
	m_ip = url;
	m_area = area;
	if(!crypCtrl)	return false;

	url += _T("/SbsqWW/login.do?callback=jQuery");
	url += GetTickCount();
	
	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"Login",3);
	http.AddParam("type","CLIENT-HELLO");
	http.AddParam(_T("clientHello"),MakeClientHello());
	http.AddParam("ymbb",(char*)CT2CA(m_Ymbb));
	rp	= http.RequestPost((char*)CT2CA(url),false);
		
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	std::string rezt,token;
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	if(root["key1"].isNull() || root["key1"].empty())
	{
		m_lastMsg = _T("二次验证key值异常");
		return false;
	}
	rezt = root["key1"].asCString();
	if(!root["key2"].isNull())
		token = root["key2"].asCString();
	if(rezt=="03")
	{
		m_lastMsg = OPT_SUCCESS;
		if(!root["key3"].isNull())
			m_nsrmc = (TCHAR*)CA2CT(http.DecodeUrl(root["key3"].asCString()).c_str(),CP_UTF8);
		if(!root["key4"].isNull())
			m_dqrq = (TCHAR*)CA2CT(root["key4"].asCString());
		return true;
	}
	if(rezt=="01")
	{
		m_svrPacket = (TCHAR*)CA2CT(root["key2"].asCString());
		m_svrRandom = (TCHAR*)CA2CT(root["key3"].asCString());
		m_authCode = MakeClientAuthCode(m_svrPacket);
		return SecondLogin();
	}
	m_lastMsg = CodeToError(rezt);
	if(m_lastMsg.IsEmpty())
	{
		m_lastMsg = _T("系统异常，错误代码为");
		m_lastMsg += CA2CT(rezt.c_str());
	}
	return false;
}
bool ChangRuan::SecondLogin()
{
	CAtlString url(m_ip);
	url += _T("/SbsqWW/querymm.do?callback=jQuery");
	url += GetTickCount();

	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"SecondLogin");
	http.AddParam(_T("cert"),m_tax);
	http.AddParam("funType","01");
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string page = root["page"].asCString();
	std::string ts = root["ts"].asCString();
	std::string publickey="";
	if(!page.empty()){
		CAtlStringA tmp(page.c_str());
		tmp.Replace("\"","\\\"");
		page = tmp.GetString();
		publickey = GxPt::Encrypt::Get()->checkTaxno((char*)CT2CA(m_tax),ts,"",page,(char*)CT2CA(m_svrRandom));
	}
	for(int n=0;n<3;n++)
	{
		if(ThirdLogin(ts,publickey))
		{
			m_lastMsg = OPT_SUCCESS;
			return true;
		}
		if(m_lastMsg==OPT_NO_USR)
		{
			return  false;
		}
		//预防频率过高被拒绝
		Sleep(100);
	}
	return true;
}
bool ChangRuan::ThirdLogin(const std::string& ts,const std::string& pubkey)
{
	CAtlString url(m_ip);
	url += _T("/SbsqWW/login.do?callback=jQuery");
	url += GetTickCount();

	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"ThirdLogin");
	http.AddParam("type","CLIENT-AUTH");
	http.AddParam(_T("clientAuthCode"),m_authCode);
	http.AddParam(_T("serverRandom"),m_svrRandom);
	http.AddParam("password","");
	http.AddParam("ts",ts);
	http.AddParam("publickey",pubkey);
	http.AddParam(_T("cert"),m_tax);
	http.AddParam("ymbb",(char*)CT2CA(m_Ymbb));
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	std::string rezt;
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	rezt = root["key1"].asCString();
	if(rezt=="00")
	{
		m_lastMsg = _T("登录失败");
		return false;
	}
	if(rezt=="03")
	{
		m_lastMsg = OPT_SUCCESS;
		m_token = (TCHAR*)CA2CT(root["key2"].asCString());
		m_nsrmc = (TCHAR*)CA2CT(root["key3"].asCString(),CP_UTF8);
		m_dqrq = (TCHAR*)CA2CT(root["key4"].asCString());
		return true;
	}
	if(rezt=="02")
	{
		m_lastMsg = OPT_NO_USR;
		return false;
	}
	m_lastMsg = CodeToError(rezt);
	if(m_lastMsg.IsEmpty())
		m_lastMsg = _T("系统异常 01");
	return false;
}
//
bool ChangRuan::GetDkTjByDate(const CAtlString& date,std::string &out)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;

	CAtlString url(m_ip);
	url += _T("/SbsqWW/dktj.do?callback=jQuery");
	url += GetTickCount();
	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"GetDkTjByDate",3000);
	http.AddParam(_T("cert"),m_tax);
	http.AddParam(_T("token"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	http.AddParam("oper","tj");
	if(!date.IsEmpty())
	{
		CAtlString tmp(date);
		tmp.Remove('-');
		http.AddParam(_T("tjyf"),tmp);
	}
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	Json::Value  root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	if(root["key1"].isNull()|| root["key1"].empty())
	{
		m_lastMsg = _T("抵扣统计查询失败");
		return false;
	}
	std::string retCode = root["key1"].asCString();
	if(retCode=="50")
	{
		m_lastMsg = _T("抵扣统计查询失败");
		return false;
	}
	if(retCode=="20")
	{
		m_token = (TCHAR*)CA2CT(root["key3"].asCString());
		out = root["key2"].asString();
		m_lastMsg = OPT_SUCCESS;
		return true;
	}
	if(retCode=="22")
	{
		m_lastMsg =_T("缺少统计信息");
	}
	else
		m_lastMsg = CodeToError(retCode);
	return false;
}
bool ChangRuan::GetRzTjByNf(const CAtlString& nf,std::string &out)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;

	CAtlString url(m_ip);
	url += _T("/SbsqWW/qrgycx.do?callback=jQuery");
	url += GetTickCount();
	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"GetRzTjByNf",3000);
	http.AddParam(_T("cert"),m_tax);
	http.AddParam(_T("token"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	if(!nf.IsEmpty())
		http.AddParam(_T("rznf"),nf);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(http.IsResponseChunk())
	{
		rp.clear();
		std::vector<std::string> chuncks = http.GetChunks();
		for(size_t t=0;t<chuncks.size();t++)
			rp += chuncks[t];
	}
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	Json::Reader reader;
	Json::Value root;
	if(!reader.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	if(root["key1"].isNull()||root["key1"].empty())
	{
		m_lastMsg = _T("后台接口格式修改或失败");
		return false;
	}
	CAtlString buffer;
	CAtlString strSSQ;	//当前税款所属期
	CAtlString strCZSD;	//当期可勾选、确认截止日期

	std::string retCode(root["key1"].asCString());
	if(retCode=="01")
	{
		m_lastMsg = OPT_SUCCESS;
		CAtlString key5;
		m_token = (TCHAR*)CA2CT(root["key4"].asCString());
		key5 = (TCHAR*)CA2CT(root["key5"].asCString());
		int nWhere=0;
		for(int n=0;n<key5.GetLength();n++)
		{
			if(key5[n]!=';')
				buffer += key5[n];
			else
			{
				nWhere += 1;
				if(nWhere==1)
					strSSQ=buffer;
				else if(nWhere==2)
					strCZSD=buffer;
				buffer.Empty();
			}
		}
		m_skssq = key5;
		m_skssq.Replace(_T(";"),_T("%3B"));
		out = rp;
		return true;
	}
	m_lastMsg = CodeToError(retCode);
	return false;
}
bool ChangRuan::GetQrGxBySsq(const CAtlString& ssq,std::string &out)
{
	if(m_ip.IsEmpty()) return false;

	CAtlString url(m_ip);
	url += _T("/SbsqWW/qrgx.do?callback=jQuery");
	url += GetTickCount();

	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"GetQrGxBySsq");
	http.AddParam("id","querysbzt");
	http.AddParam(_T("key1"),m_tax);
	http.AddParam(_T("key2"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	http.AddParam(_T("ssq"),ssq);
	rp	= http.RequestPost((char*)CT2CA(m_ip),false);
	if(http.IsResponseChunk())
	{
		rp.clear();
		std::vector<std::string> chuncks = http.GetChunks();
		for(size_t t=0;t<chuncks.size();t++)
			rp += chuncks[t];
	}
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	Json::Reader reader;
	Json::Value root;
	if(!reader.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string key1(root["key1"].asCString());
	std::string key2(root["key2"].asCString());
	if(key1=="001")
	{
		m_lastMsg=_T("数据保存失败！");
		return false;
	}
	if(key1=="000")
	{
		m_token  = CA2CT(key2.c_str());
		m_lastMsg= OPT_SUCCESS;
		return true;
	}
	if(key1=="09")
	{
		m_lastMsg=_T("会话已超时，请重新登陆！");
		return false;
	}
	if(key1=="98")
	{
		m_lastMsg=_T("外网调用内网异常，请重试！");
		return false;
	}
	m_lastMsg=_T("接口参数错误,请检查您的网络链接设置");
	return false;
}
bool ChangRuan::QueryQrgx()
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;

	curl::CHttpClient http;
	CAtlString url(m_ip);
	url += _T("/SbsqWW/qrgx.do?callback=jQuery");
	url += GetTickCount();
	std::string rp;
	CosplayIE(&http,m_ip,m_log,"QueryQrgx");
	http.AddParam("id","queryqrxx");
	http.AddParam(_T("key1"),m_tax);
	http.AddParam(_T("key2"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(http.IsResponseChunk())
	{
		rp.clear();
		std::vector<std::string> chuncks = http.GetChunks();
		for(size_t t=0;t<chuncks.size();t++)
			rp += chuncks[t];
	}
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg.Format(_T("QueryQrgx %s"),OPT_DEF_ERROR);
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg.Format(_T("QueryQrgx %s"),OPT_SUCCESS);
		return false;
	}
	std::string key1 = root["key1"].asString();
	if(key1=="00")
	{
		m_lastMsg = _T("查询您当前税款所属期的已确认信息出现异常，请稍后再试");
		return false;
	}
	if(key1=="01")
	{
		m_ljrzs = (TCHAR*)CA2CT(root["key2"].asString().c_str());
		m_dqrq = (TCHAR*)CA2CT(root["key5"].asString().c_str());
		m_token= (TCHAR*)CA2CT(root["key3"].asString().c_str());
		//key6下个月
		m_lastMsg = OPT_SUCCESS;
		return true;
	}
	m_lastMsg = CodeToError(key1);
	return false;
}
bool ChangRuan::BeforeConfirmGx()
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;

	curl::CHttpClient http;
	CAtlString url(m_ip);
	url += _T("/SbsqWW/hqssq.do?callback=jQuery");
	url += GetTickCount();
	std::string rp;
	CosplayIE(&http,m_ip,m_log,"BeforeConfirmGx");
	http.AddParam(_T("cert"),m_tax);
	http.AddParam(_T("token"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg.Format(_T("BeforeConfirmGx %s"),OPT_DEF_ERROR);
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg.Format(_T("BeforeConfirmGx %s"),OPT_SUCCESS);
		return false;
	}
	std::string key1 = root["key1"].asString();
	if(key1=="01")
	{
		m_lastMsg = OPT_SUCCESS;
		m_token = CA2CT(root["key2"].asCString());
		m_cookssq = CA2CT(root["key3"].asCString());
		m_gxrqfw = CA2CT(root["key4"].asCString());
		return true;
	}
	m_lastMsg = CodeToError(key1);
	return false;
}
bool ChangRuan::ConfirmGx(std::string &out)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;
	if(m_cookssq.IsEmpty() || m_gxrqfw.IsEmpty())
	{
		if(!BeforeConfirmGx())
			return false;
		Sleep(100);
		if(!QueryQrgx())
			return false;
	}
	curl::CHttpClient http;
	
	CAtlString cookie;
	cookie += _T("dqrq=");
	cookie += m_dqrq;
	cookie += _T(";");

	cookie += _T("nsrmc=");
	cookie += (TCHAR*)CA2CT( http.EncodeUrl((char*)CT2CA(GetUserInfo(),CP_UTF8)).c_str() );
	cookie += _T(";");

	cookie += _T("skssq=");
	cookie += (TCHAR*)CA2CT( http.EncodeUrl((char*)CT2CA(m_cookssq)).c_str() );
	cookie += _T(";");
	
	cookie += _T("gxrqfw=");
	cookie += (TCHAR*)CA2CT( http.EncodeUrl((char*)CT2CA(m_gxrqfw)).c_str() );
	cookie += _T(";");

	cookie += _T("sxy=");
	cookie += _T("0002");
	cookie += _T(";");

	cookie += _T("token=");
	cookie += (TCHAR*)CA2CT( http.EncodeUrl((char*)CT2CA(m_token)).c_str() );
	
	CAtlString url(m_ip);
	url += _T("/SbsqWW/qrgx.do?callback=jQuery");
	url += GetTickCount();
	
	CAtlString referer(m_ip);
	referer += _T("/sbqr.08eaffb0.html");

	std::string rp;
	CosplayIE(&http,referer,m_log,"ConfirmGx");
	http.AddParam("id","querysbzt");
	http.AddParam(_T("key1"),m_tax);
	http.AddParam(_T("key2"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	if(!cookie.IsEmpty())
		http.SetCookie((char*)CT2CA(cookie));
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg.Format(_T("ConfirmGx %s"),OPT_DEF_ERROR);
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string retS(root["key1"].asCString());
	std::string retO(root["key2"].asCString());
	if(retO=="0")
	{
		m_lastMsg = _T("申报状态出现异常，请稍后再试");
		return false;
	}
	else if(retO=="3" && retS=="01")
	{
		m_qrgxData.clear();
		bool bRet = SecondConfirmGx(out);
		Json::Value ret;
		parser.parse(out,ret);
		ret["qrljcs"]=(char*)CT2CA(m_ljrzs);	//第几次确认勾选
		ret["ssq"] = (char*)CT2CA(m_cookssq);
		out = ret.toStyledString();
		return bRet;
	}
	if (retS=="01" && retO=="2") 
	{
		m_lastMsg = _T("税款所属期的申报工作已完成，本批次发票请您在下期进行勾选认证操作");
		return false;
	}
	if( retS=="01" && retO=="1") 
	{
		m_qrgxData.clear();
		bool bRet = SecondConfirmGx(out);
		Json::Value ret;
		parser.parse(out,ret);
		ret["qrljcs"]=(char*)CT2CA(m_ljrzs);	//第几次确认勾选
		ret["ssq"] = (char*)CT2CA(m_cookssq);
		out = ret.toStyledString();
		return bRet;
	}
	if( retS=="01" && retO=="4") 
	{
		m_qrgxData.clear();
		bool bRet = SecondConfirmGx(out);
		Json::Value ret;
		parser.parse(out,ret);
		ret["qrljcs"]=(char*)CT2CA(m_ljrzs);	//第几次确认勾选
		ret["ssq"] = (char*)CT2CA(m_cookssq);
		out = ret.toStyledString();
		return bRet;
	}
	m_lastMsg = CodeToError(retS);
	return false;
}
bool ChangRuan::ConfirmGxEnd()
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;
	if(m_ljhzxxfs.size()==0)	return false;
	if(m_signature.size()==0)	return false;
	return ThirdConfirmGx(m_ljhzxxfs,m_signature);
}
bool ChangRuan::SecondConfirmGx(std::string &out)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;

	CAtlString url(m_ip);
	url += _T("/SbsqWW/qrgx.do?callback=jQuery");
	url += GetTickCount();

	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"SecondConfirmGx");
	http.AddParam("id","queryqrhz");
	http.AddParam(_T("key1"),m_tax);
	http.AddParam(_T("key2"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg.Format(_T("SecondConfirmGx %s"),OPT_DEF_ERROR);
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string retT = root["key1"].asCString();
	std::string retS = root["key2"].asCString(); //勾选成功返回数据
	std::string retO = root["key4"].asCString();
	std::string retR = root["key5"].asCString();
	std::string token = root["key3"].asCString();
	if(retT=="00")
	{
		m_lastMsg = _T("查询您已勾选的发票信息出现异常，请稍后再试");
		return false;
	}
	if (retT=="01") 
	{
		m_token = CA2CT(token.c_str());
		Json::Value jsonOut;
		jsonOut["key2"] = retS;
		jsonOut["key4"] = retO;
		jsonOut["key5"] = retR;
		out = jsonOut.toStyledString();
		if(!retS.empty())
		{
			if(atol(retO.c_str())>0)
			{
				//勾选提交成功ok
				std::vector<std::string> smWhat;
				if(!GxPt::SplitBy(retS,'*',smWhat))
				{
					m_lastMsg = _T("二次确认 数据格式错误");
					return false;
				}
				m_ljhzxxfs = smWhat[1];
				m_signature = smWhat[2];
				/*
				进项管理需要展示数据不能自动提交
				if(ThirdConfirmGx(m_ljhzxxfs,m_signature))
				{
					m_lastMsg = _T("当前状态：待提交");
					return true;
				}*/
				return true;
			}
			else
			{
				m_lastMsg = _T("当前没有可以确认的数据");
				return false;				
			}
		}
		else
		{
			m_lastMsg = _T("获取确认汇总信息出错，请稍后再试");
			return false;
		}
	}
	else if (retT=="09") 
	{
		m_lastMsg = _T("会话已超时，请重新登陆");
		return false;		
	}
	else if (retT=="10"||retT=="20"||retT=="98"||retT=="99"||retT=="101")
	{
		m_lastMsg = _T("查询您已勾选的发票信息出现异常，请稍后再试");
		return false;
	}
	else
	{
		m_lastMsg = _T("查询您已勾选的发票信息出现异常，请稍后再试");
		return false;
	}
	return true;
}
bool ChangRuan::ThirdConfirmGx(const std::string& p1,const std::string& p2)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;
	CAtlString url(m_ip);
	url += _T("/SbsqWW/qrgx.do?callback=jQuery");
	url += GetTickCount();

	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"ThirdConfirmGx");
	http.AddParam("id","commitqrxx");
	http.AddParam(_T("key1"),m_tax);
	http.AddParam(_T("key2"),m_token);
	http.AddParam("signature",p2);
	http.AddParam(_T("ymbb"),m_Ymbb);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg.Format(_T("ThirdConfirmGx %s"),OPT_DEF_ERROR);
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string key1 = root["key1"].asString();
	std::string key2 = root["key2"].asString();
	if(key1=="00")
	{
		m_lastMsg=_T("提交确认信息出现异常，请稍后再试！");
		return false;
	}
	if(key1=="01"&&(key2=="Y"||key2=="YY"))
	{
		m_token = CA2CT(root["key3"].asCString());
		m_lastMsg=_T("勾选确认成功");
		return true;
	}
	if(key2=="NN")
	{
		m_lastMsg = _T("签名校验不一致，无法完成已勾选发票的确认提交，请重新确认已勾选发票信息！");
		return false;
	}
	m_lastMsg = CodeToError(key1);
	return false;
}

bool ChangRuan::GetFpFromGx(const Query& q,std::string &out)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;

	std::string aoData = BuildPagerJson(const_cast<Query*>(&q),14);
	
	CAtlString url(m_ip);
	url += _T("/SbsqWW/gxcx.do?callback=jQuery");
	url += GetTickCount();
	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"GetFpFromGx");
	http.AddParam("fpdm","");
	http.AddParam("fphm","");
	http.AddParam("xfsbh","");
	if(q.rz==Query::RZ_ALL)
		http.AddParam("rzzt","-1");
	else if(q.rz==Query::RZ_YES)
	{
		http.AddParam("rzzt","1");
		http.AddParam("rzfs","-1");
	}
	else
	{
		http.AddParam("rzzt","0");
		http.AddParam("rzfs","");
	}
	http.AddParam("gxzt","-1");
	http.AddParam("fpzt","-1");
	http.AddParam("fplx","-1");
	http.AddParam(_T("cert"),m_tax);
	http.AddParam(_T("token"),m_token);
	http.AddParam(_T("rq_q"),q.ksrq);
	http.AddParam(_T("rq_z"),q.jsrq);
	http.AddParam(_T("ymbb"),m_Ymbb);
	http.AddParam("aoData",aoData);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0==rp.size())
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	if(http.IsResponseChunk())
	{
		rp.clear();
		std::vector<std::string> chuncks = http.GetChunks();
		for(size_t t=0;t<chuncks.size();t++)
			rp += chuncks[t];
	}
	TakeJson(rp);
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string retcode(root["key1"].asCString());
	if(retcode=="01")
	{
		if(root["key4"].asString()=="0")
		{
			m_lastMsg = _T("没有符合条件的记录！");		
			return false;
		}
		m_token = CA2CT(root["key3"].asCString());
		m_lastMsg = OPT_SUCCESS;
		Json::Value key2 = root["key2"];
		if(!key2.empty())
		{
			Json::Value outJson;
			outJson["iTotalRecords"] = key2["iTotalRecords"];
			outJson["data"] = key2["aaData"];
			out = outJson.toStyledString();
		}
		return true;
	}
	m_lastMsg = CodeToError(retcode);
	return false;
}
bool ChangRuan::QueryDkcx(const Tj& tj,std::string& out)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;
	
	std::string aoData = BuildPagerJson(const_cast<Tj*>(&tj),11);

	CAtlString tmp;
	CAtlString url(m_ip);
	url += _T("/SbsqWW/dktj.do?callback=jQuery");
	url += GetTickCount();
	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"QueryDkcx");
	http.AddParam("aoData",aoData);
	http.AddParam(_T("ymbb"),m_Ymbb);
	http.AddParam(_T("cert"),m_tax);
	http.AddParam(_T("token"),m_token);
	http.AddParam("oper","cx");
	http.AddParam(_T("fpdm"),tj.fpdm);
	http.AddParam(_T("fphm"),tj.fphm);
	http.AddParam(_T("xfsbh"),tj.xfsbh);
	http.AddParam(_T("qrrzrq_q"),tj.qrrzrq_q);
	http.AddParam(_T("qrrzrq_z"),tj.qrrzrq_z);
	http.AddParam("fply","0");
	{
		tmp = tj.tjyf;
		tmp.Remove('-');
		tmp = tmp.Mid(0,6);
		http.AddParam(_T("tjyf"),tmp);
	}
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(http.IsResponseChunk())
	{
		rp.clear();
		std::vector<std::string> chuncks = http.GetChunks();
		for(size_t t=0;t<chuncks.size();t++)
			rp += chuncks[t];
	}
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string key1 = root["key1"].asString();
	if(key1=="00")
	{
		m_lastMsg = _T("查询失败，请稍后再试！");
		return false;
	}
	if(key1=="01")
	{	
		out = root["key2"].toStyledString();
		m_token = CA2CT(root["key3"].asCString());
		m_lastMsg = OPT_SUCCESS;
		return true;
	}
	m_lastMsg = CodeToError(key1);
	return false;
}
bool ChangRuan::QueryQrHzFp(const CAtlString& ssq,std::string& out)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;
	CAtlString url(m_ip);
	url += _T("/SbsqWW/qrgx.do?callback=jQuery");
	url += GetTickCount();
	std::string rp;
	curl::CHttpClient http;
	CAtlString ssqV(ssq);
	ssqV.Remove('-');
	ssqV = ssqV.Mid(0,6);
	if(ssqV.GetLength()<6)
		return false;
	CosplayIE(&http,m_ip,m_log,"QueryRzfp");
	http.AddParam("id","querylsqrxx");
	http.AddParam(_T("ssq"),ssqV);
	http.AddParam(_T("key1"),m_tax);
	http.AddParam(_T("key2"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string key1 = root["key1"].asString();
	if(key1=="00")
	{
		m_lastMsg = _T("查询发票信息出现异常，请稍后再试！");
		return false;
	}
	if(key1=="01")
	{
		std::string token = root["key3"].asString();
		m_token = CA2CT(token.c_str());
		out = root["key2"].toStyledString();
		size_t bgnPos = out.find('"');
		size_t endPos = out.rfind('"');
		if(bgnPos!=-1 && endPos!=-1)
		{
			out = out.substr(bgnPos+1,endPos-bgnPos-1);
		}
		m_lastMsg = OPT_SUCCESS;
		return true;
	}
	m_lastMsg = CodeToError(key1);
	return false;
}
bool ChangRuan::QueryRzfp(const Rz& rz,std::string& out)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;
	std::string aoData = BuildPagerJson(const_cast<Rz*>(&rz),11);
	CAtlString url(m_ip);
	url += _T("/SbsqWW/qrgx.do?callback=jQuery");
	url += GetTickCount();
	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"QueryRzfp");
	http.AddParam("id","queryqrjg");
	if(rz.zt==Rz::RZ_GX)
		http.AddParam("qrzt","1");
	else if(rz.zt==Rz::RZ_QR)
		http.AddParam("qrzt","2");
	http.AddParam(_T("key1"),m_tax);
	http.AddParam(_T("key2"),m_token);
	http.AddParam("aoData",aoData);
	http.AddParam(_T("ymbb"),m_Ymbb);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string key1 = root["key1"].asString();
	std::string nNum = root["key4"].asString();
	Json::Value key2 = root["key2"];
	if(key1=="00")
	{
		m_lastMsg = _T("查询发票信息出现异常，请稍后再试！");
		return false;
	}
	if(key1=="01")
	{
		std::string token = root["key3"].asString();
		m_token = CA2CT(token.c_str());
		out = key2.toStyledString();
		m_lastMsg = OPT_SUCCESS;
		return true;
	}
	if(nNum=="0")
	{
		m_lastMsg = _T("没有符合条件的记录！");
		return false;
	}
	m_lastMsg = CodeToError(key1);
	return false;
}
ChangRuan::MM ChangRuan::QueryPubKey(const char *p)
{
	MM ret;
	CAtlString url(m_ip);
	url += _T("/SbsqWW/querymm.do?callback=jQuery");
	url += GetTickCount();

	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"QueryPubKey");
	http.AddParam(_T("cert"),m_tax);
	http.AddParam("funType","02");
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		return ret;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		return ret;
	}
	CAtlStringA page = root["page"].asCString();
	ret.ts = root["ts"].asCString();
	if(!page.IsEmpty()){
		page.Replace("\"","\\\"");
		if(0==strcmp(p,"checkInvConf"))
		{
			//publickey = $.checkInvConf(cert,getCookie("token") , ts ,'',page);
			ret.pubkey = GxPt::Encrypt::Get()->checkInvConf((char*)CT2CA(m_tax),(char*)CT2CA(m_token),ret.ts,"",page.GetString());
		}
	}
	return ret;
}
bool ChangRuan::SubmitGx(const std::vector<Gx>& gx)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;
	if(gx.size()==0)	return false;
	CAtlString fpdms,fphms,kprqs,zts;
	for(size_t n=0;n<gx.size();n++)
	{
		const Gx *item = &gx[n];
		fpdms += item->dm;
		fpdms += _T("=");
		
		fphms += item->hm;
		fphms += _T("=");
		
		kprqs += item->kprq;
		kprqs += _T("=");
		if(item->zt==Gx::GX_YES)
			zts += _T("1");
		else
			zts += _T("0");
		zts += _T("=");
	}
	fpdms.Delete(fpdms.GetLength()-1);
	fphms.Delete(fphms.GetLength()-1);
	kprqs.Delete(kprqs.GetLength()-1);

	if(zts.GetLength())
	{
		zts.Delete(zts.GetLength()-1);
	}

	if(fphms.IsEmpty()||fpdms.IsEmpty()||kprqs.IsEmpty()||zts.IsEmpty())
	{
		return false;
	}
	MM mm = QueryPubKey();
	
	CAtlString url(m_ip);
	url += _T("/SbsqWW/gxtj.do?callback=jQuery");
	url += GetTickCount();

	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"SubmitGx");
	http.AddParam(_T("fpdm"),fpdms);
	http.AddParam(_T("fphm"),fphms);
	http.AddParam(_T("kprq"),kprqs);
	http.AddParam(_T("zt"),zts);
	http.AddParam(_T("cert"),m_tax);
	http.AddParam(_T("token"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	http.AddParam("ts",mm.ts);
	http.AddParam("publickey",mm.pubkey);

	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = OPT_DEF_ERROR;
		return false;
	}
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = OPT_BAD_DATA;
		return false;
	}
	std::string key1 = root["key1"].asString();
	if(key1=="001")
	{
		m_lastMsg = _T("数据保存失败！");
		return false;
	}
	else if(key1=="000")
	{
		m_token = CA2CT(root["key2"].asCString());
		m_lastMsg = OPT_SUCCESS;
		return true;
	}
	m_lastMsg = CodeToError(key1);
	return false;
}
bool ChangRuan::Quit()
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;
	CAtlString url(m_ip);
	url += _T("/SbsqWW/quit.do?callback=jQuery");
	url += GetTickCount();

	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"Quit");
	http.AddParam(_T("cert"),m_tax);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = _T("请先退出系统");
		return false;
	}
	std::string rezt;
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = _T("数据格式解析错误");
		return false;
	}
	rezt = root["key1"].asCString();
	if(rezt=="01")
	{
		m_lastMsg = OPT_SUCCESS;
		return true;
	}
	m_lastMsg=CodeToError(rezt);
	return false;
}
bool ChangRuan::QueryQy(std::string& out)
{
	if(!crypCtrl)	return false;
	if(m_ip.IsEmpty()) return false;
	CAtlString url(m_ip);
	url += _T("/SbsqWW/nsrcx.do?callback=jQuery");
	url += GetTickCount();

	std::string rp;
	curl::CHttpClient http;
	CosplayIE(&http,m_ip,m_log,"QueryQy");
	http.AddParam(_T("cert"),m_tax);
	http.AddParam(_T("token"),m_token);
	http.AddParam(_T("ymbb"),m_Ymbb);
	rp	= http.RequestPost((char*)CT2CA(url),false);
	if(0x00!=TakeJson(rp))
	{
		m_lastMsg = _T("请先退出系统");
		return false;
	}
	std::string rezt;
	Json::Value root;
	Json::Reader parser;
	if(!parser.parse(rp,root))
	{
		m_lastMsg = _T("数据格式解析错误");
		return false;
	}
	rezt = root["key1"].asCString();
	if(rezt=="01")
	{
		m_lastMsg = OPT_SUCCESS;
		out = root["key2"].asCString();
		m_token = CA2CT(root["key3"].asCString());
		return true;
	}
	m_lastMsg=CodeToError(rezt);
	return false;
	return false;
}
void ChangRuan::Release()
{
	//CloseDev();
	if(crypCtrl)
	{
		//crypCtrl->ClearPin();
		crypCtrl.Release();
	}
}

CAtlString ChangRuan::MakeClientAuthCode(const CAtlString& svrPacket)
{
	CAtlString ret;
	CComBSTR bstrTmp;
	if(!crypCtrl)	return ret;
	if(!OpenDev())	return ret;
	HRESULT hr = crypCtrl->ClientAuth(_bstr_t(svrPacket));
	hr  = crypCtrl->get_strResult(&bstrTmp);
	ret = bstrTmp;
	CloseDev();
	return ret;
}
CAtlString ChangRuan::MakeClientHello()
{
	CAtlString ret;
	CComBSTR bstrTmp;
	if(!crypCtrl)	return ret;
	if(!OpenDev())	return ret;
	CComBSTR key;
	crypCtrl->ClientHello(0);
	crypCtrl->get_strResult(&bstrTmp);
	ret = bstrTmp;
	CloseDev();
	return ret;
}

bool ChangRuan::OpenDev()
{
	if(!crypCtrl)	return false;
	if( !crypCtrl->IsDeviceOpened() )
	{
		crypCtrl->CloseDevice();
	}
	HRESULT hr =0;
	long nCode = 0;
	try
	{
		hr = crypCtrl->OpenDeviceEx(_bstr_t(m_pwd));
		if(SUCCEEDED(hr))
		{
			hr = crypCtrl->get_ErrCode(&nCode);
			if(nCode==0x57)
			{
				if(SUCCEEDED(crypCtrl->OpenDeviceEx(_bstr_t(m_pwd))))
					hr = crypCtrl->get_ErrCode(&nCode);
			}
		}
		if(!SUCCEEDED(hr))
			return false;
		if( nCode != 0 && nCode != -1)
			return false;
	}
	catch(...)
	{
		return false;
	}
	return true;
}
bool ChangRuan::CloseDev()
{
	if(!crypCtrl)	return false;
	HRESULT hr = crypCtrl->CloseDevice();
	if(!SUCCEEDED(hr))
	{
		return false;
	}
	return true;
}
bool ChangRuan::IsInitAndPwd()
{
	return m_hasInitPwd;
}
bool ChangRuan::ReInitAndPwd()
{
	return (Init()&&CheckPwd(m_pwd));
}
bool ChangRuan::ReLogin()
{
	return Login(m_area);
}
void ChangRuan::CopyData(const ChangRuan& cr)
{
	m_token = cr.m_token;
	m_ip = cr.m_ip;
	m_lastMsg = cr.m_lastMsg;
	m_pwd = cr.m_pwd;
	m_tax = cr.m_tax;
	m_Ymbb = cr.m_Ymbb;
	m_nsrmc = cr.m_nsrmc;
	m_dqrq = cr.m_dqrq;
	m_ljrzs= cr.m_ljrzs;
	m_svrPacket = cr.m_svrPacket;
	m_svrRandom = cr.m_svrRandom;
	m_atuoQuit = cr.m_atuoQuit;
	m_skssq = cr.m_skssq;
	m_area = cr.m_area;

	m_cookssq = cr.m_cookssq;
	m_gxrqfw  = cr.m_gxrqfw;
}
void ChangRuan::EnableAutoQuit(bool b)
{
	m_atuoQuit = b;
}
namespace GxPt
{
	void HandleRzTjByNf(const std::string& key3,RzTjs &out)
	{
		size_t all = key3.size();
		std::string tmp;
		int nIndex = 0;
		RzTj rztj;
		if(all==0)	return ;
		out.clear();
		for(size_t n=0;n<all;n++)
		{
			//最后一个字符
			if( n==(all-1) )
			{
				tmp += key3[n];
				rztj.zt = CA2CT(tmp.c_str());
				out.insert(std::make_pair(rztj.tm,rztj));
				continue;
			}
			if(key3[n]!='='&&key3[n]!=';')
			{
				tmp += key3[n];
				continue;
			}
			if(key3[n]=='=')
			{
				if(nIndex==0)
					rztj.tm = CA2CT(tmp.c_str());
				if(nIndex==1)
					rztj.zhangs = CA2CT(tmp.c_str());
				if(nIndex==2)
					rztj.sehj = CA2CT(tmp.c_str());
				if(nIndex==3)
					rztj.zt = CA2CT(tmp.c_str());
				tmp.clear();
				nIndex += 1;
				continue;
			}
			if(key3[n]==';')
			{
				out.insert(std::make_pair(rztj.tm,rztj));
				tmp.clear();
				nIndex = 0;
				rztj.clear();
			}
		}
		return ;
	}
	/* 数据解析规则
	function changeLslb() {
	var e = $("#lsqslb").val().split("~"),
	t = e[2].split("*");
	本次为所属期-------begin
	共勾选发票
	$("#bcqrfpsl").text(t[0])
	有效勾选发票
	$("#bcyxgxsl").text(t[1])
	勾选且扫描认证发票
	$("#bcqrgxqrz").text(t[2])
	勾选不可抵扣发票
	$("#bcqrgxbkdk").text(t[3])
	增值税专用发票 数量 金额 税额
	$("#zpbcsl").text(t[4])	$("#zpbcje").text(t[5])	$("#zpbcse").text(t[6])
	机动车发票 数量 金额 税额
	$("#jdcbcsl").text(t[7]) $("#jdcbcje").text(t[8]) $("#jdcbcse").text(t[9])
	货运发票 数量 金额 税额	
	$("#hybcsl").text(t[10]) $("#hybcje").text(t[11]) $("#hybcse").text(t[12])
	通行费发票 数量 金额 税额	
	$("#txfbcsl").text(t[33]) $("#txfbcje").text(t[34]) $("#txfbcse").text(t[35]) 
	合计 数量 金额 税额	
	$("#hjbcsl").text(t[13]), $("#hjbcje").text(t[14]), $("#hjbcse").text(t[15]);
	//截止本次勾选确认，共确认5 次，累计勾选34张发票，其中：
	var s = token.split("~");
	if("0" == s[0] || "5" == s[0] || "6" == s[0]){
		截止本次勾选确认，有效勾选发票
		$("#ljqryxgxsl").text(t[36])
	}
	else	{
		截止本次勾选确认，有效勾选发票
		$("#ljqryxgxsl").text(t[17])

		$("#ljqrfpsl").text(t[16])
		$("#ljqrgxqrz").text(t[18])
		$("#ljqrgxbkdk").text(t[19])
		累计有效勾选统计
		增值税专用发票 数量 金额 税额
		$("#zpljsl").text(t[20]) $("#zpljje").text(t[21]), $("#zpljse").text(t[22])
		机动车发票 数量 金额 税额
		$("#jdljcsl").text(t[23]) $("#jdljcje").text(t[24]) $("#jdljcse").text(t[25])
		货运发票 数量 金额 税额	
		$("#hyljsl").text(t[26]) $("#hyljje").text(t[27]) $("#hyljse").text(t[28]) 
		合计 数量 金额 税额	
		$("#hjljsl").text(t[29]) $("#hjljje").text(t[30]) $("#hjljse").text(t[31]) 
		
		"1" == t[32] ? $("#tjzt").html("当前状态：已完成") : $("#tjzt").html("当前状态：已提交")

		通行费发票 数量 金额 税额	
		$("#txfljsl").text(t[36]) $("#txfljje").text(t[37]) $("#txfljse").text(t[38]);
	} 
	}
	*/
	void HandleQrHzFp(const std::string& key2,QrHzFp &out)
	{
		if(key2.size()==0)	return ;
		std::vector<std::string> o;
		SplitBy(key2,'=',o);
		for(size_t r = 0;r<o.size();r++)
		{
			QrHzFp::Qr qr;
			std::vector<std::string> i;
			SplitBy(o[r],'~',i);
			qr.cs = CA2CT(i[1].c_str());
			qr.tm = CA2CT(i[0].c_str());
			{
				QrHzFp::Hz zpbc;	//增值税专用发票
				QrHzFp::Hz jdcbc;	//机动车发票
				QrHzFp::Hz hybc;	//货运发票
				QrHzFp::Hz txfbc;	//通行费发票
				QrHzFp::Hz hjbc;	//合计
				std::vector<std::string> v;
				SplitBy(i[2],'*',v);
				if( v.size()>=35 )
				{
					txfbc.column = QrHzFp::TX();
					txfbc.sl = CA2CT(v[33].c_str());
					txfbc.je = CA2CT(v[34].c_str());
					txfbc.se = CA2CT(v[35].c_str());
					qr.pushCurGxTj(txfbc);
				}
				if( v.size()>=15 )
				{
					zpbc.column = QrHzFp::ZP();
					zpbc.sl = CA2CT(v[4].c_str());
					zpbc.je = CA2CT(v[5].c_str());
					zpbc.se = CA2CT(v[6].c_str());
					qr.pushCurGxTj(zpbc);

					jdcbc.column = QrHzFp::JDC();
					jdcbc.sl = CA2CT(v[7].c_str());
					jdcbc.je = CA2CT(v[8].c_str());
					jdcbc.se = CA2CT(v[9].c_str());
					qr.pushCurGxTj(jdcbc);

					hybc.column = QrHzFp::HY();
					hybc.sl = CA2CT(v[13].c_str());
					hybc.je = CA2CT(v[14].c_str());
					hybc.se = CA2CT(v[15].c_str());
					qr.pushCurGxTj(hybc);

					hjbc.column = QrHzFp::HJ();
					hjbc.sl = CA2CT(v[13].c_str());
					hjbc.je = CA2CT(v[14].c_str());
					hjbc.se = CA2CT(v[15].c_str());
					qr.pushCurGxTj(hjbc);
				}
				if( v.size()>=20 && v.size()<=40 )
				{
					zpbc.column = QrHzFp::ZP();
					zpbc.sl = CA2CT(v[20].c_str());
					zpbc.je = CA2CT(v[21].c_str());
					zpbc.se = CA2CT(v[22].c_str());
					qr.pushCountGxTj(zpbc);

					jdcbc.column = QrHzFp::JDC();
					jdcbc.sl = CA2CT(v[23].c_str());
					jdcbc.je = CA2CT(v[24].c_str());
					jdcbc.se = CA2CT(v[25].c_str());
					qr.pushCountGxTj(jdcbc);

					hybc.column = QrHzFp::HY();
					hybc.sl = CA2CT(v[26].c_str());
					hybc.je = CA2CT(v[27].c_str());
					hybc.se = CA2CT(v[28].c_str());
					qr.pushCountGxTj(hybc);
					
					hjbc.column = QrHzFp::HJ();
					hjbc.sl = CA2CT(v[29].c_str());
					hjbc.je = CA2CT(v[30].c_str());
					hjbc.se = CA2CT(v[31].c_str());
					qr.pushCountGxTj(hjbc);

					txfbc.column = QrHzFp::TX();
					txfbc.sl = CA2CT(v[36].c_str());
					txfbc.je = CA2CT(v[37].c_str());
					txfbc.se = CA2CT(v[38].c_str());
					qr.pushCountGxTj(hjbc);
				}
			}
			out.push(qr);
		}
	}
	//
	//处理抵扣统计数据
	//原始数据
	//01=34=1146135.96=140601.98=0=0=0=34=1146135.96=140601.98;
	//02=0=0=0=0=0=0=0=0=0;
	//03=0=0=0=0=0=0=0=0=0;
	//14=0=0=0=0=0=0=0=0=0;
	//99=34=1146135.96=140601.98=0=0=0=34=1146135.96=140601.98;
	//
	void HandleDkTj(const std::string& key2,DkTjs &out)
	{
		if(key2.size()==0)	return ;
		std::vector<std::string> avg;
		SplitBy(key2,';',avg);
		for(size_t t=0;t<avg.size();t++)
		{
			std::vector<std::string> result;
			SplitBy(avg[t],'=',result);
			if(result.size()==0)	break;
			DkTj dktj;
			DkTj::Zu zuGx;	//勾选
			DkTj::Zu zuSm;	//扫描
			DkTj::Zu zuHj;	//合计
			if(result[0]=="01")
			{
				dktj.label = _T("增值税专用发票");
				
				zuGx.label = _T("勾选认证");
				zuGx.sl = CA2CT(result[1].c_str());
				zuGx.je = CA2CT(result[2].c_str());
				zuGx.se = CA2CT(result[3].c_str());
				
				zuSm.label = _T("扫描认证");
				zuSm.sl = CA2CT(result[4].c_str());
				zuSm.je = CA2CT(result[5].c_str());
				zuSm.se = CA2CT(result[6].c_str());
				
				zuHj.label = _T("合计");
				zuHj.sl = CA2CT(result[7].c_str());
				zuHj.je = CA2CT(result[8].c_str());
				zuHj.se = CA2CT(result[9].c_str());

				dktj.push(zuGx);		
				dktj.push(zuSm);
				dktj.push(zuHj);
			}
			if(result[0]=="02")
			{
				dktj.label = _T("货物运输业增值税专用发票");
				
				zuGx.label = _T("勾选认证");
				zuGx.sl = CA2CT(result[1].c_str());
				zuGx.je = CA2CT(result[2].c_str());
				zuGx.se = CA2CT(result[3].c_str());

				zuSm.label = _T("扫描认证");
				zuSm.sl = CA2CT(result[4].c_str());
				zuSm.je = CA2CT(result[5].c_str());
				zuSm.se = CA2CT(result[6].c_str());

				zuHj.label = _T("合计");
				zuHj.sl = CA2CT(result[7].c_str());
				zuHj.je = CA2CT(result[8].c_str());
				zuHj.se = CA2CT(result[9].c_str());

				dktj.push(zuGx);		
				dktj.push(zuSm);
				dktj.push(zuHj);
			}
			if(result[0]=="03")
			{
				dktj.label = _T("机动车销售统一发票");

				zuGx.label = _T("勾选认证");
				zuGx.sl = CA2CT(result[1].c_str());
				zuGx.je = CA2CT(result[2].c_str());
				zuGx.se = CA2CT(result[3].c_str());

				zuSm.label = _T("扫描认证");
				zuSm.sl = CA2CT(result[4].c_str());
				zuSm.je = CA2CT(result[5].c_str());
				zuSm.se = CA2CT(result[6].c_str());

				zuHj.label = _T("合计");
				zuHj.sl = CA2CT(result[7].c_str());
				zuHj.je = CA2CT(result[8].c_str());
				zuHj.se = CA2CT(result[9].c_str());

				dktj.push(zuGx);		
				dktj.push(zuSm);
				dktj.push(zuHj);
			}
			if(result[0]=="14")
			{
				dktj.label = _T("通行费发票");
				zuGx.label = _T("勾选认证");
				zuGx.sl = CA2CT(result[1].c_str());
				zuGx.je = CA2CT(result[2].c_str());
				zuGx.se = CA2CT(result[3].c_str());

				zuSm.label = _T("扫描认证");
				zuSm.sl = CA2CT(result[4].c_str());
				zuSm.je = CA2CT(result[5].c_str());
				zuSm.se = CA2CT(result[6].c_str());

				zuHj.label = _T("合计");
				zuHj.sl = CA2CT(result[7].c_str());
				zuHj.je = CA2CT(result[8].c_str());
				zuHj.se = CA2CT(result[9].c_str());

				dktj.push(zuGx);		
				dktj.push(zuSm);
				dktj.push(zuHj);
			}
			if(result[0]=="99")
			{
				dktj.label = _T("总计");
				zuGx.label = _T("勾选认证");
				zuGx.sl = CA2CT(result[1].c_str());
				zuGx.je = CA2CT(result[2].c_str());
				zuGx.se = CA2CT(result[3].c_str());

				zuSm.label = _T("扫描认证");
				zuSm.sl = CA2CT(result[4].c_str());
				zuSm.je = CA2CT(result[5].c_str());
				zuSm.se = CA2CT(result[6].c_str());

				zuHj.label = _T("合计");
				zuHj.sl = CA2CT(result[7].c_str());
				zuHj.je = CA2CT(result[8].c_str());
				zuHj.se = CA2CT(result[9].c_str());

				dktj.push(zuGx);		
				dktj.push(zuSm);
				dktj.push(zuHj);
			}
			if(dktj.fnZu.size())
				out.push_back(dktj);						
		}
	}
	/*
	当前
	勾选发票数 有效勾选发票 勾选且扫描认证发票 勾选不可抵扣发票 增值税专用发票   金额   税额  机动车发票 金额  税额  货运发票 金额   税额 合计 金额    税额   通行费  金额   税额
	1         1            0               0                1         780.54 46.83  0        0.00  0.00  0       0.00  0.00  1  780.54  46.83  0      0.00  0.00
	累计
	1~1~0~0~1~780.54~46.83~0~0.00~0.00~0~0.00~0.00~1~780.54~46.83~0~0.00~0.00
	*/
	void HandleFirstConfirm(const std::string& json,RzGx &cur,RzGx &dq)
	{
		Json::Value  root;
		Json::Reader reader;
		if(!reader.parse(json,root))	return ;
		Json::Value s = root["key2"];
		cur.qrgxsl = root["qrljcs"].asCString();
		dq.qrgxsl = root["qrljcs"].asCString();
		
		std::vector<std::string> ssq;
		SplitBy(root["ssq"].asCString(),';',ssq);
		cur.ssq = ssq[0].c_str();
		dq.ssq = ssq[0].c_str();

		std::vector<std::string> i;
		SplitBy(s.asCString(),'*',i);
		if(i.size()==0)	return ;
		//0 当前勾选
		for(size_t szI=0;szI<i.size();szI++)
		{
			std::vector<std::string> a;
			SplitBy(i[szI],'~',a);
			if(a.size()==0)	continue;
			RzGx *pOut = (szI==0?&cur:&dq);
			pOut->bcqrfpsl = a[0].c_str();
			pOut->bcyxgxsl = a[1].c_str();
			pOut->bcqrgxqrz= a[2].c_str();
			pOut->bcqrgxbkdk= a[3].c_str();
			for(int nxt=4;nxt<a.size();)
			{
				RzGx::Zu zu;
				if(nxt==4)
					zu.label=_T("增值税专用发票");
				else if(nxt==7)
					zu.label=_T("机动车发票");
				else if(nxt==10)
					zu.label=_T("货运发票");
				else if(nxt==10)
					zu.label=_T("货运发票");
				else if(nxt==13)
					zu.label=_T("合计");
				else if(nxt==16)
					zu.label=_T("通行费发票");

				zu.sl =  a[nxt].c_str();
				nxt += 1;

				zu.je =  a[nxt].c_str();
				nxt += 1;

				zu.se =  a[nxt].c_str();
				nxt += 1;

				pOut->push(zu);
			}
		}		
		return ;
	}
	//
	//"%E6%9D%AD%E5%B7%9E%E7%88%B1%E4%BF%A1%E8%AF%BA%E8%88%AA%E5%A4%A9%E4%BF%A1%E6%81%AF%E6%9C%89%E9%99%90%E5%85%AC%E5%8F%B8=0======330100555199156=A=0=91330106555199156Q=ZC"
	//
	void HandleQy(const std::string& json,const std::string& token,Usr &out)
	{
		curl::CHttpClient http;
		std::vector<std::string> infoArr;
		std::string tmp;
		if(0==SplitBy(json,'=',infoArr))	return ;
		tmp = http.DecodeUrl(infoArr[0]);
		out.qymc = CA2CT(tmp.c_str(),CP_UTF8);
		if(infoArr[1]=="0")
			out.sbzq=_T("月");
		else
			out.sbzq=_T("季");
		{
			std::vector<std::string> tokArr;
			SplitBy(token,'~',tokArr);
			if(tokArr[1]=="1"){
				out.qylx = _T("生产企业");
			}else if(tokArr[1]=="2"){
				out.qylx = _T("外贸企业");
			}else if(tokArr[1]=="3"){
				out.qylx = _T("外综服企业");
			}else{
				out.qylx = _T("-");
			}
		}
		out.oldsh = infoArr[7].c_str();
		out.qysh = infoArr[10].c_str();
		out.level= infoArr[8].c_str();
		//out.nsdq = infoArr[11].c_str();
		return ;
	}
	//
	size_t SplitBy(const std::string& src,char delim,std::vector<std::string> &ret)
	{
		size_t sz = src.size();
		std::string tmp;
		size_t last = 0;  
		size_t index=src.find_first_of(delim,last);  
		if(index==std::string::npos)
		{
			return 0;
		}
		while (index!=std::string::npos)  
		{
			ret.push_back(src.substr(last,index-last));  
			last=index+1;  
			index=src.find_first_of(delim,last);  
		}  
		if (index-last>0)  
		{  
			ret.push_back(src.substr(last,index-last));  
		}
		return ret.size();
	}
	//
	std::string Encrypt::BuildCall(const char *fun,const char *p1,...)
	{
		std::stringstream ssArgs;
		ssArgs << fun << '(';
		if(p1)
		{
			va_list Args;
			va_start(Args, p1);
			do 
			{
				ssArgs << "\"";
				ssArgs.write(p1,strlen(p1));
				ssArgs << "\"";
				ssArgs << ',';
			} while (p1=va_arg(Args,const char*));
			va_end(Args);
		}
		std::string ret = ssArgs.str();
		ret.erase(ret.length()-1,1);
		ret += ");";
		return ret;
	}
	void Encrypt::AddJsCode(HMODULE hInst,LPCTSTR sType,LPCTSTR sName)
	{
		HRSRC hRes = ::FindResource(hInst,sName,sType);
		if (hRes==NULL)		return ;
		HGLOBAL hGlobal = ::LoadResource(hInst, hRes);
		if (hGlobal==NULL)	return ;
		DWORD size = ::SizeofResource(hInst, hRes);
		if (size == 0)		return ;
		void *data = ::LockResource(hGlobal);
		if (data == NULL)	return ;
		std::string stream;
		stream.append((char*)data,size);
		jsCode[sName] = stream;
		return;
	}
	void Encrypt::AddJsCode(const CAtlString& pathFile)
	{
		std::stringstream fContent;
		char tmp[1000] = { 0 };
		CAtlFile file;
		HRESULT hr = file.Create(pathFile,GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING);
		if(!SUCCEEDED(hr))	return ;
		while(1)
		{
			memset(tmp,0,sizeof(tmp));
			DWORD dwReadLen = 0;
			ULONGLONG pos=0,size=0;
			hr = file.GetPosition(pos);
			hr = file.GetSize(size);
			if(pos==size)
				break;
			file.Read(tmp,sizeof(tmp),dwReadLen);
			if(dwReadLen)
				fContent.write(tmp,dwReadLen);
		}
		jsCode[pathFile] = fContent.str();
	}
	std::string Encrypt::checkTaxno(const std::string& a,const std::string& b,const std::string& c,const std::string& d,const std::string& e)
	{
		std::string fun;
		fun = BuildCall("jQueryT.checkTaxno",a.c_str(),b.c_str(),c.c_str(),d.c_str(),e.c_str(),NULL);
	
		std::string ret;
		MSScriptControl::IScriptControlPtr pScriptControl(__uuidof(MSScriptControl::ScriptControl)); 
		pScriptControl->Language = "JavaScript";
		pScriptControl->AllowUI = FALSE; 
		try{
			for(FileMapCode::iterator it=jsCode.begin();it!=jsCode.end();it++)
			{
				pScriptControl->AddCode( it->second.c_str() );
			}
			_variant_t outpar = pScriptControl->Eval(fun.c_str()); 
			ret = CT2CA(outpar.bstrVal);
			pScriptControl.Release();
		}
		catch(_com_error e){

		}
		return ret;
	}
	std::string Encrypt::checkOneInv(const std::string& a,const std::string& b,const std::string& c,const std::string& d,const std::string& e,const std::string& f,const std::string& g)
	{
		std::string fun;
		fun = BuildCall("jQueryT.checkOneInv",a.c_str(),b.c_str(),c.c_str(),d.c_str(),e.c_str(),f.c_str(),g.c_str(),NULL);

		std::string ret;
		MSScriptControl::IScriptControlPtr pScriptControl(__uuidof(MSScriptControl::ScriptControl)); 
		pScriptControl->Language = "JavaScript";
		pScriptControl->AllowUI = FALSE; 
		try{
			for(FileMapCode::iterator it=jsCode.begin();it!=jsCode.end();it++)
			{
				pScriptControl->AddCode( it->second.c_str() );
			}
			_variant_t outpar = pScriptControl->Eval(fun.c_str()); 
			ret = CT2CA(outpar.bstrVal);
			pScriptControl.Release();
		}
		catch(_com_error e){

		}
		return ret;
	}
	std::string Encrypt::checkInvConf(const std::string& a,const std::string& b,const std::string& c,const std::string& d,const std::string& e)
	{
		std::string fun;
		fun = BuildCall("jQueryT.checkInvConf",a.c_str(),b.c_str(),c.c_str(),d.c_str(),e.c_str(),NULL);

		std::string ret;
		MSScriptControl::IScriptControlPtr pScriptControl(__uuidof(MSScriptControl::ScriptControl)); 
		pScriptControl->Language = "JavaScript";
		pScriptControl->AllowUI = FALSE; 
		try{
			for(FileMapCode::iterator it=jsCode.begin();it!=jsCode.end();it++)
			{
				pScriptControl->AddCode( it->second.c_str() );
			}
			_variant_t outpar = pScriptControl->Eval(fun.c_str()); 
			ret = CT2CA(outpar.bstrVal);
			pScriptControl.Release();
		}
		catch(_com_error e)	{

		}
		return ret;
	}
	std::string Encrypt::checkDeduDown(const std::string& a,const std::string& b,const std::string& c,const std::string& d,const std::string& e,const std::string& f)
	{
		std::string fun;
		fun = BuildCall("jQueryT.checkDeduDown",a.c_str(),b.c_str(),c.c_str(),d.c_str(),e.c_str(),f.c_str(),NULL);
		MSScriptControl::IScriptControlPtr pScriptControl(__uuidof(MSScriptControl::ScriptControl)); 
		pScriptControl->Language = "JavaScript";
		pScriptControl->AllowUI = FALSE; 
		std::string ret;
		try{
			for(FileMapCode::iterator it=jsCode.begin();it!=jsCode.end();it++)
			{
				pScriptControl->AddCode( it->second.c_str() );
			}
			_variant_t outpar = pScriptControl->Eval(fun.c_str()); 
			ret = CT2CA(outpar.bstrVal);
			pScriptControl.Release();
		}
		catch(_com_error e){

		}
		return ret;
	}
}