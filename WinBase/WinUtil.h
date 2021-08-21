#ifndef BASE_WIN32_WIN_UTIL_H_
#define BASE_WIN32_WIN_UTIL_H_


#include <string>

namespace base
{
	void SetAutoStart(CAtlString showname,CAtlString exefile);
	void KillAutoStart(CAtlString showname);

	//遍历系统服务列表根据名称判断是否存在目标服务
	bool HaveService(const CAtlString &strSrvName);
	//重启服务
	bool RestartService(const CAtlString &strSrvName);
	//关闭服务
	bool StopService(const CAtlString &strSrvName); 

	//自动导入证书至根证书颁发机构
	//certfile 证书文件
	//name 证书名称
	bool AutoImportCertFile(CAtlString certfile,CAtlString name);

	std::vector<std::string> GetLocalIp();
	
	bool CanOpenFile(LPCTSTR file);

	bool IsAddressInCurrentModule(void* address);

	bool IsModuleHandleValid(HMODULE module_handle);

	HMODULE GetModuleHandleFromAddress(void* address);

	HMODULE GetCurrentModuleHandle();

	std::wstring GetHostName();

	CAtlString GetRunProcessPath(const CAtlString& szProcessName);
	HANDLE FindProcessByPath(const CAtlString& szPath);
	bool IsExistProcess(const CAtlString& szProcessName,HANDLE *process=NULL);
	bool KillProcess(const CAtlString& szProcessName);

	bool RunApp(const wchar_t *application, HANDLE *process = NULL);
	bool RunAppWithCommand(const wchar_t *application, const wchar_t *command, HANDLE *process = NULL);
	bool RunAppWithRedirection(const wchar_t *application,
		const wchar_t *command,
		HANDLE input,
		HANDLE output,
		HANDLE error,
		bool bConsole,
		HANDLE *process = NULL);
	bool RunConsoleApp(const wchar_t *application, HANDLE *process = NULL);
	bool RunConsoleAppWithCmd(const wchar_t *application, const wchar_t *command, HANDLE *process = NULL);


	bool MinimizeProcessWorkingSize();
	bool SingletonRun(const wchar_t *application);

	bool IsRunningOnVistaOrHigher();

	bool OpenResource(const wchar_t *resource,const wchar_t *type,
		void *&data,unsigned long &size,HMODULE module = NULL);
	//获取适配器1 mac
	bool GetMacAddressByNetBIOS(std::string &mac_address);
	//获取适配器1 mac
	void GetMacAddress(std::string &mac);

	BOOL IsWow64();

	CAtlString GetRegValue(HKEY hKey,LPCTSTR strKey);

	bool InstallLink(LPCTSTR protocol,LPCTSTR appFilePath);

	class Software
	{
	public:
		typedef struct _Data
		{
			CAtlString name;
			CAtlString version;
			CAtlString location;
			CAtlString publisher;
			CAtlString url;
			CAtlString uninstall;
			CAtlString id;
			CAtlString icon;	//DisplayIcon
			bool IsSuc(){
				return (id.IsEmpty()?false:true);
		 }
		}DATA;
		void push(const DATA& d);
		DATA findByName(LPCTSTR name);
		DATA findById(LPCTSTR id);
	public:
		std::map<CAtlString,DATA> data_;
	};
	Software DumpInstallSoftware(bool bMachine=true);

	std::string  AllocGuidA();
	std::wstring AllocGuidW();

	CAtlString GetOcxPath(LPCTSTR name);
	bool SetOcxPath(LPCTSTR name,LPCTSTR path);

	void  TraceA(LPCSTR pstrFormat, ...);
	void  TraceW(LPCWSTR pstrFormat, ...);
	
	#define MAX_TRACEBUF    502
	#ifdef _UNICODE
	#define MyTRACE TraceW
	#else
	#define MyTRACE TraceA
	#endif
} 



#endif // BASE_WIN32_WIN_UTIL_H_
