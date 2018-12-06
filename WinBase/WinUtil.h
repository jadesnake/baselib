#ifndef BASE_WIN32_WIN_UTIL_H_
#define BASE_WIN32_WIN_UTIL_H_


#include <string>

namespace base
{

 bool IsAddressInCurrentModule(void* address);

 bool IsModuleHandleValid(HMODULE module_handle);

 HMODULE GetModuleHandleFromAddress(void* address);

 HMODULE GetCurrentModuleHandle();

 std::wstring GetHostName();

 HANDLE FindProcessByPath(const CAtlString& szPath);
 bool IsExistProcess(const CAtlString& szProcessName);
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
 Software DumpInstallSoftware();

 std::string  AllocGuidA();
 std::wstring AllocGuidW();

 #define MAX_TRACEBUF    502
 void  TraceA(LPCSTR pstrFormat, ...);
 void  TraceW(LPCWSTR pstrFormat, ...);
 #ifdef _UNICODE
 #define MyTRACE TraceW
 #else
 #define MyTRACE TraceA
 #endif
} 



#endif // BASE_WIN32_WIN_UTIL_H_
