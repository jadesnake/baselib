#include "stdafx.h"
#include "StringUtils.h"
#include <sstream>
#include <Shellapi.h>
#pragma comment(lib,"Shell32.lib")
namespace base{
	std::string MakeLower(std::string& val)
	{
		std::transform(val.begin(), val.end(), val.begin(), ::tolower);
		return val;
	}
	std::string MakeUpper(std::string& val)
	{
		std::transform(val.begin(), val.end(), val.begin(), ::toupper);
		return val;
	}
	//-------------------------------------------------------------------------------------------------
	int IsTextGBK(const std::string& txt)
	{
		unsigned char chr;
		DWORD nBytes=0;//GBK2个字符编码
		bool bAllAscii=true; //如果全部都是ASCII, 说明不是UTF-8
		for(int i=0;i<txt.length();i++)
		{
			chr=txt[i];
			if( (chr&0x80) != 0 ) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
				bAllAscii= FALSE;
			if(nBytes==0) //如果不是ASCII码,应该是多字节符,计算字节数
			{	//符号区
				if(chr>=0xa1 && chr<=0xa9)
					nBytes=1;
				//文字区
				else if(chr>=0xb0 && chr<=0xF7)
					nBytes=2;
			}
			else if(nBytes==1)
			{
				if(chr<=0xef)
					return true;
			}
			else if(nBytes==2)
			{
				if(chr<=0xfe)
					return true;
			}
		}
		return false;
	}
	//utf8-3编码范围
	unsigned char utf8_3[][2] ={{0xA0,0xBF},{0x80,0xBF},{0x80,0x9F},{0x80,0xBF}};
	//utf8-4编码范围
	unsigned char utf8_4[][2][2] ={ {{0x90,0xBF},{0x80,0xBF}},{{0x80,0xBF},{0x80,0xBF}},{{0x80,0x8F},{0x80,0xBF}} };
	int IsTextUTF8(const std::string& txt)
	{
		DWORD nBytes=0;//UFT8可用1-6个字节编码,ASCII用一个字节
		unsigned char chr;
		bool bAllAscii=true; //如果全部都是ASCII, 说明不是UTF-8
		unsigned char chTxt[2] = {0, 0}; //utf8中文编码为3个字符，如果是2个字符判断一波gbk
		bool bCheckGBK= false;
		char tbn = '0'; //表类型
		unsigned int sIndex = 0; //子索引
		long long nValid=0;	//有效utf8字符数
		//缺失，5，6编码范围
		for(int i=0;i<txt.length();i++)
		{
			chr=txt[i];
			if( (chr&0x80) != 0 ) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
				bAllAscii= FALSE;
			if(nBytes==0) //如果不是ASCII码,应该是多字节符,计算字节数
			{
				bCheckGBK = false;
				if(chr>=0x80)
				{
					if(chr>=0xFC&&chr<=0xFD)
					{
						tbn = '6';
						chTxt[0] = chr;
						nBytes=6;
					}
					else if(chr>=0xF8)
					{
						tbn = '5';
						chTxt[0] = chr;
						nBytes=5;
					}
					if(chr==0xF0)
					{
						tbn = '4';
						sIndex = 0;
						chTxt[0] = chr;
						nBytes=4;
					}
					else if(chr>=0xF1&&chr<=0xF3)
					{
						tbn = '4';
						sIndex = 1;
						chTxt[0] = chr;
						nBytes=4;
					}
					else if(chr==0xF4)
					{
						tbn = '4';
						sIndex = 2;
						chTxt[0] = chr;
						nBytes=4;
					}
					else if(chr==0xE0)
					{
						tbn = '3';
						sIndex = 0;
						chTxt[0] = chr;
						nBytes=3;
					}
					else if(chr>=0xE1&&chr<=0xEC)
					{
						tbn = '3';
						sIndex = 1;
						chTxt[0] = chr;
						nBytes=3;
					}
					else if(chr==0xED)
					{
						tbn = '3';
						sIndex = 2;
						chTxt[0] = chr;
						nBytes=3;
					}
					else if(chr>=0xEE&&chr<=0xEF)
					{
						tbn = '3';
						sIndex = 3;
						chTxt[0] = chr;
						nBytes=3;
					}
					else if(chr>=0xC2 && chr<=0xDF)
					{
						tbn = '2';
						nBytes=2;
						chTxt[0] = chr;
						bCheckGBK = true;
					}
					else
					{
						return false;
					}
					nBytes--;
				}
			}
			else //多字节符的非首字节,应为 10xxxxxx
			{
				chTxt[1] = chr;
				if( (chr&0xC0) != 0x80 )
				{
					return false;
				}
				if(nBytes==1&& (chr>=0x80&&chr<=0xBF))
				{
					//屏蔽 “豫”D4 A5
					if(0xD4==chTxt[0] && 0xA5==chTxt[1])
						return false;
					nBytes--;
					continue;
				}
				else if(tbn=='4' && nBytes && chr>=utf8_4[sIndex][3-nBytes][0] && chr<=utf8_4[sIndex][3-nBytes][1])
				{
					nBytes--;
					continue;
				}
				else if(tbn=='3' && nBytes && chr>=utf8_3[sIndex][0] && chr<=utf8_3[sIndex][1])
				{
					nBytes--;
					continue;
				}
				if(bCheckGBK)
				{
					if(IsTextGBK(std::string((char*)chTxt,2)));
						return false;
				}				
			}
		}
		if( nBytes > 0 ) //违返规则
		{
			return false;
		}
		if( bAllAscii ) //如果全部都是ASCII, 说明不是UTF-8
		{
			return false;
		}
		return true;
	}
	//-------------------------------------------------------------------------------------------------
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
		for(size_t t=0;t<(ret.size()-1);t++)
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
		for(size_t t=0;t<(ret.size()-1);t++)
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
	KeyCmd GetCmdForHash(wchar_t spliter)
	{
		KeyCmd ret;
		int nNumArgs = 0;
		LPWSTR *szArglist = CommandLineToArgvW(::GetCommandLineW(),&nNumArgs);	
		if (nNumArgs == 1)
			return	ret;
		std::wstring cmd;	//当前指令
		for (int n = 1; n < nNumArgs;n++)
		{
			std::wstring key,val;
			LPWSTR nxtArg = szArglist[n];
			wchar_t *pSp = wcschr(nxtArg,spliter);
			if(pSp)
			{
				key.append(nxtArg,pSp-nxtArg);
				val.append(pSp+1);
			}
			else
				key = nxtArg;
			if(cmd.empty())
				cmd = key;
			ret[key] = val;
		}
		ret[L"cmdkey"] = cmd;
		::LocalFree(szArglist);
		return ret;
	}
}