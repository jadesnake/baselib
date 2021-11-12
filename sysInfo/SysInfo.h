/*************************************************
Author:

Date:2021-07-15

Description:获取系统参数

**************************************************/

#pragma once
#include <atlstr.h>

/*************************************************

Function:                   GetMFDiskSpaceInfo

Description:              获取最大剩余容量磁盘信息

Input:					   diskName //最大剩余容量磁盘(单位G)

                           freeSpace    // 剩余容量(单位G)
Return:                    

*************************************************/ 

void GetMFDiskSpaceInfo(CString &diskName,CString &freeSpace);

/*************************************************

Function:                   GetCPUType

Description:              获取CPU型号信息

Input:					   mCPUType //CPU型号

Return:                    

*************************************************/ 

void GetCPUType(CString &mCPUType);

/*************************************************

Function:                   GetMemoryInfo

Description:              获取系统物理内存信息

Input:					   dwTotalPhys //物理内存总量(单位G)

Return:                    

*************************************************/ 

void  GetMemoryInfo(CString &dwTotalPhys);
/*************************************************

Function:                   GetOSVersion

Description:              获取操作系统版本信息

Input:					   strOSVersion //操作系统版本

Return:                    

*************************************************/ 
void GetOSVersion(CString &strOSVersion);

int round(double number);
