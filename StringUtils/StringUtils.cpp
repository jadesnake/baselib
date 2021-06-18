#include "stdafx.h"
#include "StringUtils.h"
namespace base{
	bool IsFullNumberW(const std::wstring&str)
	{
		bool bRet = true;
		for (int nPos =0;nPos < str.length();nPos++)
		{
 			if(!isdigit(str[nPos]))
			{
				bRet = false;
				break;
			}
		}
		return bRet;
	}

	std::string JoinA(char delim,const std::vector<std::string> &ret)
	{
		std::stringstream ss;
		size_t sEnd = ret.size();
		if(sEnd==0)
			return "";
		sEnd -= 1;
		for(size_t t=0;t<ret.size();t++)
			ss << ret[t].c_str() << delim;
		ss << ret[sEnd].c_str();
		return ss.str();
	}
	std::wstring JoinW(char delim,const std::vector<std::wstring> &ret)
	{
		std::wstringstream ss;
		size_t sEnd = ret.size();
		if(sEnd==0)
			return L"";
		sEnd -= 1;
		for(size_t t=0;t<ret.size();t++)
			ss << ret[t].c_str() << delim;
		ss << ret[sEnd].c_str();
		return ss.str();
	}
	//-------------------------------------------------------------------------------------------------
	size_t SplitByW(const std::wstring& src,char delim,std::vector<std::wstring> &ret)
	{
		size_t sz = src.size();
		std::wstring tmp;
		size_t last = 0;  
		size_t index=src.find_first_of(delim,last);  
		if(index==std::wstring::npos)
		{
			ret.clear();
			if(!src.empty())
				ret.push_back(src);
			return ret.size();
		}
		ret.clear();
		while (index!=std::wstring::npos)  
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
	size_t SplitBy(const std::string& src,char delim,std::vector<std::string> &ret)
	{
		size_t sz = src.size();
		std::string tmp;
		size_t last = 0;  
		size_t index=src.find_first_of(delim,last);  
		if(index==std::string::npos)
		{
			ret.clear();
			if(!src.empty())
				ret.push_back(src);
			return ret.size();
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
	//-------------------------------------------------------------------------------------------------
	void Replace(std::string &strBase,std::string strSrc,std::string strDes)
	{
		std::string::size_type pos = 0;  
		std::string::size_type srcLen = strSrc.size();  
		std::string::size_type desLen = strDes.size();  
		pos=strBase.find(strSrc, pos);   
		while((pos != std::string::npos))  
		{  
			strBase.replace(pos, srcLen, strDes);  
			pos = strBase.find(strSrc, (pos+desLen));  
		}  
	}
	//-------------------------------------------------------------------------------------------------
	std::map<std::string,std::string> ParseUrlGetParams(const char* url)
	{
		std::map<std::string,std::string> ret;
		const char *p = strchr(url,'?');
		if(p==0)			return ret;
		int szParam = strlen(url) - (url-p);
		std::string params(p+1,szParam-1);
		std::vector<std::string> splits;
		if(params.length()&&0==SplitBy(params,'&',splits))
			splits.push_back(params);
		for(size_t s=0;s<splits.size();s++)
		{
			std::vector<std::string> vals;
			if(2==SplitBy(splits[s],'=',vals))
				ret[vals[0]] = vals[1];
		}
		return ret;
	}
	//-------------------------------------------------------------------------------------------------
	std::vector<std::string> GetJsFunParams(const char* jsFun)
	{
		std::vector<std::string> ret;
		const char *p1= strchr(jsFun,'(');
		const char *p2= strrchr(jsFun,')');
		if(p1==NULL||p2==NULL)
			return ret;
		bool bStr = false;
		char chFlag = 0;
		std::string exp(p1+1,p2-p1-1);
		std::string tmp;
		exp += ',';
		for(size_t s=0;s<exp.length();s++)
		{
			if(bStr==false&&(exp[s]=='"'||exp[s]=='\'') )
			{
				chFlag = exp[s];
				bStr = true;
				continue;
			}
			if(bStr&&chFlag==exp[s])
			{
				chFlag = 0;
				bStr = false;
				continue;
			}
			if(exp[s]==',')
			{
				ret.push_back(tmp);
				tmp.clear();
				continue;
			}
			tmp += exp[s];
		}
		return ret;
	}
	//-------------------------------------------------------------------------------------------------
	std::string GetJsVarStr(const char* var,const std::string& js)
	{
		std::string ret,line;
		std::stringstream ss(js);
		while(std::getline(ss,line))
		{
			std::vector<std::string> exp;
			if(2==base::SplitBy(line,'=',exp))
			{
				if(-1!=exp[0].find(var))
				{
					bool bvarVal=false;
					std::string varVal;
					char chFlag = 0;
					for(size_t s=0;s<exp[1].length();s++)
					{
						if(exp[1][s]=='"'||exp[1][s]=='\'')
						{
							if(bvarVal==false)
							{
								bvarVal = true;
								chFlag = exp[1][s];
								continue;
							}
							else if(chFlag==exp[1][s])
							{
								bvarVal = false;
								break;
							}
						}
						if(bvarVal)
						{
 							if(exp[1][s]=='\\')
							 s+=1;
							varVal += exp[1][s];
						}
					}
					ret = varVal;
					break;
				}
			}
		}
		return ret;
	} 
}