#include "stdafx.h"
#include "CustomControl.h"
#include "../base/calendar/calendar.h"
#include "../base/MacroX.h"
#include "../base/baseX.h"
#include "../base/StringUtils/NumberConvert.h"
namespace CustomUI{
	class DayOption : public DuiLib::COptionUI
	{
	public:
		DUI_DECLARE(DayOption,DuiLib::COptionUI);

		DayOption(const ATL::CTime &tm,int nShowMonth)
		{
			m_bToday = false;
			Update(tm,nShowMonth);
			SetAttribute(_T("hotbkcolor"),_T("#FFF2F2F2"));
			SetAttribute(_T("selectedbkcolor"),_T("#FF3487E2"));
			SetAttribute(_T("align"),_T("center"));
			SetAttribute(_T("valign"),_T("vcenter"));
			SetAttribute(_T("selectedtextcolor"),_T("#FFFFFFFF"));			
			SetGroup(_T("CustomUI::Calendar::DayOption"));
		}
		void Update(const ATL::CTime &tm,int nShowMonth)
		{
			m_time = tm;
			m_nShowMonth=nShowMonth;
			UpdateAttribute();
			SetText(base::IntToString16(m_time.GetDay()).c_str());
			if( m_tmToday.IsValid() )
			{
				if( m_tmToday.GetYear()==m_time.GetYear()   && 
					m_tmToday.GetMonth()==m_time.GetMonth() &&
					m_tmToday.GetDay()==m_time.GetDay()	)
				{
					SetBorderSize(1);
					SetAttribute(_T("bordercolor"),_T("#FF3487E2"));
					SetAttribute(_T("textcolor"),_T("#FF3487E2"));
					m_bToday = true;
				}
				else if(m_bToday)
				{
					SetBorderSize(0);
					SetAttribute(_T("bordercolor"),_T("#FFFFFFFF"));
					SetAttribute(_T("textcolor"),_T("#FF323232"));
					m_bToday = false;
				}
			}			
		}
		void UpdateAttribute()
		{
			if(m_time.GetMonth()==m_nShowMonth)
			{
				//返回基于本地时间周;1 = sunday，2 = mon，为7 =星期六。
				if(1==m_time.GetDayOfWeek()||7==m_time.GetDayOfWeek())
				{
					SetAttribute(_T("textcolor"),_T("#FD6E0E"));
				}
				else
				{
					SetAttribute(_T("textcolor"),_T("#323232"));
				}
				if(m_bToday==false)
					SetAttribute(_T("bkcolor"),_T("#FFFFFF"));
			}
			else
			{
				SetAttribute(_T("textcolor"),_T("#969696"));
			}	
		}
		void SetEnabled(bool bEnable)
		{
			DuiLib::COptionUI::SetEnabled(bEnable);
			if(!bEnable)
			{
				SetAttribute(_T("bkcolor"),_T("#DEDEDE"));
				SetAttribute(_T("textcolor"),_T("#969696"));
			}
			else
			{
				UpdateAttribute();
			}
		}
		void Selected(bool bSelected)
		{
			__super::Selected(bSelected);
		}
		const ATL::CTime& GetTime()
		{
			return m_time;
		}
		void SetCurrentDay(const ATL::CTime &now)
		{
			m_tmToday = now;
			Update(m_time,m_nShowMonth);
		}
	protected:
		base::Calendar m_tmToday;
		ATL::CTime m_time;
		int		   m_nShowMonth;
		bool	   m_bToday;
	};
/*-------------------------------------------------------------------------------------------------------------------*/	
	class CalendarHeaderUI : public DuiLib::CHorizontalLayoutUI
	{
	public:
		CalendarHeaderUI()
		{

		}
		~CalendarHeaderUI()
		{

		}
		DUI_DECLARE(CalendarHeaderUI,DuiLib::CHorizontalLayoutUI);
	};
/*-------------------------------------------------------------------------------------------------------------------*/	
	class CalendarChooseUI : public DuiLib::CVerticalLayoutUI
	{
	public:
		CalendarChooseUI()
		{
			header = NULL;
			lastLayout = NULL;
			nLineHeight = 26;
		}
		~CalendarChooseUI()
		{

		}
		void SetHeader(CalendarHeaderUI *header)
		{
			this->header = header;
			Invalidate();
		}
		DayOption* AppendDay(const ATL::CTime &tm,int nShowMonth)
		{
			DayOption *ret = NULL;
			if( GetCount()==0 )
				lastLayout = new DuiLib::CHorizontalLayoutUI();
			else if( lastLayout->GetCount()==header->GetCount() )
				lastLayout = new DuiLib::CHorizontalLayoutUI();
			if( lastLayout->GetManager()==NULL )
			{
				if(nLineHeight>0)
					lastLayout->SetFixedHeight(nLineHeight);
				DuiLib::CVerticalLayoutUI::Add(lastLayout);
			}
			ret =  new DayOption(tm,nShowMonth);
			lastLayout->Add(ret);
			return ret;
		}
		DUI_DECLARE(CalendarChooseUI,DuiLib::CVerticalLayoutUI);
	protected:
		void SetPos(RECT rc)
		{
			__super::SetPos(rc);
			if(header==NULL || GetCount()==0)
				return ;
			for(int n=0;n<GetCount();n++)
			{
				DuiLib::CContainerUI *layout = base::SafeConvert<DuiLib::CContainerUI>(GetItemAt(n),DUI_CTR_CONTAINER);
				for(int s=0;s<layout->GetCount();s++)
				{
					DuiLib::CControlUI *column = header->GetItemAt(s);
					DuiLib::CControlUI *ctrl = layout->GetItemAt(s);
					if(column && ctrl)
					{
						RECT rcColumn = column->GetPos();
						RECT rcCtrl = ctrl->GetPos();
						rcCtrl.left = rcColumn.left;
						rcCtrl.right= rcColumn.right;
						ctrl->SetPos(rcCtrl);
					}
				}
			}
		}
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
		{
			if(_tcscmp(pstrName,_T("lineHeight"))==0)
			{
				base::StringToInt(pstrValue,&nLineHeight);
				return ;
			}
			DuiLib::CVerticalLayoutUI::SetAttribute(pstrName,pstrValue);
		}
	private:
		CalendarHeaderUI *header;
		DuiLib::CHorizontalLayoutUI *lastLayout;
		int nLineHeight;
	};
/*-------------------------------------------------------------------------------------------------------------------*/	
	CalendarUI::CalendarUI(HWND hParent)
		:m_hParent(hParent)
	{
		fromValid = NULL;
		toValid = NULL;
		calendarHeaderUI = NULL;
		calendarChooseUI = NULL;
		calendar = new base::Calendar();
		Create(m_hParent, NULL, WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
	}
	CalendarUI::~CalendarUI()
	{
		if(calendar)
		{
			delete calendar;
			calendar=NULL;
		}
		if( fromValid )
		{
			delete fromValid;
			fromValid = NULL;
		}
		if( toValid )
		{
			delete toValid;
			toValid = NULL;
		}
	}
	void CalendarUI::Show(const POINT &pt)
	{
		DuiLib::CDuiRect rcWnd;
		GetWindowRect(m_hWnd, &rcWnd);
		int nWidth = rcWnd.GetWidth();
		int nHeight = rcWnd.GetHeight();
		rcWnd.left = pt.x;
		rcWnd.top = pt.y;
		rcWnd.right = rcWnd.left + nWidth;
		rcWnd.bottom = rcWnd.top + nHeight;
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		DuiLib::CDuiRect rcWork = oMonitor.rcMonitor;
		if (rcWnd.bottom > rcWork.bottom)
		{
			if (nHeight >= rcWork.GetHeight()) 
			{
				rcWnd.top = 0;
				rcWnd.bottom = nHeight;
			}
			else 
			{
				rcWnd.bottom = rcWork.bottom;
				rcWnd.top = rcWnd.bottom - nHeight;
			}
		}
		if (rcWnd.right > rcWork.right)
		{
			if (nWidth >= rcWork.GetWidth()) 
			{
				rcWnd.left = 0;
				rcWnd.right = nWidth;
			}
			else 
			{
				rcWnd.right = rcWork.right;
				rcWnd.left = rcWnd.right - nWidth;
			}
		}
		if(m_hParent)
			::SendMessage(m_hParent, WM_NCACTIVATE, TRUE, 0L);
		::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(),
					   SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
		::SetForegroundWindow(m_hWnd);
		::SetFocus(m_hWnd);
	}
	void CalendarUI::InitWindow()
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	}
	DuiLib::CDuiString CalendarUI::GetSkinFolder()
	{
		return _T("");
	}
	DuiLib::CDuiString CalendarUI::GetSkinFile()
	{
		return _T("xml/calendar.xml");
	}
	LPCTSTR CalendarUI::GetWindowClassName(void) const
	{
		return _T("CalendarUIWindow");
	}
	UILIB_RESOURCETYPE CalendarUI::GetResourceType() const
	{
		return UILIB_ZIPRESOURCE;
	}
	LPCTSTR		CalendarUI::GetResourceID() const
	{
		return MAKEINTRESOURCE(IDR_SKINRES);
	}
	DuiLib::CControlUI* CalendarUI::CreateControl(LPCTSTR pstrClass)
	{
		if( 0==_tcscmp(pstrClass,_T("CalendarHeader")) )
		{
			return new CalendarHeaderUI();
		}
		if( 0==_tcscmp(pstrClass,_T("CalendarChoose")) )
		{
			return new CalendarChooseUI();
		}
		return CustomUI::CreateCustomUI(pstrClass,this);
	}
	void CalendarUI::SetValidRange(const base::Calendar &from,base::Calendar &to)
	{
		if(fromValid==NULL)
		{
			fromValid = new base::Calendar();
		}
		if(toValid==NULL)
		{
			toValid = new base::Calendar();
		}
		*fromValid = from;
		*toValid = to;
	}
	void CalendarUI::OnFinalMessage( HWND hWnd )
	{
		if( calendarChooseUI )
			calendarChooseUI->SetHeader(NULL);

		DuiLib::WindowImplBase::OnFinalMessage(hWnd);
		delete this;
	}
	void CalendarUI::SetCurrentPage(const base::Calendar &showCal)
	{
		static LPCTSTR manMonth[]={_T("一月"),_T("二月"),_T("三月"),_T("四月"),
								   _T("五月"),_T("六月"),_T("七月"),_T("八月"),
								   _T("九月"),_T("十月"),_T("十一月"),_T("十二月")};
		calendarChooseUI->RemoveAll();
		DuiLib::CControlUI *showYear = m_PaintManager.FindControl(_T("curYear"));
		DuiLib::CControlUI *showMonth = m_PaintManager.FindControl(_T("curMonth"));
		showYear->SetText( base::IntToString16(showCal.GetYear()).c_str() );
		showMonth->SetText( manMonth[showCal.GetMonth()-1] );

		//当前月一共多少天
		int nDays = base::Calendar::GetMonthDays(showCal.GetYear(),showCal.GetMonth());
		//月初
		base::Calendar monthStart(showCal.GetYear(),showCal.GetMonth(),1,8,8,8);
		//月初之前空余位天数
		int nSpaceDay = monthStart.GetDayOfWeekByChina();
		if(nSpaceDay!=7)
		{
			//采用上月余下天数填充空余头部数据
			ATL::CTimeSpan span(nSpaceDay,0,0,0);
			base::Calendar spanMonth(monthStart.GetTime());		//上个月
			spanMonth -= span;
			for(int n=0;n< nSpaceDay;n++)
			{
				ATL::CTimeSpan span(n,0,0,0);
				base::Calendar calendatTemp(spanMonth.GetTime());
				calendatTemp += span;
				DayOption *dayOption = calendarChooseUI->AppendDay(calendatTemp,showCal.GetMonth());
				dayOption->SetCurrentDay(showCal);
			}
		}		
		for(int n=0;n<nDays;n++)
		{
			ATL::CTimeSpan span(n,0,0,0);
			base::Calendar calendatTemp(monthStart.GetTime());
			calendatTemp += span;
			DayOption *dayOption = calendarChooseUI->AppendDay(calendatTemp,showCal.GetMonth());
			dayOption->SetCurrentDay(showCal);
		}
		//计算剩余可容纳天数
		int remainDay = 0;
		{
			int rows = 6;
			int nTotalDays = nDays;
			if(nSpaceDay!=7)
			{
				rows -= 1; //第一行占用天数
				nTotalDays -= 7-nSpaceDay;
			}
			rows -= nTotalDays/7;
			int r = nTotalDays%7;
			//如果存在余数据
			if( r )
			{
				rows -= 1;
				remainDay += 7-r;
			}
			if(rows)
				remainDay += rows*7;
		}
		if( remainDay )
		{
			//从下个月借remainDay天填充数据
			ATL::CTimeSpan span(nDays,0,0,0);
			base::Calendar nextMonth(monthStart);
			nextMonth += span;
			for(int n=0;n<remainDay;n++)
			{
				ATL::CTimeSpan span(n,0,0,0);
				base::Calendar calendatTemp(nextMonth.GetTime());
				calendatTemp += span;
				DayOption *dayOption = calendarChooseUI->AppendDay(calendatTemp,showCal.GetMonth());
				dayOption->SetCurrentDay(showCal);
			}
		}
		return ;
	}
	void CalendarUI::ShowFootBar(bool v)
	{
		DuiLib::CControlUI *footBar = m_PaintManager.FindControl(_T("footBar"));
		bool reWinHight = (footBar->IsVisible() != v);
		footBar->SetVisible(v);
		if( reWinHight && footBar->GetFixedHeight() )
		{
			RECT rcWin;
			int  nH = 0;
			GetWindowRect(m_hWnd,&rcWin);
			if(v)
			{
				nH = rcWin.bottom-rcWin.top+footBar->GetFixedHeight();
			}
			else
			{
				nH = rcWin.bottom-rcWin.top-footBar->GetFixedHeight();
			}			
			::SetWindowPos(m_hWnd,NULL,0,0,rcWin.right-rcWin.left,nH,SWP_NOACTIVATE|SWP_NOZORDER);
		}
	}
	bool CalendarUI::IsValid(const base::Calendar &v)
	{
		bool valid = true;
		if(fromValid && toValid)
		{
			valid = (v >= *fromValid && v <= *toValid);
		}
		return valid;
	}
	void CalendarUI::OnClick(DuiLib::TNotifyUI& msg)
	{
		if( msg.pSender->GetName()==_T("beforeYear") )
		{
			base::Calendar tmp(*calendar);
			if( tmp.PlusYear(-1) && IsValid(tmp) )
			{
				*calendar = tmp;
				SetCurrentPage(*calendar);
			}
			return ;
		}
		if( msg.pSender->GetName()==_T("beforeMonth") )
		{
			base::Calendar tmp(*calendar);
			if( tmp.PlusMonth(-1) && IsValid(tmp) )
			{
				*calendar = tmp;
				SetCurrentPage(*calendar);
			}
			return ;
		}
		if( msg.pSender->GetName()==_T("nextMonth") )
		{
			base::Calendar tmp(*calendar);
			if( tmp.PlusMonth(1) && IsValid(tmp) )
			{
				*calendar = tmp;
				SetCurrentPage(*calendar);
			}
			return ;
		}
		if( msg.pSender->GetName()==_T("nextYear") )
		{
			base::Calendar tmp(*calendar);
			if( tmp.PlusYear(1) && IsValid(tmp) )
			{
				*calendar = tmp;				
				SetCurrentPage(*calendar);
			}
			return ;
		}
		if( msg.pSender->GetName()==_T("today") )
		{
			*calendar = ATL::CTime::GetCurrentTime();
			SetCurrentPage(*calendar);
			return ;
		}
	}
	void CalendarUI::Notify(DuiLib::TNotifyUI& msg)
	{
		DuiLib::WindowImplBase::Notify(msg);
		if(msg.sType==DUI_MSGTYPE_WINDOWINIT)
		{			
			calendarHeaderUI = base::SafeConvert<CalendarHeaderUI>(&m_PaintManager,_T("header"),_T("CalendarHeaderUI"));
			calendarChooseUI = base::SafeConvert<CalendarChooseUI>(&m_PaintManager,_T("choose"),_T("CalendarChooseUI"));
			calendarChooseUI->SetHeader(calendarHeaderUI);
			*calendar = ATL::CTime::GetCurrentTime();
			SetCurrentPage(*calendar);
			return ;
		}
		if( msg.sType==DUI_MSGTYPE_SELECTCHANGED )
		{
			DayOption *dayOption = base::SafeConvert<DayOption>(msg.pSender,_T("DayOption"));
			if( dayOption->IsSelected() )
			{
				ATL::CTime tm = dayOption->GetTime();
				if( IsValid(base::Calendar(tm.GetTime())) )
				{
					*calendar = tm;
					if( tm.GetMonth()!=calendar->GetMonth() )
						SetCurrentPage(*calendar);					
					if( fireSource )
					{
						fireSource((void*)this);
					}
				}
			}
			return ;
		}
	}
	LRESULT CalendarUI::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if(uMsg==WM_KILLFOCUS)
		{
			Close();
			return 0;
		}
		if(uMsg==WM_KEYDOWN && wParam==VK_ESCAPE)
		{
			Close();
			return 0;
		}
		return DuiLib::WindowImplBase::HandleMessage(uMsg,wParam,lParam);
	}
	const base::Calendar& CalendarUI::GetChooseDate() const
	{
		return *calendar;
	}
}