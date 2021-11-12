#include "StdAfx.h"
#include "SysInfo.h"
#include <windows.h>

int round(double number)
{
	return (number>0.0)?(number+0.5):(number-0.5);
}

void GetMFDiskSpaceInfo(CString &diskName,CString &freeSpace)
{

	ULARGE_INTEGER nFreeBytesAvailable;
	ULARGE_INTEGER nTotalNumberOfBytes;
	ULARGE_INTEGER nTotalNumberOfFreeBytes;

	WCHAR Drive[MAX_PATH] = { 0 };//初始化数组用以存储盘符信息
	GetLogicalDriveStrings(MAX_PATH, Drive);//获得本地所有盘符存在Drive数组中

	CString myDrive;
	double mostFreeSpace = 0;
	int i = 0;
	while (Drive[i - 1] != '\0' || Drive[i] != '\0')//搜索数组尝试得到盘符信息，如果两者皆否，则数组搜索到尽头
	{
		myDrive = L"";
		myDrive = Drive[i];
		myDrive = myDrive + L":";//将盘符信息的格式调整为像“C:”这种格式，以作为GetDiskFreeSpaceEx函数的第一个参数
		i += 4;
		
		double mywholespace,myfreespace;
		if (GetDiskFreeSpaceEx(myDrive,
			&nFreeBytesAvailable,
			&nTotalNumberOfBytes,
			&nTotalNumberOfFreeBytes))//利用GetDiskFreeSpaceEx函数获得指定磁盘信息
		{
			mywholespace = nTotalNumberOfBytes.QuadPart;
			mywholespace = mywholespace/1024/1024/1024;//得到磁盘总空间，单位为G
			myfreespace = nTotalNumberOfFreeBytes.QuadPart;
			myfreespace = myfreespace/1024/1024/1024;//得到磁盘剩余空间，单位为G

			if (mostFreeSpace<myfreespace)
			{
				mostFreeSpace = myfreespace;
				freeSpace.Format(_T("%.2lf"),mostFreeSpace);
				//freeSpace = myfreespace;
				diskName = myDrive;
			}
			
		}
	}
}
DWORD deax;
DWORD debx;
DWORD decx;
DWORD dedx;
void ExeCPUID(DWORD veax)//初始化CPU
{
	__asm
	{
		mov eax,veax
			cpuid
			mov deax,eax
			mov debx,ebx
			mov decx,ecx
			mov dedx,edx
	}
}
void GetCPUType(CString &mCPUType)
{

	const DWORD id = 0x80000002; //从0x80000002开始,到0x80000004结束
	CHAR CPUType[49];//用来存储CPU型号信息
	memset(CPUType,0,sizeof(CPUType));//初始化数组

	for(DWORD t = 0 ; t < 3 ; t++ )
	{
		ExeCPUID(id+t);
		//每次循环结束,保存信息到数组
		memcpy(CPUType+16*t+ 0,&deax,4);
		memcpy(CPUType+16*t+ 4,&debx,4);
		memcpy(CPUType+16*t+ 8,&decx,4);
		memcpy(CPUType+16*t+12,&dedx,4);
	}
	mCPUType = CPUType;
}
void  GetMemoryInfo(CString &dwTotalPhys) 
{ 
	double dMaxMemory,dFreeMemory;
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	DWORD nMemoryLoad = statex.dwMemoryLoad;
	DWORD nMaxMemory = statex.ullTotalPhys/1024/1024;
	DWORD nFreeMemory = statex.ullAvailPhys/1024/1024;
	dMaxMemory = nMaxMemory;
	dMaxMemory = dMaxMemory/1024;
	dFreeMemory = nFreeMemory;
	dFreeMemory = dFreeMemory/1024;

	dwTotalPhys.Format(_T("%.2lf"),dMaxMemory);
}
void GetOSVersion(CString &strOSVersion)
{
	//setlocale(LC_ALL, "Chinese-simplified");


	OSVERSIONINFOEX OsVersionInfortion;
	OsVersionInfortion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO*)&OsVersionInfortion);


	CString OSystemVersinName(_T("未知版本"));

	switch (OsVersionInfortion.dwPlatformId)
	{

	case VER_PLATFORM_WIN32_NT:
		if (OsVersionInfortion.dwMajorVersion <= 4)
			OSystemVersinName = _T("WindowsNT");
		if (OsVersionInfortion.dwMajorVersion == 5 && OsVersionInfortion.dwMinorVersion == 0)
			OSystemVersinName = _T("Windows2000");
		if (OsVersionInfortion.dwMajorVersion == 5 && OsVersionInfortion.dwMinorVersion == 1)
			OSystemVersinName = _T("WindowsXP");
		if (OsVersionInfortion.dwMajorVersion == 5 && OsVersionInfortion.dwMinorVersion == 2)
			OSystemVersinName = _T("Windows2003");
		if (OsVersionInfortion.dwMajorVersion == 6 && OsVersionInfortion.dwMinorVersion == 0)
			OSystemVersinName = _T("WindowsVista");
		if (OsVersionInfortion.dwMajorVersion == 6 && OsVersionInfortion.dwMinorVersion == 1)
			OSystemVersinName = _T("Windows7");
		if (OsVersionInfortion.dwMajorVersion == 6 &&OsVersionInfortion.dwMinorVersion == 2)
			OSystemVersinName = _T("Windows10");
		break;
	}
	strOSVersion = OSystemVersinName;
}