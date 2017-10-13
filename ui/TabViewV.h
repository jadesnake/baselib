#pragma once

#include "../base/MacroX.h"
namespace CustomUI
{
	class TabViewV : public DuiLib::CVerticalLayoutUI
	{
	public:
		class Page : public DuiLib::CHorizontalLayoutUI
		{
		public:
			Page(TabViewV *pParent,DuiLib::CControlUI *pView);
			Page();
			~Page();
			void SetScrollPosAt( SIZE szPos );
			SIZE GetScrollPosAt();
			void SetTabView( TabViewV *pV );
			TabViewV*	GetTabView() const;
			bool IsSelected() const;
			void SetVisible(bool bVisible=true);
			void SetView(DuiLib::CControlUI* pView);
			DuiLib::COptionUI*	GetOptionUI() const;
			DuiLib::CControlUI*	GetView() const;
			void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
			DUI_DECLARE(TabViewV::Page,DuiLib::CHorizontalLayoutUI)
		protected:
			bool OnChangeView(void *param);
		private:
			TabViewV			*m_pParent;
			DuiLib::CControlUI	*m_pView;
			DuiLib::COptionUI	*m_pOptCtrl;
			DuiLib::CDuiString	m_strViewClass;
			SIZE				m_szPos;
		};
		TabViewV();
		~TabViewV();
		DuiLib::CTabLayoutUI*			GetTabLayout();
		DuiLib::CHorizontalLayoutUI*	GetLableLayout();
		bool	Switch( Page *page ,bool bFocus=true);
		bool	Switch(LPCTSTR pstrName,bool bFocus=true);
		Page*	RemoveUI( Page *page );
		void	ResetPageView(Page *page,DuiLib::CControlUI *pView);
		Page*	AddPage(DuiLib::CControlUI *pView);
		bool	Add(DuiLib::CControlUI *pCtl);
		int		GetPageCount();
		Page*	GetPage(int nI);
		Page*	FindPage(LPCTSTR pstrName);
		void	SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		Page*	GetCurPage();
		DuiLib::CEventSource OnSwitch;
		DUI_DECLARE(CustomUI::TabViewV,DuiLib::CVerticalLayoutUI)
	protected:
		void PaintBorder(HDC hDC);
		void AddPageView(DuiLib::CControlUI *pView);
		void ClearPageView(DuiLib::CControlUI *pView);
		bool OnTablayoutChange(void *param);
	private:
		DuiLib::CHorizontalLayoutUI	*m_pLable;	//LableÍ·
		DuiLib::CTabLayoutUI		*m_pTable;	//table¿Ø¼þ
		friend						Page;
	};
}