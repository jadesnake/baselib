#pragma once
#include <tchar.h>

class CIniFile
{
public:
	void SetFilePath(LPCTSTR strFileName);
	//读取ini文件
	CAtlString GetValue(LPCTSTR strSection,LPCTSTR strKeyName, LPCTSTR def = NULL);
	LONG GetValue(LPCTSTR strSection,LPCTSTR strKeyName, LONG def = 0);
	double GetValue(LPCTSTR strSection,LPCTSTR strKeyName, double def = 0);
	DWORD GetValue(LPCTSTR strSection,LPCTSTR strKeyName, DWORD def = 0);
	bool GetValue(LPCTSTR strSection,LPCTSTR strKeyName, bool def = 0);
	//写ini文件
	BOOL SetValue(LPCTSTR strSection,LPCTSTR strKeyName, LPCTSTR set);
	BOOL SetValue(LPCTSTR strSection,LPCTSTR strKeyName, LONG set);
	BOOL SetValue(LPCTSTR strSection,LPCTSTR strKeyName, double set);
	BOOL SetValue(LPCTSTR strSection,LPCTSTR strKeyName, DWORD set);
	BOOL SetValue(LPCTSTR strSection,LPCTSTR strKeyName, BOOL set);
	//删除ini文件
	BOOL DelKeyName(LPCTSTR strSection,LPCTSTR strKeyName);
	BOOL DelSection(LPCTSTR strSection);

	CIniFile();
	CIniFile(LPCTSTR strFileName);
	virtual ~CIniFile();
private:
	CAtlString m_strFileName;
};
