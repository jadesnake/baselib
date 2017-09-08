#include "stdafx.h"
#include "IniFile.h"
#include <stdio.h>

#define  MAX_BUFFER 1024

CIniFile::CIniFile(LPCTSTR strFileName)
{
	m_strFileName = strFileName;
}

CIniFile::CIniFile()
{

}

CIniFile::~CIniFile()
{

}

//设置ini文件路径
void CIniFile::SetFilePath(LPCTSTR strFileName)
{
	m_strFileName = strFileName;
}

//获取配置文件的strSection下的strKeyName的值，若此值不存在则将def赋给变量
CAtlString CIniFile::GetValue(LPCTSTR strSection,LPCTSTR strKeyName, LPCTSTR def/* = NULL*/)
{
	CAtlString ret;
	TCHAR strTemp[MAX_BUFFER];
	//优先使用ms api获取配置文件信息
	DWORD dwReadLen =::GetPrivateProfileString(strSection, strKeyName, def, strTemp, ARRAYSIZE(strTemp), m_strFileName);
	return CAtlString(strTemp);
}

LONG CIniFile::GetValue(LPCTSTR strSection, LPCTSTR strKeyName, LONG def)
{
	return ::GetPrivateProfileInt(strSection, strKeyName, def, m_strFileName);
}

double CIniFile::GetValue(LPCTSTR strSection, LPCTSTR strKeyName, double def)
{
	TCHAR strTemp[MAX_PATH];
	TCHAR strDef[MAX_PATH];
	_stprintf_s(strDef,ARRAYSIZE(strDef),TEXT("%f"),def);
	DWORD dwReadLen = ::GetPrivateProfileString(strSection, strKeyName, strDef, strTemp, ARRAYSIZE(strTemp), m_strFileName);
	return _ttol(strTemp);
}

DWORD CIniFile::GetValue(LPCTSTR strSection,LPCTSTR strKeyName, DWORD def)
{
	return ::GetPrivateProfileInt(strSection, strKeyName, def, m_strFileName);
}

bool CIniFile::GetValue(LPCTSTR strSection,LPCTSTR strKeyName, bool def /* = 0 */)
{
	return (::GetPrivateProfileInt(strSection, strKeyName, def, m_strFileName) == TRUE);
}	

BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, LPCTSTR set)
{
	return ::WritePrivateProfileString(strSection, strKeyName, set, m_strFileName);
}

BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, LONG set)
{
	TCHAR strTemp[MAX_PATH];
	_stprintf_s(strTemp,TEXT("%d"),set);
	return ::WritePrivateProfileString(strSection, strKeyName, strTemp, m_strFileName);
}

BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, double set)
{
	TCHAR strTemp[MAX_PATH];
	_stprintf_s(strTemp,TEXT("%f"),set);
	return ::WritePrivateProfileString(strSection, strKeyName, strTemp, m_strFileName);
}

BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, DWORD set)
{
	TCHAR strTemp[MAX_PATH];
	_stprintf_s(strTemp,TEXT("%u"),set);
	return ::WritePrivateProfileString(strSection, strKeyName, strTemp, m_strFileName);
}

BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, BOOL set)
{
	TCHAR strTemp[MAX_PATH];
	_stprintf_s(strTemp,TEXT("%d"),set);
	return ::WritePrivateProfileString(strSection, strKeyName, strTemp, m_strFileName);
}

BOOL CIniFile::DelKeyName(LPCTSTR strSection,LPCTSTR strKeyName)
{
	return ::WritePrivateProfileString(strSection,strKeyName,NULL,m_strFileName);
}

BOOL CIniFile::DelSection(LPCTSTR strSection)
{
	return ::WritePrivateProfileString(strSection,NULL,NULL,m_strFileName);
}
