#pragma once

#include <string>
#include <vector>
#include <map>
namespace base{
	typedef std::map<std::wstring,std::wstring> KeyCmd;
	KeyCmd GetCmdForHash(wchar_t spliter);

	int IsTextUTF8(const std::string& txt);
	int IsTextGBK(const std::string& txt);

	bool IsFullNumberW(const std::wstring&str);

	size_t SplitBy(const std::string& src,char delim,std::vector<std::string> &ret);
	size_t SplitByW(const std::wstring& src,char delim,std::vector<std::wstring> &ret);
	
	std::string JoinA(char delim,const std::vector<std::string> &ret);
	std::wstring JoinW(char delim,const std::vector<std::wstring> &ret);

	std::map<std::string,std::string> ParseUrlGetParams(const char* url);
	
	void Replace(std::string &strBase,std::string strSrc,std::string strDes);
 
	template<class _STR>
	void SetVecOrder(std::vector<_STR> &ret,bool bLarger=true)
	{
		std::map<_STR,int> map_ret;
		for (size_t n=0;n<ret.size();n++)
			map_ret.insert(std::make_pair(ret[n],0));
		std::vector<_STR> vT;
		std::map<_STR,int>::iterator iter = map_ret.begin();
		for (;iter!=map_ret.end();iter++)
			vT.push_back(iter->first);
		if(!bLarger)
		{	
			ret.clear();
			ret = vT;
		}
		else
		{
			ret.clear();
			for(size_t n=0;n<vT.size();n++)
				ret.push_back(vT[vT.size()-n-1]);
		}
	}
}