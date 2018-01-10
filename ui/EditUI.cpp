#include "StdAfx.h"
#include "EditUI.h"

namespace CustomUI
{
	EditUI::EditUI(void) 
		: m_bkTextColor(0),m_bkFont(0),m_bkTextStyle(DT_LEFT),
		  m_needbktext(true),m_bIME(true),m_imc(NULL)
	{
		memset(&m_bkTextPadding,0,sizeof(RECT));
	}
	EditUI::~EditUI(void)
	{

	}
	bool EditUI::IsPasswordMode()
	{
		return ((__super::GetWinStyle()&ES_PASSWORD)==ES_PASSWORD);
	}
	bool EditUI::IsMultiLine()
	{
		if(__super::IsWantReturn())
			return ((__super::GetWinStyle()&ES_MULTILINE)==ES_MULTILINE);
		return false;
	}
	void EditUI::EnaleMultiLine(bool b)
	{
		long style = __super::GetWinStyle();
		if(b==false)
			style &= ~ES_MULTILINE;
		else
			style |= ES_MULTILINE;
		__super::SetWinStyle(style);
		LRESULT ret = 0;
		__super::TxSendMessage(EM_SETOPTIONS,ECOOP_SET,style,&ret);
	}
	void EditUI::Init()
	{
		if(IsPasswordMode())
		{
			m_pTwh->SetPasswordChar(L'●');
			EnableIME(false);
		}
		__super::DoInit();
	}
	void EditUI::OnTxNotify(DWORD iNotify, void *pv)
	{
		if(iNotify==EN_UPDATE)
		{
			return ;
		}
		__super::OnTxNotify(iNotify,pv);
	}
	void EditUI::EnableIME(bool b)
	{
		m_bIME = b;
		if(m_bIME==false)
			m_imc = ::ImmAssociateContext(GetManager()->GetPaintWindow(),NULL);
	}
	void EditUI::DoEvent(DuiLib::TEventUI& event)
	{
		if( event.Type==DuiLib::UIEVENT_CHAR&& event.chKey==VK_RETURN&&!IsMultiLine())
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
				if(m_imc)
					::ImmAssociateContext(GetManager()->GetPaintWindow(),m_imc);
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
				if(m_bIME==false)
					m_imc = ::ImmAssociateContext(GetManager()->GetPaintWindow(),NULL);
				if(OnInputEvent)
					OnInputEvent(this);
				this->Invalidate();
			}
		}
		__super::DoEvent(event);
	}
	void EditUI::PaintBorder(HDC hDC)
	{
		if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && m_dwFocusBorderColor )
		{
			DuiLib::CRenderEngine::DrawRect(hDC,m_rcItem,m_nBorderSize,
									GetAdjustColor(m_dwFocusBorderColor));
			return ;
		}
		__super::PaintBorder(hDC);
	}
	void EditUI::PaintText(HDC hDC)
	{
		if( !m_sText.IsEmpty() )
		{
			__super::PaintText(hDC);
			return ;
		}
		if( !m_bkText.IsEmpty() && m_needbktext && 0==__super::GetTextLength() )
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
	void EditUI::SetBkTextColor(DWORD dwColor)
	{
		m_bkTextColor = dwColor;
		this->Invalidate();
	}
	void EditUI::SetBkFont(int nI)
	{
		m_bkFont = nI;
		this->Invalidate();
	}
	void EditUI::SetBkTextPadding(RECT rc)
	{
		memcpy(&m_bkTextPadding,&rc,sizeof(RECT));
		SetTextPadding(m_bkTextPadding);
		this->Invalidate();
	}
	void EditUI::SetText(LPCTSTR pstrValue)
	{
		__super::SetText(pstrValue);
		if( GetTextLength() )
			m_needbktext = true;
		this->Invalidate();
	}
	void EditUI::SetBkText(LPCTSTR pstrValue)
	{
		m_bkText = pstrValue;
		m_needbktext = true;
		this->Invalidate();
	}
	HRESULT EditUI::TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult) const
	{
		HRESULT hr = 0;
		EditUI *pT = const_cast<EditUI*>(this);
		if(msg==WM_CUT || msg==WM_PASTE)
		{
			hr = __super::TxSendMessage(msg,wparam,lparam,plresult);
			if(pT->OnInputEvent)
				pT->OnInputEvent(pT);
			return hr;
		}
		if( msg==WM_KEYDOWN )
		{
			bool replace = false;
			if( wparam==VK_DELETE || wparam==VK_BACK )
			{
				replace = true;
			}
			if(::GetKeyState(VK_CONTROL)<0)
			{	//按下复制，黏贴快捷键
				TCHAR ch = ::MapVirtualKey(wparam,MAPVK_VK_TO_CHAR);
				if( ch=='X' || ch=='x')
					replace = true;
				if( ch=='V' || ch=='v')
					replace = true;
				if( ch=='Z' || ch=='z')
					replace = true;
				if( ch=='Y' || ch=='y')
					replace = true;
			}
			if(replace)
			{
				hr = __super::TxSendMessage(msg,wparam,lparam,plresult);
				if(pT->OnInputEvent)
					pT->OnInputEvent(pT);
				return hr;
			}
		}
		if( msg==WM_CHAR )
		{
			bool bInput = true;
			{
				DuiLib::TEventUI ev;
				ev.Type = DuiLib::UIEVENT_CHAR;			
				ev.chKey = (TCHAR)wparam;
				ev.lParam = lparam;
				ev.wParam = wparam;
				ev.dwTimestamp = ::GetTickCount();
				ev.ptMouse = GetManager()->GetMousePos();
				ev.pSender = pT;
				bInput = pT->OnInputFilter(&ev);
			}
			if(bInput)
			{
				hr = __super::TxSendMessage(msg,wparam,lparam,plresult);
				if(pT->OnInputEvent)
					pT->OnInputEvent(pT);
			}
			if(wparam==VK_RETURN&&!pT->IsMultiLine())
			{
				//单行发出
				pT->GetManager()->SendNotify(const_cast<EditUI*>(this),DUI_MSGTYPE_RETURN);
			}
			return hr;
		}
		return __super::TxSendMessage(msg,wparam,lparam,plresult);
	}
	void EditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
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