#include "StdAfx.h"
#include "ListExUI.h"

namespace CustomUI
{
	ListUI::ListUI(void)
	{

	}
	void ListUI::SetPos(RECT rc)
	{
		__super::SetPos(rc);
		RECT rcBody = m_pList->GetPos();
		m_pList->SetPos(rcBody);
		if(firePos)
			firePos(this);
	}
	void ListUI::PaintBorder(HDC hDC)
	{
		DuiLib::CScrollBarUI *vBar = GetVerticalScrollBar();
		if(vBar && vBar->IsVisible() )
		{
			DuiLib::CListUI::PaintBorder(hDC);
			return ;
		}
		RECT rcBorder = m_rcItem;
		DuiLib::CListHeaderUI *header = GetHeader();
		DuiLib::CControlUI *last = NULL;
		bool calHeader = true;
		if(GetCount())
		{
			DuiLib::CControlUI *tmp = NULL;
			for(int n=GetCount()-1;n>=0;n++)
			{
				tmp = GetItemAt(n);
				if(!tmp->IsFloat() && tmp->IsVisible())
				{
					last = tmp;
					break;
				}
			}
			if(last)
			{
				rcBorder.bottom = last->GetPos().bottom;
				calHeader = false;
			}
		}		
		if(calHeader)
		{
			rcBorder.bottom = header->GetPos().bottom;
		}
		rcBorder.bottom += GetInset().bottom;

		if(m_dwBorderColor != 0 || m_dwFocusBorderColor != 0)
		{
			if(m_nBorderSize > 0 && ( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 ))//»­Ô²½Ç±ß¿ò
			{
				if (IsFocused() && m_dwFocusBorderColor != 0)
					DuiLib::CRenderEngine::DrawRoundRect(hDC, rcBorder, m_nBorderSize, m_cxyBorderRound.cx, m_cxyBorderRound.cy, GetAdjustColor(m_dwFocusBorderColor));
				else
					DuiLib::CRenderEngine::DrawRoundRect(hDC, rcBorder, m_nBorderSize, m_cxyBorderRound.cx, m_cxyBorderRound.cy, GetAdjustColor(m_dwBorderColor));
			}
			else
			{
				if (IsFocused() && m_dwFocusBorderColor != 0 && m_nBorderSize > 0)
					DuiLib::CRenderEngine::DrawRect(hDC, m_rcItem, m_nBorderSize, GetAdjustColor(m_dwFocusBorderColor),m_nBorderStyle);
				else if(m_rcBorderSize.left > 0 || m_rcBorderSize.top > 0 || m_rcBorderSize.right > 0 || m_rcBorderSize.bottom > 0)
				{
					RECT rcBorderTmp;
					if(m_rcBorderSize.left > 0){
						rcBorderTmp		= rcBorder;
						rcBorderTmp.right	= rcBorder.left;
						if(IsFocused() && m_dwFocusBorderColor != 0)
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorderTmp,m_rcBorderSize.left,GetAdjustColor(m_dwFocusBorderColor),m_nBorderStyle);
						else
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorderTmp,m_rcBorderSize.left,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
					if(m_rcBorderSize.top > 0){
						rcBorderTmp		= rcBorder;
						rcBorderTmp.bottom	= rcBorder.top;
						if(IsFocused() && m_dwFocusBorderColor != 0)
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorderTmp,m_rcBorderSize.top,GetAdjustColor(m_dwFocusBorderColor),m_nBorderStyle);
						else
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorderTmp,m_rcBorderSize.top,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
					if(m_rcBorderSize.right > 0){
						rcBorderTmp		= rcBorder;
						rcBorderTmp.left	= rcBorder.right;
						rcBorderTmp.left = rcBorderTmp.left-1;
						rcBorderTmp.right = rcBorderTmp.right-1;
						if(IsFocused() && m_dwFocusBorderColor != 0)
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorderTmp,m_rcBorderSize.right,GetAdjustColor(m_dwFocusBorderColor),m_nBorderStyle);
						else
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorderTmp,m_rcBorderSize.right,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
					if(m_rcBorderSize.bottom > 0){
						rcBorderTmp		= rcBorder;
						rcBorderTmp.top	= rcBorder.bottom;
						rcBorderTmp.top = rcBorderTmp.top-1;
						rcBorderTmp.bottom = rcBorderTmp.bottom-1;
						if(IsFocused() && m_dwFocusBorderColor != 0)
						{
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorderTmp,m_rcBorderSize.bottom,GetAdjustColor(m_dwFocusBorderColor),m_nBorderStyle);
						}
						else
						{
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorderTmp,m_rcBorderSize.bottom,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
						}
					}
				}
				else if(m_nBorderSize > 0)
					DuiLib::CRenderEngine::DrawRect(hDC, rcBorder, m_nBorderSize, GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
			}
		}
	}
}