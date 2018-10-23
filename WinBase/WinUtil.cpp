#include "stdafx.h"
#include "WinUtil.h"
#include <assert.h>
#include <stdlib.h>
#include <nb30.h>
#include <snmp.h>
#include <iphlpapi.h>
#include <Strsafe.h>
#include <tlhelp32.h>
#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
namespace base
{
HMODULE GetModuleHandleFromAddress(void* address)
{
	MEMORY_BASIC_INFORMATION mbi = {0};
	DWORD result = ::VirtualQuery(address, &mbi, sizeof(mbi));
	assert(result == sizeof(mbi));
	return static_cast<HMODULE>(mbi.AllocationBase);
}

HMODULE GetCurrentModuleHandle()
{
	return GetModuleHandleFromAddress(GetCurrentModuleHandle);
}

bool IsAddressInCurrentModule(void* address)
{
	return GetCurrentModuleHandle() == GetModuleHandleFromAddress(address);
}

std::wstring GetHostName()
{
	std::wstring host_name;
	DWORD name_len = MAX_COMPUTERNAME_LENGTH + 1;
	host_name.resize(name_len);
	bool result = !!::GetComputerName(&host_name[0], &name_len);
	assert(result);
	host_name.resize(name_len);
	return host_name;
}

bool IsModuleHandleValid(HMODULE module_handle)
{
	if (!module_handle)
		return true;
	return module_handle == GetModuleHandleFromAddress(module_handle);
}
bool RunConsoleApp(const wchar_t *application, HANDLE *process)
{
	return RunAppWithRedirection(application, NULL, NULL, NULL, NULL,true,process);
}
bool RunConsoleAppWithCmd(const wchar_t *application, const wchar_t *command, HANDLE *process)
{
	return RunAppWithRedirection(application, command, NULL, NULL, NULL,true,process);
}

bool RunApp(const wchar_t *application, HANDLE *process)
{
	return RunAppWithRedirection(application, NULL, NULL, NULL, NULL,false,process);
}

bool RunAppWithCommand(const wchar_t *application, const wchar_t *command, HANDLE *process)
{
	return RunAppWithRedirection(application, command, NULL, NULL, NULL,false,process);
}

bool RunAppWithRedirection(const wchar_t *application,
						   const wchar_t *command,
						   HANDLE input,
						   HANDLE output,
						   HANDLE error,
						   bool bConsole,
						   HANDLE *process)
{
	PROCESS_INFORMATION pi;
	STARTUPINFOW si;

	memset(&si, 0, sizeof(si));

	if (!!input || !!output || !!error)
		si.dwFlags = STARTF_USESTDHANDLES;

	si.cb			= sizeof(si);
	si.hStdInput	= input ? input : ::GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput	= output ? output : ::GetStdHandle(STD_OUTPUT_HANDLE);
	si.hStdError	= error ? error : ::GetStdHandle(STD_ERROR_HANDLE);
	DWORD createFlags = 0;
	if(bConsole)
		createFlags |= CREATE_NO_WINDOW;

	wchar_t *command_dup = wcsdup(command);
	HANDLE hToken = NULL;
	::OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken);
	if (::CreateProcessAsUserW(hToken,application,
						 command_dup,
						 NULL,
						 NULL,
						 (si.dwFlags & STARTF_USESTDHANDLES) ? TRUE : FALSE,
						 createFlags,
						 NULL,
						 NULL,
						 &si,
						 &pi))
	{
		::CloseHandle(pi.hThread);
		if (process == NULL)
			::CloseHandle(pi.hProcess);
		else
			*process = pi.hProcess;
		::CloseHandle(hToken);
		free(command_dup);
		return true;
	}
	::CloseHandle(hToken);
	free(command_dup);
	return false;
}

bool MinimizeProcessWorkingSize()
{
	OSVERSIONINFOW osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	::GetVersionExW(&osvi);
	if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		::SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);
		return true;
	}

	return false;
}

