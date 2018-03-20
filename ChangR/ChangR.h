#pragma once

//长软接口
#include "cryp_ctrl.tlh"

class ChangRuan
{
public:
	typedef enum{
		BEIJING,	//北京
		TIANJIN,	//天津
		HEBEI,		//河北
		SANXI,		//山西
		NEIMENG,	//内蒙古
		LIAONING,	//辽宁
		DALIAN,		//大连
		JILIN,		//吉林
		HEILJ,		//黑龙江
		SHANGHAI,	//上海
		JIANGSHU,	//江苏
		ZHEJIANG,	//浙江
		NINGBO,		//宁波
		ANHUI,		//安徽
		FUJIAN,		//福建
		XIAMEN,		//厦门
		GUIZHOU,	//贵州
		YUNNAN,		//云南
		XIZANG,		//西藏
		SHANXI,		//陕西
		GANSU,		//甘肃
		QINGHAI,	//青海
		NINGXIA,	//宁夏
		XINJIANG,	//新疆
		JIANGXI,	//江西
		SANDONG,	//山东
		QINGDAO,	//青岛
		HENAN,		//河南
		HUBEI,		//湖北
		HUNAN,		//湖南
		GUANGDONG,	//广东
		SHENZHEN,	//深圳
		GUANGXI,	//广西
		HAINAN,		//海南
		CHONGQING,	//重庆
		SHICHUAN	//四川
	}AREA;
	class Pager
	{
	public:
		Pager()
		{
			nMax= _T("500");
			page=_T("0");
		}
		CAtlString nMax;
		CAtlString page;	//当前页面
	};
	class Query : public Pager
	{
	public:
		Query()
		{
			rz = RZ_ALL;
			
		}
		typedef enum{
			RZ_YES,
			RZ_NO,
			RZ_ALL
		}ZTRZ;
		ZTRZ rz;	//认证
		CAtlString ksrq;	//开始日期
		CAtlString jsrq;	//结束日期
	};
	//勾选
	class Gx
	{
	public:
		//勾选状态
		typedef enum{
			GX_YES,
			GX_NO
		}ZT;
		Gx()
		{
			zt = GX_NO;
		}
		CAtlString dm;
		CAtlString hm;
		CAtlString kprq;
		ZT zt;
	};
	class Rz : public Pager
	{
	public:
		typedef enum{
			RZ_GX,	//已勾选未确认
			RZ_QR	//已经确认
		}RZZT;
		Rz()
		{
			zt = RZ_GX;
		}
		RZZT zt;	//认证状态
	};
	//统计查询
	class Tj : public Pager
	{
	public:
		CAtlString tjyf;
		CAtlString fpdm;
		CAtlString fphm;
		CAtlString xfsbh;
		CAtlString qrrzrq_q;
		CAtlString qrrzrq_z;
	};
	class Log
	{
	public:
		virtual ~Log(){	}
		virtual void Release(){	 delete this; }		
		virtual void OnHttpTrace(const std::stringstream& ss)=0;
		virtual void OnOperator(const std::string& method){ }
	};

	ChangRuan();
	virtual ~ChangRuan();
	void SetLog(Log *log);
	bool IsInitAndPwd();
	void SetPwd(const CAtlString& pwd);
	void SetArea(AREA area);
	bool Init();
	bool CheckPwd(const CAtlString& pwd);
	bool ReInitAndPwd();
	bool ReLogin();
	bool Login(AREA area);
	bool Quit();
	CAtlString GetUserInfo();
	//查询发票信息
	bool GetFpFromGx(const Query& q,std::string &out);
	//根据年份获取认证信息
	bool GetRzTjByNf(const CAtlString& nf,std::string &out);
	//根据年份月份获取抵扣统计
	//date格式"201702"
	bool GetDkTjByDate(const CAtlString& date,std::string &out);
	//根据税款所属期查询
	bool GetQrGxBySsq(const CAtlString& ssq,std::string &out);
	//确认勾选
	bool ConfirmGx();
	//保存勾选状态
	bool SubmitGx(const std::vector<Gx>& gx);
	//查询勾选认证发票
	bool QueryRzfp(const Rz& rz,std::string& out);
	//统计查询
	bool QueryDkcx(const Tj& tj,std::string& out);
	//发票确认汇总查询
	bool QueryQrHzFp(const CAtlString& ssq,std::string& out);
	//
	const CAtlString& GetLastMsg();
	void CopyData(const ChangRuan& cr);
	void Release();
	void EnableAutoQuit(bool b);
	bool QueryQrgx();

