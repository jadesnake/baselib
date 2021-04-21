#pragma once
#include "inc/json.h"
#include <sstream>
namespace JsonUtils{
	/**
	json格式化成字符串

	@author jiayh

	@inParam doc json对象

	@outParam 无

	@throw 无

	@return std::string 结果

	@since 1.0
	**/
	std::string FmtJson(const Json::Value &doc);
	/**
	解析json字符串至json对象

	@author jiayh

	@inParam v json字符串

	@outParam doc json对象

	@throw 无

	@return bool true-成功，false-失败

	@since 1.0
	**/
	bool ParseJson(const std::string& v,Json::Value &doc);
	/**
	安全获取json对象内key值

	@author jiayh

	@inParam doc json对象
	@inParam k json对象key关键字

	@outParam 无

	@throw 无

	@return string key值的字符串

	@since 1.0
	**/
	std::string SafeJsonValue(const Json::Value &doc,const std::string& k);
	long SafeJsonValueINT(const Json::Value &doc,const std::string& k);
	bool SafeJsonValueBOOL(const Json::Value &doc,const std::string& k);
	std::string SafeJsonValueStr(const std::string &json,const std::string& k);
};