bool SingletonRun(const wchar_t *application)
{
	assert(application);
	if (application == NULL)
		return false;

	HANDLE hMutex = ::CreateMutexW(NULL, TRUE, application);

	if (hMutex == NULL)
		return false;

	if (::GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::CloseHandle(hMutex);
		return false;
	}
	/* 从此这个互斥信号量就被不受控地打开了，直到进程退出 */
	return true;
}

bool IsRunningOnVistaOrHigher()
{
	OSVERSIONINFO os_version = {0};
	os_version.dwOSVersionInfoSize = sizeof(os_version);
	GetVersionEx(&os_version);
	return os_version.dwMajorVersion >= 6;
}

bool OpenResource(const wchar_t *resource,
				  const wchar_t *type,
				  void *&data,
				  unsigned long &size,
				  HMODULE module)
{
	HRSRC resource_handle = ::FindResourceW(module,
											resource,
											type);
	if (resource_handle == NULL)
		return false;
	HGLOBAL data_handle = ::LoadResource(module, resource_handle);
	if (data_handle == NULL)
		return false;
	size = ::SizeofResource(module, resource_handle);
	if (size == 0)
		return false;
	data = ::LockResource(data_handle);
	if (data == NULL)
		return false;
	return true;
}

bool GetMacAddressByNetBIOS(std::string &mac_address)
{
	typedef struct _ASTAT  
	{  
		ADAPTER_STATUS adapt;  
		NAME_BUFFER NameBuffer[30];  
	}ASTAT, *PASTAT;

	ASTAT     Adapter;  
	NCB       Ncb;  
	UCHAR     uRetCode;  
	LANA_ENUM lenum;  
	int       i;  
	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBENUM;  
	Ncb.ncb_buffer  = (UCHAR *)&lenum;  
	Ncb.ncb_length  = sizeof(lenum);  
	uRetCode        = Netbios(&Ncb);  
	for (i=0; i < lenum.length; ++i)  
	{  
		memset(&Ncb, 0, sizeof(Ncb));  
		Ncb.ncb_command  = NCBRESET;  
		Ncb.ncb_lana_num = lenum.lana[i];  
		uRetCode         = Netbios(&Ncb);                                       
		memset(&Ncb, 0, sizeof(Ncb));  
		Ncb.ncb_command  = NCBASTAT;  
		Ncb.ncb_lana_num = lenum.lana[i];  
		strcpy((char *)Ncb.ncb_callname, "*                               ");  
		Ncb.ncb_buffer   = (unsigned char *)&Adapter;  
		Ncb.ncb_length   = sizeof(Adapter);  
		uRetCode         = Netbios(&Ncb);  
		if (uRetCode == 0)  
		{  
			if (Adapter.adapt.adapter_address[0]+  
				Adapter.adapt.adapter_address[1]+  
				Adapter.adapt.adapter_address[2]+  
				Adapter.adapt.adapter_address[3]+  
				Adapter.adapt.adapter_address[4]+  
				Adapter.adapt.adapter_address[5]!=0)  
			{
				char tmp[128];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02x-%02x-%02x-%02x-%02x-%02x",
					Adapter.adapt.adapter_address[0],  
					Adapter.adapt.adapter_address[1],  
					Adapter.adapt.adapter_address[2],  
					Adapter.adapt.adapter_address[3],  
					Adapter.adapt.adapter_address[4],  
					Adapter.adapt.adapter_address[5]); 
				mac_address = tmp;
				return true;  
			}  
		}
	}
	return false;
}

