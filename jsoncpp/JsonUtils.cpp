#include "JsonUtils.h"
namespace JsonUtils{
	std::string FmtJson(const Json::Value &doc)
	{
		Json::FastWriter writer;
		std::string ret = writer.write(doc);
		if(ret.size() && ret[ret.length()-1]=='\n')
			ret = ret.substr(0,ret.length()-1);
		return ret;
	}
	bool ParseJson(const std::string& v,Json::Value &doc)
	{
		Json::Reader jsR;
		if(v.empty())
			return false;
		bool bRet = false;
		try
		{
			bRet = jsR.parse(v,doc);
		}
		catch(Json::Exception &e)
		{
			bRet = false;
		}
		return bRet;
	}
	long SafeJsonValueINT(const Json::Value &doc,const std::string& k)
	{
		long ret = 0;
		try{
			if(doc.isNull())
				return ret;
			if(doc[k].isNull())
				return ret;
			else if(doc[k].isInt())
				ret = doc[k].asInt();
			else if(doc[k].isDouble())
				ret = (long)doc[k].asDouble();
			else if(doc[k].isBool())
			{
				if(doc[k].asBool())
					ret = 1;
				else
					ret = 0;
			}
		}
		catch(Json::Exception &e){

		}
		return ret;
	}
	bool SafeJsonValueBOOL(const Json::Value &doc,const std::string& k)
	{
		bool ret = false;
		try{
			if(doc.isNull())
				return ret;
			if(doc[k].isNull())
				return ret;
			if(doc[k].isString())
			{
				if(doc[k].asString()=="yes" || doc[k].asString()=="true")
					ret = true;
			}
			else if(doc[k].isInt())
			{
				if(doc[k].asInt())
					ret = true;
			}
			else if(doc[k].isBool())
			{
				ret = doc[k].asBool();
			}
		}
		catch(Json::Exception &e){

		}
		return ret;
	}
	std::string SafeJsonValue(const Json::Value &doc,const std::string& k)
	{
		std::stringstream ss;
		std::string ret;
		try{
			if(doc.isNull())
				return ret;
			if(doc[k].isNull())
				return ret;
			if(doc[k].isString())
				ss << doc[k].asCString();
			else if(doc[k].isInt())
				ss << doc[k].asInt();
			else if(doc[k].isObject() || doc[k].isArray())
			{
				Json::FastWriter fw;
				ss << fw.write(doc[k]).c_str();
			}
			else if(doc[k].isDouble())
				ss << doc[k].asDouble();
			else if(doc[k].isBool())
			{
				if(doc[k].asBool())
					ss << "true";
				else
					ss << "false";
			}
			ret = ss.str();
		}
		catch(Json::Exception &e){
			
		}
		return ret;
	}
	std::string SafeJsonValueStr(const std::string &json,const std::string& k)
	{
		std::string ret;
		Json::Value doc;
		if(!ParseJson(json,doc))
			return ret;
		ret = SafeJsonValue(doc,k);
		return ret;
	}
}
