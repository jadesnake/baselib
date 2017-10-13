#pragma once

#include "../base/MacroX.h"
namespace CustomUI
{
	class EditUI : public DuiLib::CEditUI 
	{
	public:
		EditUI(void);
		~EditUI(void);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetBkTextColor(DWORD dwColor);
		void SetBkFont(int nI);
		void SetBkTextPadding(RECT rc);
		void SetText(LPCTSTR pstrValue);
		void SetBkText(LPCTSTR pstrValue);
		virtual void Init();
		DUI_DECLARE(CustomUI::EditUI,DuiLib::CEditUI)
	protected:
		void DoEvent(DuiLib::TEventUI& event);
		void PaintText(HDC hDC);
		void PaintBorder(HDC hDC);
	protected:
		DuiLib::CDuiString m_bkText;//背景文本
		DWORD m_bkTextColor;		//背景文本颜色
		RECT  m_bkTextPadding;		//背景文本位置
		int	  m_bkFont;				//背景文本字体
		UINT  m_bkTextStyle;		//背景文本样式
		bool  m_needbktext;
	};
}