//////////////////////////////////////  
// 功能：获取适配器特性  
// 参数：   
//   adapter_name 适配器 ID  
// 返回值：成功则返回由参数指定的适配器的特性标志，是一个 DWORD 值，失败返回 0  
//  
UINT GetAdapterCharacteristics(char* adapter_name)  
{  
	if(adapter_name == NULL || adapter_name[0] == 0)  
		return 0;  
	HKEY root = NULL;  
	// 打开存储适配器信息的注册表根键  
	if(ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}", 0, KEY_READ, &root))  
		return 0;  
	DWORD subkeys = 0;  
	// 获取该键下的子键数  
	if(ERROR_SUCCESS != RegQueryInfoKeyA(root, NULL, NULL, NULL, &subkeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL))  
		subkeys = 100;  
	DWORD ret_value = 0;  
	for(DWORD i = 0; i < subkeys; i++)  
	{  
		// 每个适配器用一个子键存储，子键名为从 0 开始的 4 位数  
		char subkey[512];  
		memset(subkey, 0, 512);  
		StringCbPrintfA(subkey, 512, "%04u", i);  
		// 打开该子键  
		HKEY hKey = NULL;  
		if(ERROR_SUCCESS != RegOpenKeyExA(root, subkey, 0, KEY_READ, &hKey))  
			continue;  
		// 获取该子键对应的适配器 ID，存于 name 中  
		char name[MAX_PATH];  
		DWORD type = 0;  
		DWORD size = MAX_PATH;  
		if(ERROR_SUCCESS != RegQueryValueExA(hKey, "NetCfgInstanceId", NULL, &type, (LPBYTE)name, &size))  
		{  
			RegCloseKey(hKey);  
			continue;  
		}  
		// 对比该适配器 ID 是不是要获取特性的适配器 ID  
		if(StrCmpIA(name, adapter_name) != 0)  
		{  
			RegCloseKey(hKey);  
			continue;
		}  
		// 读取该适配器的特性标志，该标志存储于值 Characteristics 中  
		DWORD val = 0;  
		size = 4;  
		LSTATUS ls = RegQueryValueExA(hKey, "Characteristics", NULL, &type, (LPBYTE)&val, &size);  
		RegCloseKey(hKey);  
		if(ERROR_SUCCESS == ls)  
		{  
			ret_value = val;  
			break;  
		}  
	}
	RegCloseKey(root);  
	return ret_value;  
}  
//////////////////////////////////////  
// 功能：获取 Mac 地址的二进制数据  
// 参数：  
//   mac 用于输出 Mac 地址的二进制数据的缓冲区指针  
// 返回值：成功返回 mac 地址的长度，失败返回 0，失败时 mac 中保存一些简单的错误信息，可适当修改，用于调试  
//  
void GetMacAddress(std::string &out)  
{  
	#define NCF_PHYSICAL 0x4
	char mac[512];
	memset(mac, 0, sizeof(mac));  

	DWORD AdapterInfoSize = 0;  
	if(ERROR_BUFFER_OVERFLOW != GetAdaptersInfo(NULL, &AdapterInfoSize))  
	{  
		StringCbPrintfA((LPSTR)mac, sizeof(mac), "GetMAC Failed! ErrorCode: %d", GetLastError());  
		out = mac;
		return ;
	}  
	void* buffer = malloc(AdapterInfoSize);  
	if(buffer == NULL)  
	{  
		lstrcpyA((LPSTR)mac, "GetMAC Failed! Because malloc failed!");  
		out = mac;
		return ;  
	} 
	PIP_ADAPTER_INFO pAdapt = (PIP_ADAPTER_INFO)buffer;  
	if(ERROR_SUCCESS != GetAdaptersInfo(pAdapt, &AdapterInfoSize))  
	{  
		StringCbPrintfA((LPSTR)mac, sizeof(mac), "GetMAC Failed! ErrorCode: %d", GetLastError());  
		free(buffer); 
		out = mac;
		return ;  
	}  
	while(pAdapt)  
	{  
		if(pAdapt->AddressLength >= 6 && pAdapt->AddressLength <= 8)  
		{  
			memcpy(mac, pAdapt->Address, pAdapt->AddressLength);  
			UINT flag = GetAdapterCharacteristics(pAdapt->AdapterName);  
			bool is_physical = ((flag & NCF_PHYSICAL) == NCF_PHYSICAL);  
			if(is_physical)
			{
				BYTE *adaptAddress = pAdapt->Address;
				if(6==pAdapt->AddressLength)
				{
					sprintf(mac,"%02x-%02x-%02x-%02x-%02x-%02x",adaptAddress[0], adaptAddress[1], 
						adaptAddress[2], adaptAddress[3], adaptAddress[4], adaptAddress[5]);
					out = mac;
				}
				else
				{
					sprintf(mac,"%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",adaptAddress[0], adaptAddress[1],
						adaptAddress[2], adaptAddress[3], adaptAddress[4], adaptAddress[5],adaptAddress[6],adaptAddress[7]);
					out = mac;
				}
				break;  
			}
		}  
		pAdapt = pAdapt->Next;  
	}  
	free(buffer);  
}

