#include "stdafx.h"
#include "ChinaAmount.h"

namespace base
{
	bool ChinaAmount::CapitalRMB(const CAtlString &inRmb,CAtlString &out)
	{
		static TCHAR CAPITAL_NUM[]=_T("零壹贰叁肆伍陆柒捌玖");
		static TCHAR CAPITAL_UNIT[]=_T("分角元拾佰仟万亿");

		CAtlString integer;	//整数部分
		CAtlString decimal;	//小数部分
		int dot = inRmb.ReverseFind('.');
		if(inRmb.IsEmpty())
			return false;
		if(dot==-1 || dot==(inRmb.GetLength()-1) )
		{
			integer = inRmb;
		}
		else
		{
			integer = inRmb.Mid(0,dot);
			decimal = inRmb.Mid(dot+1);
		}
		//去掉整数部分的人民币标识符
		integer.Remove('￥');
		//超出转换范围
		if(decimal.GetLength()>2)
			return false;
		if(integer.GetLength()>10)
			return false;
		//去掉小数部分尾部的0值
		if( !decimal.IsEmpty() )
		{
			if(decimal==_T("00"))
				decimal.Empty();
			else if(decimal==_T("0"))
				decimal.Empty();
			else if( '0'==decimal[decimal.GetLength()-1] )
				decimal.Delete(decimal.GetLength()-1);
		}
		CAtlString tmp1,tmp2;
		int nUnit = 1;		//单位索引
		//处理小数部分
		for(int nNum=0,nUnit=1;nNum<decimal.GetLength();nNum++,nUnit--)
		{
			int price = decimal[nNum] - _T('0');
			if(price==0)	continue;	//跳过0
			TCHAR capital = CAPITAL_NUM[price];
			TCHAR unit = CAPITAL_UNIT[nUnit];
			tmp2 += capital;
			tmp2 += unit;
		}
		for(int nNum=0,nUnit=2;nNum<integer.GetLength();nNum++)
		{
			CAtlString tmp;
			int price = integer[integer.GetLength()-nNum-1] - _T('0');
			if(price==0)	continue;	//跳过0
			TCHAR capital = CAPITAL_NUM[price];
			TCHAR unit;
			if(nNum<=5)		//亿单位
				unit = CAPITAL_UNIT[nUnit+nNum];
			else
			{	//超出亿表示范围
				int nU = nNum-5;
				unit = CAPITAL_UNIT[nUnit+nU];
			}
			tmp += capital;
			tmp += unit;
			tmp1.Insert(0,tmp);
		}
		if( tmp1[tmp1.GetLength()-1]!=_T('元') )
			tmp1 += _T("元");
		tmp2 += _T("整");
		out = tmp1 + tmp2;
		return true;
	}
}