#pragma once

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

namespace HttpService {
	//运行参数
	class ExternMG;
	class RunParam
	{
	public:
		RunParam();
		~RunParam();
		ExternMG* GetMG();
		void CloseMG();
 		void SetPort(CAtlString port);
		void SetSSL(CAtlString name,CAtlString pem,CAtlString key,CAtlString cert);
		void Stop();
	public:
		HANDLE	evReady;	//当服务启动完成，通知宿主程序事件
		HANDLE  evRun;		//运行标记
		HANDLE  mThread;	//保留
		bool    mStop;		//退出结束标记
		ExternMG *mMG;
 	};

	typedef std::map<std::string,std::string> PARAMS;
	struct ReqHead
	{
		std::string type;
		std::string charset;
		std::string method;
	};
	struct ReqParam
	{
		PARAMS params;
		ReqHead head;
		std::string body;
	};
	struct Result
	{
		bool bNext;	//是否执行下一步处理逻辑，如：OnRequest
		std::string type;	//返回报文格式
		std::string response; //返回报文
	};

	class Handler
	{
	public:
		virtual ~Handler(){	}
		virtual Result OnHeader(const std::string& uri,const ReqHead& type)
		{
			Result ret;
			ret.bNext = true;
			return ret;
		}
		virtual Result OnRequest(const std::string& uri,const ReqParam& param)=0;
		virtual void OnIdle(){ };
	};
	bool BindHander(Handler *handler);
	bool Run(void);

	extern RunParam gRunParam;
};