std::string AllocGuidA()
{
	std::string strRet;
	GUID uID;
	CoCreateGuid(&uID);
	char buf[512];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x"
		,uID.Data1,uID.Data2,uID.Data3,uID.Data4[0],uID.Data4[1]
		,uID.Data4[2],uID.Data4[3],uID.Data4[4],uID.Data4[5]
		,uID.Data4[6],uID.Data4[7]);
	strRet = buf;
	return strRet;
}
std::wstring AllocGuidW()
{
	std::wstring strRet;
	GUID uID;
	CoCreateGuid(&uID);
	wchar_t buf[512];
	memset(buf,0,sizeof(buf));
	wsprintf(buf,L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x"
		,uID.Data1,uID.Data2,uID.Data3,uID.Data4[0],uID.Data4[1]
	,uID.Data4[2],uID.Data4[3],uID.Data4[4],uID.Data4[5]
	,uID.Data4[6],uID.Data4[7]);
	strRet = buf;
	return strRet;
}

void  TraceA(LPCSTR pstrFormat, ...)
{
#ifdef _DEBUG
	char szBuffer[MAX_TRACEBUF] = { 0 };
	va_list args;
	va_start(args, pstrFormat);
	int len = _vscprintf( pstrFormat, args ) // _vscprintf doesn't count
		+ 1; // terminating '\0'
	if(len<MAX_TRACEBUF-1)
	{
		vsprintf_s(szBuffer, ARRAYSIZE(szBuffer)-1, pstrFormat, args);
		strcat(szBuffer, "\n");
		::OutputDebugStringA(szBuffer);
	}
	va_end(args);
#endif
}

void  TraceW(LPCWSTR pstrFormat, ...)
{
#ifdef _DEBUG
	wchar_t szBuffer[MAX_TRACEBUF] = { 0 };
	va_list args;
	va_start(args, pstrFormat);
	int len = _vscwprintf( pstrFormat, args ) // _vscprintf doesn't count
		+ 1; // terminating '\0'
	if(len<MAX_TRACEBUF-1)
	{
		vswprintf_s(szBuffer, ARRAYSIZE(szBuffer)-1, pstrFormat, args);
		wcscat(szBuffer, L"\n");
		::OutputDebugStringW(szBuffer);
	}
	va_end(args);
#endif
}

BOOL IsWow64()    
{    
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);    
	LPFN_ISWOW64PROCESS fnIsWow64Process;
	BOOL bIsWow64 = FALSE;    
	//IsWow64Process is not available on all supported versions of Windows.    
	//Use GetModuleHandle to get a handle to the DLL that contains the function    
	//and GetProcAddress to get a pointer to the function if available.    
	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(    
		GetModuleHandle(TEXT("kernel32")),"IsWow64Process");   
	if(NULL != fnIsWow64Process)    
	{    
		if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))    
		{    
			//handle error    
		}    
	}    
	return bIsWow64;    
}

bool IsExistProcess(const CAtlString& szProcessName)
{
	bool bRet = false;
	PROCESSENTRY32 processEntry32;   
	HANDLE toolHelp32Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,  0);  
	if(((int)toolHelp32Snapshot) != -1)  
	{  
		processEntry32.dwSize = sizeof(processEntry32);  
		if (Process32First(toolHelp32Snapshot, &processEntry32))  
		{  
			do  
			{
				if(0==szProcessName.CompareNoCase(processEntry32.szExeFile))
				{
					bRet = true;
					break;
				}  
			}while (Process32Next(toolHelp32Snapshot, &processEntry32));  
		}  
		CloseHandle(toolHelp32Snapshot);  
	}
	return bRet; 
}

