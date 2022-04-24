// calendar.cpp : 定义控制台应用程序的入口点。
//
#pragma once
#include <atltime.h>

namespace base{

	class Calendar : public ATL::CTime
	{
	public:
		Calendar() throw();
		Calendar( __time64_t time ) throw();
		Calendar( const ATL::CTime & tm ) throw();
		Calendar( int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,int nDST = -1 );
		Calendar( WORD wDosDate, WORD wDosTime, int nDST = -1 );
		Calendar( const SYSTEMTIME& st, int nDST = -1 );
		Calendar( const FILETIME& ft, int nDST = -1 );
		//获取中国习惯的星期几
		//周一 1 —— 周日 7
		int GetDayOfWeekByChina();
		bool IsValid();
		Calendar& operator=( const ATL::CTime& time );
		Calendar& operator=( const Calendar& time);
		bool PlusYear(int v);
		bool PlusMonth(int v);
		bool MinusMonths(int v);
		bool MinusDays(int v);
		//获取当前时间精确到毫秒
		static time_t CurrentTimeLarge();
		bool ParseDateTime(LPCTSTR lpszDate);
		//dt - 待解析数据
		//temp - 日期模板 可选值：
		//				yyyy-MM-dd hh:mm:ss
		//				yy-MM-dd hh:mm:ss
		//				hh:mm:ss
		//				yyyy-MM-dd
		//				yy-MM-dd
		//				yyyyMMdd
		//	
		static SYSTEMTIME ParseDateTime(const std::string& dt,const std::string& temp);
		static std::string FormatDateTime(const SYSTEMTIME systm,const std::string& fmt);
	public:
		//获取当前月天数
		int GetMonthDays();
		//获取某年某月天数
		static int GetMonthDays(int y,int m);
		//基于基姆拉尔森计算公式
		//@param y 年份 如：1990年
		//@param m 月份 如：1月
		//@param d 日期 如：20号
		static int DayOfWeek(int y,int m,int d);
	};
}