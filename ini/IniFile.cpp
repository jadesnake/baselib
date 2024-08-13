#include "stdafx.h"
#include "IniFile.h"
#include <stdio.h>
#include <fstream>
#include <sstream>
#define MAX_BUFFER 1000

CIniFile::CIniFile(LPCTSTR strFileName)
{
	m_strFileName = strFileName;
	MakeData();
}

CIniFile::CIniFile()
{

}

CIniFile::~CIniFile()
{

}
void CIniFile::MakeData()
{
	mAllData.clear();
 	std::fstream file(m_strFileName.GetString());
	std::string line;
	CIniFile::Data *pData = NULL;
	while(std::getline(file, line))
	{
		CAtlStringA sTmpLine(line.c_str());
		sTmpLine = sTmpLine.Trim();
		std::string sSection = Mid(sTmpLine.GetString(),'[',']');
		if(!sSection.empty() && sTmpLine[0]=='[')
		{
			pData = FindSection((TCHAR*)CA2CT(sSection.c_str()));
			if(pData==NULL)
				pData = AllocSection((TCHAR*)CA2CT(sSection.c_str()));
			continue;
		}
		if(pData==NULL) continue;
		std::vector<std::string> kvs;
		if(Split(line, '=', kvs)!=2)
			continue;
		sTmpLine = kvs[1].c_str();
		sTmpLine = sTmpLine.Trim();
		if(sTmpLine.GetLength() && sTmpLine[0]=='\'')
			sTmpLine = Mid(sTmpLine.GetString(),'\'','\'').c_str();
		else if(sTmpLine.GetLength() && sTmpLine[0]=='\"')
			sTmpLine = Mid(sTmpLine.GetString(),'\"','\"').c_str();
		pData->Set((TCHAR*)CA2CT(kvs[0].c_str()), (TCHAR*)CA2CT(sTmpLine));
	}
	file.close();
	return ;
}
size_t CIniFile::Split(const std::string& src,char delim,std::vector<std::string>& ret)
{
	size_t last = src.find_first_of(delim);
	if(last==std::string::npos)	return 0;
	if(last==0)	return 0;
	ret.clear();
	ret.push_back(src.substr(0,last));  
	ret.push_back(src.substr(last+1));  
	return ret.size();
}
std::string CIniFile::Mid(const std::string& val,char a,char b)
{
	size_t cA = val.find_first_of(a);
	size_t cB = val.find_last_of(b);
	if(cA==std::string::npos || cB==std::string::npos)
		return "";
	return val.substr(cA+1,cB-1);
}
CIniFile::Data* CIniFile::FindSection(CAtlString name)
{
	std::vector<CIniFile::Data>::iterator it = mAllData.begin();
	for(it;it!=mAllData.end();it++)
	{
		CIniFile::Data *pData = &(*it);
		if(pData->mSection.CompareNoCase(name)==0){
			return pData;
		}
	}
	return NULL;
}
CIniFile::Data* CIniFile::AllocSection(CAtlString name)
{
	name = name.Trim();
	CIniFile::Data nData;
	nData.mSection = name;
	mAllData.push_back(nData);
	return FindSection(name);
}
//设置ini文件路径
void CIniFile::SetFilePath(LPCTSTR strFileName)
{
	m_strFileName = strFileName;
	MakeData();
}
void CIniFile::Flush()
{
	std::fstream file(m_strFileName.GetString(),std::ios::out | std::ios::trunc);
	std::vector<CIniFile::Data>::iterator it = mAllData.begin();
	for(it;it!=mAllData.end();it++)
	{
		CIniFile::Data *pData = &(*it);
		std::string tmp = (char*)CT2CA(pData->mSection.GetString());
		file<<"["<<tmp.c_str()<<"]"<<std::endl;
		std::map<CAtlString,CAtlString>::iterator it = pData->mData.begin();
		for(it;it!=pData->mData.end();it++)
		{
			std::string tmpK = (char*)CT2CA(it->first.GetString());
			std::string tmpV = (char*)CT2CA(it->second.GetString());
			file<<tmpK.c_str()<<"="<<tmpV.c_str()<<std::endl;
		}
	}
	file.close();
	return ;
}
//获取配置文件的strSection下的strKeyName的值，若此值不存在则将def赋给变量
CAtlString CIniFile::GetValue(LPCTSTR strSection,LPCTSTR strKeyName, LPCTSTR def/* = NULL*/)
{
	CAtlString ret;
	CIniFile::Data *pData = FindSection(strSection);
	if(pData)
		ret = pData->Find(strKeyName);
	if(ret.IsEmpty() && def)
		ret = def;
	return ret;
}