	const CAtlString& GetTaxNo() const	{
		return m_tax;
	}
	const CAtlString& GetDqrq() const {
		return m_dqrq;
	}
	const CAtlString& GetLjrzs() const {
		return m_ljrzs;
	}
protected:
	bool SecondLogin();
	bool SecondConfirmGx();
	bool ThirdConfirmGx(const std::string& p1,const std::string& p2);

	CAtlString MakeClientAuthCode(const CAtlString& svrPacket);
	CAtlString MakeClientHello();
	bool OpenDev();
	bool CloseDev();
	CAtlString GetTickCount();
	bool BeforeConfirmGx();
public:
	CAtlString   m_token;
	CAtlString   m_ip;
	CAtlString   m_pwd;
	CAtlString   m_authCode;
	//通过QueryQrgx接口获取
	CAtlString   m_ljrzs;	//累计认证次数
	CAtlString	 m_dqrq;	//当前时间
	//
	CAtlString   m_Ymbb;
	CAtlString	 m_nsrmc;
	CAtlString   m_svrPacket;
	CAtlString   m_svrRandom;
	CAtlString   m_skssq;
	CAtlString	 m_tax;
	//用于确认勾选
	CAtlString   m_cookssq;	//勾选所属期
	CAtlString   m_gxrqfw;	//勾选范围
	//
	AREA m_area;
private:
	Log	*m_log;
	CAtlString   m_lastMsg;

	CComPtr<_CryptCtl>  crypCtrl;
	
	//勾选确认后返回的数据
	std::string  m_qrgxData;

	bool m_atuoQuit;
	bool m_hasInitPwd;
};
//勾选平台
namespace GxPt
{
	//抵扣统计数据
	class DkTj
	{
	public:
		class Zu
		{
		public:
			CAtlString label;	//标签
			CAtlString sl;		//数量
			CAtlString se;		//税额
			CAtlString je;		//金额
		};
		void push(const Zu& zu){
			fnZu.push_back(zu);
		}
		std::vector<Zu>	fnZu;	//勾选，扫描，合计
		CAtlString label;		//标签
	};
	typedef std::vector<DkTj> DkTjs; //抵扣统计数据
	//首页认证统计数据
	class RzTj
	{
	public:
		CAtlString tm;		//时间
		CAtlString zhangs;	//发票张数
		CAtlString sehj;	//税额合计
		CAtlString zt;		//状态 1-已申报 0-当前所属期 2-未申报
		void clear(){
			tm.Empty();
			zhangs.Empty();
			sehj.Empty();
		}
	};
	//确认汇总数据
	class QrHzFp
	{
	public:
		static const TCHAR* ZP(){
			return _T("增值税专用发票");
		}
		static const TCHAR* JDC(){
			return _T("机动车发票");
		}
		static const TCHAR* HY(){
			return _T("货运发票");
		}
		static const TCHAR* TX(){
			return _T("通行费发票");
		}
		static const TCHAR* HJ(){
			return _T("合计");
		}
		class Hz
		{
		public:
			CAtlString column;
			CAtlString sl;	//数量
			CAtlString je;	//金额
			CAtlString se;	//税额
		};
		typedef std::map<CAtlString,Hz> HashHz;	//汇总数据
		class Qr
		{
		public:
			CAtlString cs;	//第几次确认
			CAtlString tm;	//确认时间
			HashHz curGxTj;	//本次有效勾选统计
			HashHz countGxTj;  //累计有效勾选统计
			inline void pushCurGxTj(const Hz& hz){
				curGxTj.insert( std::make_pair(hz.column,hz) );
			}
			inline void pushCountGxTj(const Hz& hz){
				countGxTj.insert( std::make_pair(hz.column,hz) );
			}
			Hz getHzByKey(const TCHAR *key){
				Hz ret;
				HashHz::iterator it = countGxTj.find(key);
				if(it != countGxTj.end())
					ret = it->second;
				return ret;
			}
		};
		inline void push(const Qr& qr){
			qrs.push_back(qr);
		}		
	public:
		std::vector<Qr> qrs;
	
	};
	//
	struct SortDesc
	{
		bool operator()(const CAtlString& k1,const CAtlString& k2)const
		{
			return k1>k2;
		}
	};
	typedef std::map<CAtlString,RzTj,SortDesc> RzTjs;
	//处理GetRzTjByNf 返回的key3值
	void HandleRzTjByNf(const std::string& key3,RzTjs &out);
	//处理QueryQrHzFp 返回的key2值
	void HandleQrHzFp(const std::string& key2,QrHzFp &out);
	//处理抵扣统计数据
	void HandleDkTj(const std::string& key2,DkTjs &out);
	//
	size_t SplitBy(const std::string& src,char delim,std::vector<std::string> &ret);
	
}