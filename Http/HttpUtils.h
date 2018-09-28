#pragma once
#include <string>
#include <map>
#include <sstream>
namespace curl{
	class CHttpClient;
}
namespace HttpUtils
{
	std::string GetFollowLocation(const std::string& header);
	std::string GetSession(const std::string& val);
	class CookieManager
	{
	public:
		void set(curl::CHttpClient *client);
		void push(const std::string& key,const std::string& val);
		void clear();
		std::string hostSession();
		std::string getSession(const std::string& val);
		std::string query(const std::string& key);
	protected:

	private:
		typedef std::map<std::string,std::string> COOKIE;
		COOKIE m_cookie;
		std::string m_hostCookie;
	};
}