LONG CIniFile::GetValue(LPCTSTR strSection, LPCTSTR strKeyName, LONG def)
{
	CAtlString ret;
	CIniFile::Data *pData = FindSection(strSection);
	if(pData)
		ret = pData->Find(strKeyName);
	if(ret.IsEmpty())
		return def;
	return _ttol(ret.GetString());
}

double CIniFile::GetValue(LPCTSTR strSection, LPCTSTR strKeyName, double def)
{
	CAtlString ret;
	CIniFile::Data *pData = FindSection(strSection);
	if(pData)
		ret = pData->Find(strKeyName);
	if(ret.IsEmpty())
		return def;
	return _tstof(ret.GetString());
}

DWORD CIniFile::GetValue(LPCTSTR strSection,LPCTSTR strKeyName, DWORD def)
{
	return GetValue(strSection, strKeyName, def);
}

bool CIniFile::GetValue(LPCTSTR strSection,LPCTSTR strKeyName, BOOL def /* = 0 */)
{
	CAtlString ret;
	CIniFile::Data *pData = FindSection(strSection);
	if(pData)
		ret = pData->Find(strKeyName);
	if(ret.IsEmpty())
		return def;
	if(ret.CompareNoCase(L"true")==0)
		return true;
	else if(ret.CompareNoCase(L"false")==0)
		return false;
	else if(ret.CompareNoCase(L"1")==0)
		return true;
	else if(ret.CompareNoCase(L"0")==0)
		return false;
	return def;
}	

BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, LPCTSTR set)
{
	CIniFile::Data *pData = FindSection(strSection);
	if(pData==NULL)
		pData = AllocSection(strSection);
	if(pData)
	{
		pData->Set(strKeyName, set);
		Flush();
		return TRUE;
	}
	return FALSE;
}
BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, LONG set)
{
	CIniFile::Data *pData = FindSection(strSection);
	if(pData==NULL)
		pData = AllocSection(strSection);
	if(pData)
	{
		std::wstringstream ss;
		ss << set;
		pData->Set(strKeyName, ss.str().c_str());
		Flush();
		return TRUE;
	}
	return FALSE;
}
BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, double set)
{
	CIniFile::Data *pData = FindSection(strSection);
	if(pData==NULL)
		pData = AllocSection(strSection);
	if(pData)
	{
		std::wstringstream ss;
		ss.precision(30);
		ss << set;
		pData->Set(strKeyName, ss.str().c_str());
		Flush();
		return TRUE;
	}
	return FALSE;
}
BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, DWORD set)
{
	CIniFile::Data *pData = FindSection(strSection);
	if(pData==NULL)
		pData = AllocSection(strSection);
	if(pData)
	{
		std::wstringstream ss;
 		ss << set;
		pData->Set(strKeyName, ss.str().c_str());
		Flush();
		return TRUE;
	}
	return FALSE;
}

BOOL CIniFile::SetValue(LPCTSTR strSection,LPCTSTR strKeyName, BOOL set)
{
	CIniFile::Data *pData = FindSection(strSection);
	if(pData==NULL)
		pData = AllocSection(strSection);
	if(pData)
	{
		if(set)
			pData->Set(strKeyName, L"true");
		else 
			pData->Set(strKeyName, L"false");
		Flush();
		return TRUE;
	}
	return FALSE;
}

BOOL CIniFile::DelKeyName(LPCTSTR strSection,LPCTSTR strKeyName)
{
	CIniFile::Data *pData = FindSection(strSection);
	if(pData==NULL)
		return TRUE;
	BOOL ret = (pData->Del(strKeyName)?TRUE:FALSE);
	Flush();
	return ret;
}

BOOL CIniFile::DelSection(LPCTSTR strSection)
{
	CIniFile::Data *pData = FindSection(strSection);
	if(pData==NULL)
		return TRUE;
	BOOL ret = FALSE;
	std::vector<CIniFile::Data>::iterator it = mAllData.begin();
	for(it;it!=mAllData.end();it++)
	{
		if(it->mSection.CompareNoCase(strSection)==0)
		{
			ret = TRUE;
			mAllData.erase(it);
			break;
		}
	}
	if(ret)
		Flush();
	return ret;
 }
