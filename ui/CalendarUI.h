#pragma once

namespace base
{
	class Calendar;
}
namespace CustomUI {

	class CalendarHeaderUI;
	class CalendarChooseUI;

	class CalendarUI : public DuiLib::WindowImplBase
	{
	public:
		CalendarUI(HWND hParent=NULL);
		~CalendarUI();
		void Show(const POINT &pt);

		DuiLib::CEventSource fireSource;
		DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
		void SetCurrentPage(const base::Calendar &showCal);

		void ShowFootBar(bool v);
		//设置有效范围
		void SetValidRange(const base::Calendar &from,base::Calendar &to);
		const base::Calendar& GetChooseDate() const;
	protected:
		void InitWindow();
		DuiLib::CDuiString GetSkinFolder();
		DuiLib::CDuiString GetSkinFile();
		LPCTSTR GetWindowClassName(void)const;
		UILIB_RESOURCETYPE GetResourceType() const;
		LPCTSTR		GetResourceID() const;
		void OnClick(DuiLib::TNotifyUI& msg);
		void OnFinalMessage( HWND hWnd );
		void Notify(DuiLib::TNotifyUI& msg);
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		bool IsValid(const base::Calendar &v);
	protected:
		//起止范围
		base::Calendar *fromValid;
		base::Calendar *toValid;

		base::Calendar *calendar;
		HWND m_hParent;
		POINT m_BasedPoint;
		CAtlString m_sUserData;

		CalendarHeaderUI *calendarHeaderUI;
		CalendarChooseUI *calendarChooseUI;
	};
}