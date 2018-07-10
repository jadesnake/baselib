#include "stdafx.h"
#include "StringUtils.h"
namespace base
{
	size_t SplitBy(const std::string& src,char delim,std::vector<std::string> &ret)
	{
		size_t sz = src.size();
		std::string tmp;
		size_t last = 0;  
		size_t index=src.find_first_of(delim,last);  
		if(index==std::string::npos)
		{
			return 0;
		}
		ret.clear();
		while (index!=std::string::npos)  
		{
			ret.push_back(src.substr(last,index-last));  
			last=index+1;  
			index=src.find_first_of(delim,last);  
		}  
		if (index-last>0)  
		{  
			ret.push_back(src.substr(last,index-last));  
		}
		return ret.size();
	}
}