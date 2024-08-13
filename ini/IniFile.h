#pragma once
#include <tchar.h>
#include <map>
#include <vector>
class CIniFile
{
public:
	class Data
	{
	public:
		inline CAtlString Find(CAtlString key){
			std::map<CAtlString,CAtlString>::iterator it = mData.begin();
			for(it;it!=mData.end();it++){
				if(it->first.CompareNoCase(key)==0){
					return CAtlString(it->second);
				}
			}
			return CAtlString(L"");
		}
		inline void Set(CAtlString key,CAtlString val){
			key = key.Trim();
			val = val.Trim();
			mData[key] = val;
		}
		inline bool Del(CAtlString key){
			key = key.Trim();
			std::map<CAtlString,CAtlString>::iterator it = mData.begin();
			for(it;it!=mData.end();it++){
				if(it->first.CompareNoCase(key)==0){
					 mData.erase(it);
					 return true;
				}
			}
			return false;
		}
		CAtlString mSection;
		friend CIniFile;
	private:
		std::map<CAtlString,CAtlString> mData;
	};
	void SetFilePath(LPCTSTR strFileName);
	//读取ini文件
	CAtlString GetValue(LPCTSTR strSection,LPCTSTR strKeyName, LPCTSTR def = NULL);
	LONG GetValue(LPCTSTR strSection,LPCTSTR strKeyName, LONG def = 0);
	double GetValue(LPCTSTR strSection,LPCTSTR strKeyName, double def = 0);
	DWORD GetValue(LPCTSTR strSection,LPCTSTR strKeyName, DWORD def = 0);
	bool GetValue(LPCTSTR strSection,LPCTSTR strKeyName, BOOL def = 0);
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
	void Flush();
	std::string Mid(const std::string& val,char a,char b);
	size_t Split(const std::string& src,char delim,std::vector<std::string>& ret);
	CIniFile::Data* FindSection(CAtlString name);
	CIniFile::Data* AllocSection(CAtlString name);
	void MakeData();
private:
	CAtlString m_strFileName;
	std::vector<CIniFile::Data> mAllData;
};
