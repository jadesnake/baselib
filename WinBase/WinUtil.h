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
									HANDLE *process = NULL);

 bool MinimizeProcessWorkingSize();
 bool SingletonRun(const wchar_t *application);

 bool IsRunningOnVistaOrHigher();

 bool OpenResource(const wchar_t *resource,const wchar_t *type,
		void *&data,unsigned long &size,HMODULE module = NULL);
 //获取适配器1 mac
 bool GetMacAddressByNetBIOS(std::string &mac_address);
 //获取适配器1 mac
 void GetMacAddress(std::string &mac);

 std::string AllocGuidA();

} 



#endif // BASE_WIN32_WIN_UTIL_H_
