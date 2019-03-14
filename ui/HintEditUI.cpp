#include "StdAfx.h"
#include "UIlib.h"
#include "HintEditUI.h"
namespace CustomUI
{
	HintEditUI::HintEditUI(void) 
		: m_bkTextColor(0),m_bkFont(0),m_bkTextStyle(DT_LEFT),m_needbktext(true)
	{
		memset(&m_bkTextPadding,0,sizeof(RECT));
	}
	HintEditUI::~HintEditUI(void)
	{

	}
	void HintEditUI::Init()
	{
		__super::DoInit();
		if(IsPasswordMode())
		{
			SetPasswordChar(L'¡ñ');
 		}		
	}
	void HintEditUI::DoEvent(DuiLib::TEventUI& event)
	{
		if( event.Type==DuiLib::UIEVENT_CHAR&&event.chKey==VK_RETURN)
		{
			GetManager()->SendNotify(this,DUI_MSGTYPE_RETURN);
			return ;
		}
		if( event.Type==DuiLib::UIEVENT_KILLFOCUS )
		{
			if( IsEnabled() ) 
			{
				DuiLib::CDuiString v = GetText();
				if( v.IsEmpty() )
				{
					m_needbktext = true;
				}
				m_uButtonState &= ~UISTATE_FOCUSED;
				if(OnInputEvent)
					OnInputEvent(this);
				this->Invalidate();
			}
		}
		if( event.Type == DuiLib::UIEVENT_SETFOCUS )
		{
			if( IsEnabled() )
			{
				m_needbktext = false;
				m_uButtonState |= UISTATE_FOCUSED;
				if(OnInputEvent)
					OnInputEvent(this);
				this->Invalidate();
			}
		}
		__super::DoEvent(event);
	}
	void HintEditUI::PaintBorder(HDC hDC)
	{
		if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && m_dwFocusBorderColor )
		{
			DuiLib::CRenderEngine::DrawRect(hDC,m_rcItem,m_nBorderSize,
				GetAdjustColor(m_dwFocusBorderColor));
			return ;
		}
		__super::PaintBorder(hDC);
	}
	void HintEditUI::PaintText(HDC hDC)
	{
		if( !m_sText.IsEmpty() )
		{
			__super::PaintText(hDC);
			return ;
		}
		if( !m_bkText.IsEmpty() && m_needbktext )
		{
			if( m_bkTextColor == 0 )
				m_bkTextColor = m_pManager->GetDefaultDisabledColor();
			RECT rc = m_rcItem;
			rc.left   += m_bkTextPadding.left;
			rc.right  -= m_bkTextPadding.right;
			rc.top    += m_bkTextPadding.top;
			rc.bottom -= m_bkTextPadding.bottom;
			DuiLib::CRenderEngine::DrawText(hDC, m_pManager, rc, m_bkText, m_bkTextColor, \
				m_bkFont, DT_SINGLELINE | m_bkTextStyle);
		}
	}
	void HintEditUI::SetBkTextColor(DWORD dwColor)
	{
		m_bkTextColor = dwColor;
		this->Invalidate();
	}
	void HintEditUI::SetBkFont(int nI)
	{
		m_bkFont = nI;
		this->Invalidate();
	}
	void HintEditUI::SetBkTextPadding(RECT rc)
	{
		memcpy(&m_bkTextPadding,&rc,sizeof(RECT));
		SetTextPadding(m_bkTextPadding);
		this->Invalidate();
	}
	void HintEditUI::SetText(LPCTSTR pstrValue)
	{
		if(GetText()!=pstrValue)
			m_needbktext = true;
		__super::SetText(pstrValue);
		this->Invalidate();
	}
	void HintEditUI::SetBkText(LPCTSTR pstrValue)
	{
		m_bkText = pstrValue;
		m_needbktext = true;
		this->Invalidate();
	}
	void HintEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( 0 == _tcscmp(pstrName,_T("bkfont")))
		{
			SetBkFont(_ttoi(pstrValue));
			return ;
		}
		if( 0 == _tcscmp(pstrName,_T("bktext")) )
		{
			SetBkText(pstrValue);
			return ;
		}
		if( 0 == _tcscmp(pstrName,_T("bktextcolor")) )
		{
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetBkTextColor(clrColor);
			return ;
		}
		if( _tcscmp(pstrName, _T("bkalign")) == 0 )
		{
			if( _tcsstr(pstrValue, _T("left")) != NULL )
			{
				m_bkTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
				m_bkTextStyle |= DT_LEFT;
			}
			if( _tcsstr(pstrValue, _T("center")) != NULL )
			{
				m_bkTextStyle &= ~(DT_LEFT | DT_RIGHT );
				m_bkTextStyle |= DT_CENTER;
			}
			if( _tcsstr(pstrValue, _T("right")) != NULL )
			{
				m_bkTextStyle &= ~(DT_LEFT | DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				m_bkTextStyle |= DT_RIGHT;
			}
			if( _tcsstr(pstrValue, _T("top")) != NULL )
			{
				m_bkTextStyle &= ~(DT_BOTTOM | DT_VCENTER);
				m_bkTextStyle |= (DT_TOP | DT_SINGLELINE);
			}
			if( _tcsstr(pstrValue, _T("vcenter")) != NULL ) 
			{
				m_bkTextStyle &= ~(DT_TOP | DT_BOTTOM );			
				m_bkTextStyle |= (DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			if( _tcsstr(pstrValue, _T("bottom")) != NULL ) 
			{
				m_bkTextStyle &= ~(DT_TOP | DT_VCENTER);
				m_bkTextStyle |= (DT_BOTTOM | DT_SINGLELINE);
			}
			return ;
		}
		if( _tcscmp(pstrName, _T("bkendellipsis")) == 0 )
		{
			if( _tcscmp(pstrValue, _T("true")) == 0 )
				m_bkTextStyle |= DT_END_ELLIPSIS;
			else 
				m_bkTextStyle &= ~DT_END_ELLIPSIS;
			return ;
		}
		if( _tcscmp(pstrName, _T("bkfont")) == 0 )
		{
			SetBkFont(_ttoi(pstrValue));
			return ;
		}
		if( _tcscmp(pstrName, _T("bktextpadding")) == 0 ) 
		{
			RECT rcTextPadding = { 0 };
			LPTSTR pstr = NULL;
			rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
			rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
			SetBkTextPadding(rcTextPadding);
		}
		__super::SetAttribute(pstrName,pstrValue);
	}
}