#include "stdafx.h"
#include "UIlib.h"
#include "MultipleText.h"

namespace CustomUI
{
	MultipleText::MultipleText()
		:m_bLink(false),m_cursortmp(NULL),m_bShowAll(false),m_unState(0)
	{
		memset(&m_rcText,0,sizeof(RECT));
	}
	bool MultipleText::Activate()
	{
		if( !CControlUI::Activate() ) return false;
		if( m_pManager != NULL ) m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
		return true;
	}
	void MultipleText::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( 0 == _tcscmp(pstrName,_T("link")) )
		{
			m_bLink = true;
			return ;
		}
		if( 0 == _tcscmp(pstrName,_T("showall")) )
		{
			if( 0 == _tcscmp(pstrValue,_T("true")) )
				m_bShowAll = true;
			else if( 0 == _tcscmp(pstrValue,_T("false")) )
				m_bShowAll = false;
			return ;
		}
		else if( _tcscmp(pstrName, _T("valign")) == 0 )
		{
			if( _tcsstr(pstrValue, _T("top")) != NULL ) 
			{
				m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER);
				m_uTextStyle |= (DT_TOP);
			}
			else if( _tcsstr(pstrValue, _T("vcenter")) != NULL ) 
			{
				m_uTextStyle &= ~(DT_TOP | DT_BOTTOM );            
				m_uTextStyle |= (DT_VCENTER);
			}
			else if( _tcsstr(pstrValue, _T("bottom")) != NULL ) 
			{
				m_uTextStyle &= ~(DT_TOP | DT_VCENTER);
				m_uTextStyle |= (DT_BOTTOM);
			}
			return ;
		}
		__super::SetAttribute(pstrName,pstrValue);
	}
	void MultipleText::SetLink(bool bV)
	{
		m_bLink = bV;
	}
	void MultipleText::DoEvent(DuiLib::TEventUI& event)
	{
		if( event.Type == DuiLib::UIEVENT_MOUSEENTER )
		{
			m_unState |= UISTATE_HOT;
			Invalidate();
			return ;
		}
		if( event.Type == DuiLib::UIEVENT_MOUSELEAVE )
		{
			m_unState &= ~UISTATE_HOT;
			if( m_bLink && m_cursortmp )
				::SetCursor( m_cursortmp );
			Invalidate();
			return ;
		}
		if( event.Type == DuiLib::UIEVENT_MOUSEMOVE && m_bLink )
		{
			if( ((m_unState&UISTATE_HOT) == UISTATE_HOT) && 
				::PtInRect(&m_rcText,event.ptMouse)	&& IsEnabled()
			  )
			{
				m_cursortmp = ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				Invalidate();
				return ;
			}
		}
		if( event.Type == DuiLib::UIEVENT_BUTTONUP && m_bLink && IsEnabled() )
		{
			if( ::PtInRect(&m_rcText,event.ptMouse) )
			{
				if( __super::OnEvent((void*)&event) )
					__super::DoEvent(event);
				Activate();
			}
			return ;
		}
		if( event.Type == DuiLib::UIEVENT_BUTTONDOWN && m_bLink && IsEnabled() )
		{
			if( ::PtInRect(&m_rcText,event.ptMouse) )
			{
				//有按下那就会有抬起因此将click事件放在鼠标抬起时触发
				if( __super::OnEvent((void*)&event) )
					__super::DoEvent(event);
			}
			return ;
		}
		__super::DoEvent(event);
	}
	void MultipleText::SetText(LPCTSTR pstrText)
	{
		__super::SetText(pstrText);
		m_drawText = m_sText.GetData();
	}
	SIZE MultipleText::EstimateSize(SIZE szAvailable)
	{
		if( m_bShowAll )
			return DuiLib::CTextUI::EstimateSize(szAvailable);
		else
			return DuiLib::CControlUI::EstimateSize(szAvailable);
	}
	void MultipleText::CalculationsDrawText(HDC hDC,RECT &rc,DuiLib::CDuiString &outStr,int nFont)
	{
		DuiLib::CDuiString strDraw;
		bool bNeedBreak = false;
		RECT rcCal;
		HFONT hOldFont = (HFONT)::SelectObject(hDC,GetManager()->GetFont(nFont));		
		//取消单行
		//m_uTextStyle &= ~DT_SINGLELINE;
		//设置多行
		m_uTextStyle |= DT_EDITCONTROL|DT_WORDBREAK;
		memcpy(&rcCal,&rc,sizeof(RECT));
		//计算可以绘制部分的字符串
		long nCalH= 0,nCalW=0;	//计算后所需要的高度
		long nOrgH= rcCal.bottom-rcCal.top;
		//计算单个字符显示的高度
		long nCharH = 0;
		{
			RECT tmp = rcCal;
			nCharH = ::DrawText(hDC,_T("测"),-1,&tmp,m_uTextStyle|DT_CALCRECT);	
		}
		nCalH = ::DrawText(hDC,m_sText,-1,&rcCal,m_uTextStyle|DT_CALCRECT);
		if( nCalH <= nOrgH )
		{
			//一次性显示完毕
			strDraw = m_sText;
			bool bSingle = false;
			if( (m_uTextStyle&DT_VCENTER)==DT_VCENTER )
				bSingle = true;
			if( (m_uTextStyle&DT_CENTER)==DT_CENTER )
				bSingle = true;
			//如果单行那么设置单行
			if(bSingle && (abs(nCalH-nCharH)<=2) )
			{
				m_uTextStyle &= ~(DT_EDITCONTROL|DT_WORDBREAK);
				m_uTextStyle |= DT_SINGLELINE;
				rcCal = rc;
			}
		}
		else
		{
			for(int unI=1;unI < m_sText.GetLength();unI++)
			{
				strDraw = m_sText.Mid(0,unI);
				nCalH = ::DrawText(hDC,strDraw,-1,&rcCal,m_uTextStyle|DT_CALCRECT);
				memcpy(&rcCal,&rc,sizeof(RECT));
				if( nCalH > nOrgH )
				{
					if( unI > 3 )
					{
						strDraw = m_sText.Mid(0,unI-3);
						strDraw.Append(_T("..."));
					}
					break;
				}
			}
		}
		outStr = strDraw;
		memcpy(&rc,&rcCal,sizeof(RECT));
		::SelectObject(hDC, hOldFont);
	}
	RECT MultipleText::GetTextRc()
	{
		return m_rcText;
	}
	void MultipleText::PaintText(HDC hDC)
	{
		if( m_drawText.IsEmpty() )
		{
			m_nLinks = 0;
			return;
		}
		if( m_dwTextColor == 0 )
			m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 )
			m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();
		m_nLinks = sizeof(m_rcLinks)/sizeof(*m_rcLinks);
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;
		if( IsEnabled() ) 
		{
			if( m_bShowHtml )
			{
				DuiLib::CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText,
					m_dwTextColor,m_rcLinks, m_sLinks, m_nLinks, m_uTextStyle);
			}
			else
			{
				DWORD clrColor = m_dwTextColor;
				int	  nFont = m_iFont;
				if( m_bLink )
				{
					if( (m_unState&UISTATE_HOT)==UISTATE_HOT )
					{
						DuiLib::TFontInfo *info = m_pManager->GetFontInfo(GetFont());
						nFont = m_pManager->GetFontIndex(info->sFontName,info->iSize,info->bBold,
							true,info->bItalic);
						if( nFont == -1 )
						{
							HFONT hF = m_pManager->AddFont(info->sFontName,info->iSize,info->bBold,
								true,info->bItalic);
							nFont = m_pManager->GetFontIndex(hF);
						}
					}			
				}
				CalculationsDrawText(hDC,rc,m_drawText,nFont);
				DuiLib::CRenderEngine::DrawText(hDC,m_pManager,rc,m_drawText,
					clrColor,nFont,m_uTextStyle);
				if( nFont != m_iFont )
					m_pManager->RemoveFontAt(nFont);
			}			
		}
		else 
		{
			if( m_bShowHtml )
			{
				DuiLib::CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText,
					m_dwDisabledTextColor,m_rcLinks, m_sLinks, m_nLinks, m_uTextStyle);
			}
			else
			{
				CalculationsDrawText(hDC,rc,m_drawText,m_iFont);
				DuiLib::CRenderEngine::DrawText(hDC,m_pManager,rc,m_drawText,
					m_dwDisabledTextColor,m_iFont,m_uTextStyle);
			}
		}
		memcpy(&m_rcText,&rc,sizeof(RECT));
	}
}