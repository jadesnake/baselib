#pragma once
#include <string>
#include <vector>
namespace URL
{
	std::string Encode(const std::string& str);
	std::string Decode(const std::string& str);
};