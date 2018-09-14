#pragma once

#include <string>
#include <vector>
#include <map>
namespace base{
	size_t SplitBy(const std::string& src,char delim,std::vector<std::string> &ret);
	std::string GetJsVarStr(const char* var,const std::string& js);
	std::vector<std::string> GetJsFunParams(const char* jsFun);
	std::map<std::string,std::string> ParseUrlGetParams(const char* url);
	void Replace(std::string &strBase,std::string strSrc,std::string strDes);
}
