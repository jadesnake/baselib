#pragma once

#include "../base/MacroX.h"
namespace CustomUI
{
	class ScannerEditUI : public DuiLib::CRichEditUI
	{
	public:
		ScannerEditUI(void);
		~ScannerEditUI(void);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		void SetBkTextColor(DWORD dwColor);
		void SetBkFont(int nI);
		void SetBkTextPadding(RECT rc);
		void SetText(LPCTSTR pstrValue);
		void SetNumberOnly(bool bNumberOnly);
		bool IsNumberOnly();
		const DuiLib::CDuiString& GetScanText() const;
		DuiLib::CEventSource OnScanComplete;
		DuiLib::CEventSource OnEnter;
		DuiLib::CEventSource OnInputEvent;
		DuiLib::CEventSource OnLostFocus;
		DuiLib::CEventSource OnHaveFocus;
		DUI_DECLARE(CustomUI::ScannerEditUI,DuiLib::CRichEditUI)
	protected:
		void Init();
		void EnaleMultiLine(bool b);
		bool OnScanInput(void *param);
		void DoEvent(DuiLib::TEventUI& event);
		void PaintText(HDC hDC);
		void PaintBorder(HDC hDC);
		HRESULT TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult) const;
	protected:
		HIMC  m_imc;
		DuiLib::CDuiString m_scanText; //输入的内容
		DuiLib::CDuiString m_bkText;//背景文本
		DWORD m_bkTextColor;		//背景文本颜色
		RECT  m_bkTextPadding;		//背景文本位置
		int	  m_bkFont;				//背景文本字体
		UINT  m_bkTextStyle;		//背景文本样式
		bool  m_needbktext;
		bool  m_keydown;			//键盘按下状态
		bool  m_numberOnly;			//只是允许输入数字
	};
}