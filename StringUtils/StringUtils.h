#pragma once

#include <string>
#include <vector>
#include <map>
namespace base{
	size_t SplitBy(const std::string& src,char delim,std::vector<std::string> &ret);
	size_t SplitByW(const std::wstring& src,char delim,std::vector<std::wstring> &ret);

	std::map<std::string,std::string> ParseUrlGetParams(const char* url);

	void Replace(std::string &strBase,std::string strSrc,std::string strDes);

	std::string JoinA(char delim,const std::vector<std::string> &ret);
	std::wstring JoinW(char delim,const std::vector<std::wstring> &ret);
}