Software DumpSoftware(LPCTSTR szKey , HKEY hParent)
{
	Software ret;
	LRESULT lr;
	HKEY hKey;
	lr = ::RegOpenKey(hParent, szKey, &hKey);
	//不能打开注册表
	if(lr != ERROR_SUCCESS)
	{		 
		return ret;
	} 
	ret.name = base::GetRegValue(hKey, _T("DisplayName"));
	if(ret.name.IsEmpty())
	{
		ret.name = base::GetRegValue(hKey, _T("QuietDisplayName"));
	}
	ret.location = base::GetRegValue(hKey, _T("InstallLocation"));
	ret.version = base::GetRegValue(hKey, _T("DisplayVersion"));
	ret.publisher = base::GetRegValue(hKey, _T("Publisher"));
	ret.url = base::GetRegValue(hKey, _T("URLInfoAbout"));
	RegCloseKey(hKey);
	return ret;
}
std::vector<Software> DumpInstallSoftware()
{
	std::vector<Software> ret;
	HRESULT hr=0;
	CAtlString uninstall = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
	HKEY hKey;
	REGSAM samDesired = KEY_READ;
	DWORD cSubKeys = 0;
	::RegOpenKeyEx(HKEY_LOCAL_MACHINE, uninstall, 0, samDesired, &hKey);
	::RegQueryInfoKey(hKey,NULL, NULL,NULL,&cSubKeys,NULL, NULL,NULL,NULL, NULL, NULL,NULL);
	for(DWORD i=0;i<cSubKeys;i++)
	{
 		TCHAR buffer[MAX_PATH];
		hr = ::RegEnumKey(hKey,i, &buffer[0], sizeof(buffer));
		switch(hr)
		{
		case ERROR_SUCCESS:
			{
				Software soft = base::DumpSoftware(buffer,hKey);
				if(!soft.name.IsEmpty())
				{
					ret.push_back( soft );
				}
			}
			break;
		case ERROR_NO_MORE_ITEMS:
			RegCloseKey(hKey);
			break;
		default:
			RegCloseKey(hKey);
			break;
		}
	}
	return ret;
}
CAtlString GetRegValue(HKEY hKey,LPCTSTR strKey)
{
	CAtlString strValue("");
	DWORD dwSize = 0;
	DWORD dwDataType = REG_NONE;
 	// 获取缓存的长度dwSize及类型dwDataType
	::RegQueryValueEx(hKey, strKey, 0, &dwDataType, NULL, &dwSize);
	switch (dwDataType)
	{
	case REG_DWORD:
		{
			DWORD dwData = 0;
			//获取注册表中指定的键所对应的值
			if(ERROR_SUCCESS == ::RegQueryValueEx(hKey,strKey, 0, &dwDataType,(LPBYTE)&dwData, &dwSize))
			{
				strValue.Format(_T("%d"),dwData);
			}
			break;
		}
	case REG_MULTI_SZ:
		{
			//分配内存大小
			BYTE* lpValue = new BYTE[dwSize];
			//获取注册表中指定的键所对应的值
			if(ERROR_SUCCESS == ::RegQueryValueEx(hKey,strKey, 0, &dwDataType, lpValue, &dwSize))
			{
				strValue = lpValue;
			}
			delete[] lpValue;
			break;
		}
	case REG_SZ:
		{
			//分配内存大小
			wchar_t* lpValue = new wchar_t[dwSize];
			memset(lpValue, 0, dwSize * sizeof(wchar_t));
			//获取注册表中指定的键所对应的值
			if (ERROR_SUCCESS == ::RegQueryValueEx(hKey,strKey, 0, &dwDataType, (LPBYTE)lpValue, &dwSize))
			{
				strValue = CW2CT(lpValue);
			}
			delete[] lpValue;
			break;
		}
	default:
		break;
	}
	return strValue;
}

} // namespace base