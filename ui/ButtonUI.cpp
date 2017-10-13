#include "StdAfx.h"
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
			DuiLib::CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, GetTextColor(), \
			NULL, NULL, nLinks, m_uTextStyle);
		else
			DuiLib::CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, GetTextColor(), \
			m_iFont, m_uTextStyle);
		DuiLib::CButtonUI::PaintText(hDC);
	}
}
