#include "stdafx.h"
#include "HttpUtils.h"
#include "HttpClient.h"
namespace HttpUtils
{
	std::string findByKey(const std::string& inC,const char* key)
	{
		std::string ret;
		int nPosSession = inC.find(key);
		int nPosEnd = inC.find(";",nPosSession);
		if(nPosSession>0)
		{
			char h = inC.at(nPosSession-1);
			char e = inC.at(nPosSession+strlen(key));
			if(h!=' ')
				return ret;
			if(e!=' ' && e!='=')
				return ret;
		}
		if(nPosSession!=-1 && nPosEnd!=-1)
			ret = inC.substr(nPosSession,nPosEnd-nPosSession);
		return ret;
	}

	std::string GetFollowLocation(const std::string& header)
	{
		std::string keyword("location");
		std::stringstream ss(header);
		std::string line,ret;
		while(std::getline(ss,line))
		{
			std::string t1 = line.substr(0,keyword.length());
			std::transform(t1.begin(), t1.end(), t1.begin(), ::tolower);
			if(t1!=keyword)
				continue;
			ret = line.substr(keyword.length()+1);
			break;
		}
		return ret;
	}
	std::string GetSession(const std::string& val)
	{
		std::string ret;
		ret = findByKey(val,"SESSION");
		if(!ret.empty())
			return ret;
		ret = findByKey(val,"session");
		if(!ret.empty())
			return ret;
		ret = findByKey(val,"JSESSIONID");
		if(!ret.empty())
			return ret;
		ret = findByKey(val,"jsessionid");
		if(!ret.empty())
			return ret;
		return ret;
	}
	std::string GetRqUrlHost(const std::string& url)
	{
		std::string tmp;
		size_t offset=0;
		size_t pos = 0;
		pos = url.find("//");
		if(pos!=-1)
		{
			offset += pos + 2;
			tmp = url.substr(offset);
		}
		pos = tmp.find("/");
		if(pos==-1)	return url;
		offset += pos;
		tmp = url.substr(0,offset);
		return tmp;
	}
	//从请求种获取url
	//去掉请求参数
	//去掉请求资源
	std::string GetRqUrl(const std::string& url)
	{
		size_t pos = 0;
		std::string tmp = GetRqUrlHost(url);
		size_t offset=tmp.length();
		tmp = url.substr(offset);
		pos = tmp.find("/");
		if(pos==-1)	return url;
		offset += pos;
		//rqUrl去掉资源
		tmp = tmp.substr(pos);
		pos = tmp.find("?");
		if(pos!=-1)
			tmp = tmp.substr(0,pos-1);
		pos = tmp.find(".");
		if(pos!=-1)
		{
			tmp = tmp.substr(0,pos-1);
			pos = tmp.rfind("/");
			tmp = tmp.substr(0,pos);
		}
		tmp = url.substr(0,offset+tmp.length());
		return tmp;
	}
	void CookieManager::push(const std::string& key,const std::string& val)
	{
		if(key.empty()) return;
		std::string param = key;
		char end = param.at(param.length()-1);
		if(end=='/' || end=='\\')
			param.erase(param.length()-1,1);
		if(m_cookie.size()==0)	m_hostCookie = val;
		m_cookie[param] = val;
	}
	std::string CookieManager::hostSession()
	{
		return GetSession(m_hostCookie);
	}
	std::string CookieManager::query(const std::string& key)
	{
		if(key.empty())	return "";
		if(m_cookie.size()==0) return ""; 
		COOKIE::iterator it = m_cookie.begin();
		std::string first;
		//采用最优匹配
		for(it;it!=m_cookie.end();it++)
		{
			size_t pos = key.find(it->first);
			if(pos==-1)	continue;
			if(it->first.length() > first.length())
				first = it->first;			
		}
		if(first.empty())	return ""; 
		return m_cookie[first];
	}
	void CookieManager::clear()
	{
		m_cookie.clear();
	}
	std::string CookieManager::getSession(const std::string& val)
	{
		return GetSession(query(val));
	}
	void CookieManager::set(curl::CHttpClient *client)
	{
		if(client==NULL)	return ;
		if(client->GetCookie().empty())	return ;
		std::string rqUrl = GetRqUrlHost(client->m_rqUrl);
		std::string cookie = client->GetCookie();
		std::transform(cookie.begin(),cookie.end(),cookie.begin(),::tolower);
		size_t pos = cookie.rfind("path");
		if(pos!=-1)
		{
			//获取path主键
			std::string pathKey = client->GetCookie().substr(pos,4);
			std::string path = findByKey(client->GetCookie().substr(pos).c_str(),pathKey.c_str());
			if(!path.empty())
				rqUrl += path.substr(path.find("=")+1);
		}
		push(rqUrl,client->GetCookie());
	}
}