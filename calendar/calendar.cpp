// calendar.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "calendar.h"
#include <sys/timeb.h>
#include <atlcomtime.h>

namespace base
{

	Calendar::Calendar() throw()
	{

	}
	Calendar::Calendar( const ATL::CTime & tm )
		:ATL::CTime(tm.GetTime())
	{

	}
	Calendar::Calendar( __time64_t time )
		: ATL::CTime(time)
	{

	}
	Calendar::Calendar( int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,int nDST)
		: ATL::CTime(nYear,nMonth,nDay,nHour,nMin,nSec,nDST)
	{

	}
	Calendar::Calendar( WORD wDosDate, WORD wDosTime, int nDST)
		: ATL::CTime(wDosTime,wDosTime,nDST)
	{

	}
	Calendar::Calendar( const SYSTEMTIME& st, int nDST)
		: ATL::CTime(st,nDST)
	{

	}
	Calendar::Calendar( const FILETIME& ft, int nDST)
		: ATL::CTime(ft,nDST)
	{

	}
	int Calendar::GetMonthDays()
	{
		return Calendar::GetMonthDays(ATL::CTime::GetYear(), ATL::CTime::GetMonth());
	}
	int Calendar::GetMonthDays(int y,int m)
	{
		int nRet = 0;
		static UINT dayNum[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
		if (2 == m)
		{
			nRet = (((0 == y % 4) && (0 != y % 100) || (0 == y % 400)) ? 29 : 28);
		}
		else
		{
			nRet = dayNum[m - 1];
		}
		return nRet;
	}
	int Calendar::DayOfWeek(int y,int m,int d)
	{
		//注意：在公式中有个与其他公式不同的地方：
		//把一月和二月看成是上一年的十三月和十四月，例：如果是2004 - 1 - 10则换算成：2003 - 13 - 10。
		//以公元元年为参考，公元元年1月1日为星期一
		//W= (d+2*m+3*(m+1)/5+y+y/4-y/100+y/400) mod 7 
		if (m == 1 || m == 2) {
			m += 12;
			y--;
		}
		int iWeek = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
		iWeek += 1;	//0 -- 周一；7 -- 星期日
		return iWeek;
	}
	Calendar& Calendar::operator=( const ATL::CTime& time )
	{
		ATL::CTime *superTM = static_cast<ATL::CTime*>(this);
		(*superTM) = time.GetTime();
		return (*this);
	}
	Calendar& Calendar::operator=( const Calendar& time)
	{
		ATL::CTime *superTM = static_cast<ATL::CTime*>(this);
		(*superTM) = time.GetTime();
		return (*this);
	}
	int Calendar::GetDayOfWeekByChina()
	{
		if( ATL::CTime::GetDayOfWeek()==1 )
			return 7;
		return (ATL::CTime::GetDayOfWeek()-1);
	}
	bool Calendar::IsValid()
	{
		return (GetTime()>0);
	}
	bool Calendar::PlusYear(int v)
	{
		int nSelV = GetYear()+v;
		if( nSelV < 1900 )
			return false;
		base::Calendar canlendarTmp(nSelV,GetMonth(),GetDay(),GetHour(),GetMinute(),GetSecond());
		*this= canlendarTmp;
		return true;
	}
	time_t Calendar::CurrentTimeLarge()
	{
		long long time_last;    
		time_last = time(NULL);     //总秒数  
		struct __timeb64 t1;    
		_ftime(&t1);
		return (t1.time * 1000 + t1.millitm);
	}
	bool Calendar::MinusMonths(int v)
	{
		int nY = 0;	//翻的年数
		int nM = 0;	//翻的月数
		if(v>=12)
		{
			nY = v/12;
			nM = v%12;
		}
		else
			nM = v;
		int nTarY = GetYear() - 1;
		int nTarM = GetMonth() + 12 - nM;
		if(nTarM > 12)
		{
			nTarY += 1;
			nTarM -= 12;
		}
		nTarY -= nY;
		if( nTarY < 1900 )
			return false;
		base::Calendar canlendarTmp(nTarY,nTarM,GetDay(),GetHour(),GetMinute(),GetSecond());
		*this = canlendarTmp;
		return true;
	}
	bool Calendar::MinusDays(int v)
	{
		*this -= ATL::CTimeSpan(v,0,0,0);
		return true;
	}
	bool Calendar::PlusMonth(int v)
	{
		int nowDay = GetDay();
		int nOldDays = GetMonthDays(GetYear(),GetMonth());
		int nY = 0;	//翻的年数
		int nM = 0;	//翻的月数
		if(v>=12)
		{
			nY = v/12;
			nM = v%12;
		}
		else
			nM = v;
		int nSelV = GetMonth()+nM;
		if( nSelV<=0 )
		{
			nY -= 1;
			nSelV = 12 + nSelV;
		}
		else if( nSelV>12 )
		{
			nY += 1;
			nSelV -= 12;
		}
		if( (GetYear()+nY) < 1900 )
			return false;
		int nNewDays = GetMonthDays(GetYear()+nY,nSelV);
		if(nowDay>nNewDays)
			nowDay = nNewDays;
		base::Calendar canlendarTmp(GetYear()+nY,nSelV,nowDay,GetHour(),GetMinute(),GetSecond());
		*this = canlendarTmp;
		return true;
	}
	bool Calendar::ParseDateTime(LPCTSTR lpszDate)
	{
		COleDateTime oleDt;
		if( oleDt.ParseDateTime(lpszDate) )
		{
			(*this) = ATL::CTime(oleDt.m_dt);
			return true;
		}
		return false;
	}
	std::string Calendar::FormatDateTime(const SYSTEMTIME systm,const std::string& fmt)
	{
		time_t curTM = time(0);
		struct tm tmNow = *localtime(&curTM);
		std::stringstream rRet;
		std::string curYear,curMonth,curDay,curHour,curMin,curSec;
		{
			std::stringstream ss;
			ss << systm.wYear;
			curYear = ss.str();
		}
		{
			std::stringstream ss;
			ss << systm.wMonth;
			curMonth = ss.str();
		}
		{
			std::stringstream ss;
			ss << systm.wDay;
			curDay = ss.str();
		}
		{
			std::stringstream ss;
			ss << systm.wHour;
			curHour = ss.str();
		}
		{
			std::stringstream ss;
			ss << systm.wMinute;
			curMin = ss.str();
		}
		{
			std::stringstream ss;
			ss << systm.wSecond;
			curSec = ss.str();
		}
 		std::string *pVal = 0;
		std::string temp;
		for(size_t t=0;t<=fmt.size();t++)
		{
			char f = fmt[t];
			if(t && f!=fmt[t-1])
			{
				if(fmt[t-1]=='y')
					pVal = &curYear;
				else if(fmt[t-1]=='M')
					pVal = &curMonth;
				else if(fmt[t-1]=='d')
					pVal = &curDay;
				else if(fmt[t-1]=='h')
					pVal = &curHour;
				else if(fmt[t-1]=='m')
					pVal = &curMin;
				else if(fmt[t-1]=='s')
					pVal = &curSec;
				else
				{
					temp += f;
					continue;
				}
				int cz = (int)temp.size()-(int)pVal->size();
				if(cz>=0)
				{	//补0
					for(int n=0;n<cz;n++)
						rRet << "0";
					rRet << pVal->c_str();
				}
				else
				{
					for(int n=pVal->size()+cz;n<pVal->size();n++)
						rRet << pVal->at(n);
				}
				temp = "";
				if(f==' ' || f=='-' || f==':')
					rRet << f;
				else
					temp += f;
				continue;
			}
			temp += f;
		}
		return rRet.str();
	}
	SYSTEMTIME Calendar::ParseDateTime(const std::string& dt,const std::string& temp)
	{
		SYSTEMTIME sysTM;
		memset(&sysTM,0,sizeof(SYSTEMTIME));
		if(temp.size()==0)	return sysTM;
		std::string year;
		std::string month;
		std::string day;
		std::string hour;
		std::string minut;
		std::string second;	
		int npos = 0;
		for(size_t t=0;t<dt.size();t++)
		{
			char v = dt[t];
			if( t > (temp.length()-1) )	break;
			char f = temp.at(t);
			if(f=='y')
				year += v;
			else if(f=='M')
				month += v;
			else if(f=='m')
				minut += v;
			else if(f=='d')
				day += v;
			else if(f=='h')
				hour += v;
			else if(f=='s')
				second += v;
			if(v==f)
			{
				npos += 1;
				continue;
			}
		}
		if(year.size())
			sysTM.wYear = atol(year.c_str());
		if(month.size())
			sysTM.wMonth= atol(month.c_str());
		if(day.size())
			sysTM.wDay  = atol(day.c_str());
		if(hour.size())
			sysTM.wHour  = atol(hour.c_str());
		if(hour.size())
			sysTM.wMinute  = atol(minut.c_str());
		if(second.size())
			sysTM.wSecond = atol(second.c_str());
		if(sysTM.wDay==0)
			sysTM.wDay=1;
		return sysTM;
	}

}
