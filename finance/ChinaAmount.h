#pragma once

namespace ChinaAmount
{
	//rmb 小写转大写
	bool CapitalRMB(const CAtlString &inRmb,CAtlString &out);
	//数字小写转大写
	CAtlString CapitalNumber(const CAtlString &inNum);
	//单位转数值
	UINT Unit2Number(const CAtlString& unit);
};