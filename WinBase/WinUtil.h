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
