#include "StdAfx.h"
#include "UIlib.h"
#include "ButtonUI.h"
namespace CustomUI
{
	ButtonUI::ButtonUI(void)
	{
	}
	ButtonUI::~ButtonUI(void)
	{

	}
	void ButtonUI::PaintText(HDC hDC)
	{
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;
		if( m_bShowHtml )
			DuiLib::CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, GetText(), GetTextColor(), \
			NULL, NULL, nLinks, m_uTextStyle);
		else
			DuiLib::CRenderEngine::DrawText(hDC, m_pManager, rc, GetText(), GetTextColor(), \
			m_iFont, m_uTextStyle);
		DuiLib::CButtonUI::PaintText(hDC);
	}
	void ButtonUI::SetText(LPCTSTR pstrText)
	{
		bool parentUpdate = GetText().Compare(pstrText)?true:false;
		__super::SetText(pstrText);
		if(parentUpdate)
			NeedParentUpdate();
	}
	SIZE ButtonUI::EstimateSize(SIZE szAvailable)
	{
		if(GetText().IsEmpty())
			return __super::EstimateSize(szAvailable);
		RECT rcText = { 0, 0, (m_cxyFixed.cx>0)?m_cxyFixed.cx:MAX(szAvailable.cx, m_cxyFixed.cx), 9999 };
		rcText.left += m_rcTextPadding.left;
		rcText.right -= m_rcTextPadding.right;
		if( m_bShowHtml )
		{   
			int nLinks = 0;
			DuiLib::CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), m_pManager, rcText, GetText(), m_dwTextColor, NULL, NULL, nLinks, DT_CALCRECT | m_uTextStyle);
		}
		else 
		{
			DuiLib::CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, GetText(), m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
		}
		SIZE cXY = {rcText.right - rcText.left + m_rcTextPadding.left + m_rcTextPadding.right,
			rcText.bottom - rcText.top + m_rcTextPadding.top + m_rcTextPadding.bottom};

		if( m_cxyFixed.cy != 0 ) cXY.cy = m_cxyFixed.cy;
		if(m_cxyFixed.cx > 0) cXY.cx = m_cxyFixed.cx;
		return cXY;
	}

}
