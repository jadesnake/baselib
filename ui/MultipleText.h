#pragma once

#include "MacroX.h"
namespace CustomUI
{
	class MultipleText : public DuiLib::CTextUI
	{
	public:
		MultipleText();
		void SetText(LPCTSTR pstrText);
		void CalculationsDrawText(HDC hDC,RECT &rc,DuiLib::CDuiString &outStr,int nFont);
		
		void SetLink(bool bV);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void DoEvent(DuiLib::TEventUI& event);
		bool Activate();
		SIZE EstimateSize(SIZE szAvailable);
		RECT GetTextRc();
		DUI_DECLARE(CustomUI::MultipleText,DuiLib::CTextUI)
	protected:
		void PaintText(HDC hDC);
	protected:
		DuiLib::CDuiString m_drawText;		//可以
		bool		m_bShowAll;
		bool		m_bLink;		//按链接方式显示	
		RECT		m_rcText;		//文本区域
		UINT		m_unState;		//文本区域状态
		HCURSOR		m_cursortmp;	//鼠标指针临时对象
	};
}