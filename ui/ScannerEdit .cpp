#include "stdafx.h"
#include "ScannerEdit.h"

namespace CustomUI{

	ScannerEditUI::ScannerEditUI()
		: m_bkTextColor(0),m_bkFont(0),m_bkTextStyle(DT_LEFT),
		  m_needbktext(true),m_keydown(false),m_imc(NULL),
		  m_numberOnly(false)
	{
		memset(&m_bkTextPadding,0,sizeof(RECT));
		SetRich(false);
	}
	ScannerEditUI::~ScannerEditUI()
	{

	}
	void ScannerEditUI::DoEvent(DuiLib::TEventUI& event)
	{
		if( event.Type == DuiLib::UIEVENT_KILLFOCUS )
		{
			if( IsEnabled() ) 
			{
				DuiLib::CDuiString v = GetText();
				if( v.IsEmpty() )
				{
					m_needbktext = true;
				}
				m_uButtonState &= ~UISTATE_FOCUSED;
				this->Invalidate();
				::ImmAssociateContext(GetManager()->GetPaintWindow(),m_imc);
				if( OnLostFocus )
					OnLostFocus(this);
			}
		}
		if( event.Type == DuiLib::UIEVENT_SETFOCUS )
		{
			if( IsEnabled() )
			{
				m_needbktext = false;
				m_uButtonState |= UISTATE_FOCUSED;
				this->Invalidate();
				m_imc = ::ImmAssociateContext(GetManager()->GetPaintWindow(),NULL);
				if( OnHaveFocus )
					OnHaveFocus(this);
			}
		}
		__super::DoEvent(event);
	}
	void ScannerEditUI::PaintBorder(HDC hDC)
	{
		if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && m_dwFocusBorderColor )
		{
			DuiLib::CRenderEngine::DrawRect(hDC,m_rcItem,m_nBorderSize,
				GetAdjustColor(m_dwFocusBorderColor));
			return ;
		}
		__super::PaintBorder(hDC);
	}
	void ScannerEditUI::PaintText(HDC hDC)
	{
		if( !m_sText.IsEmpty() )
		{
			__super::PaintText(hDC);
			return ;
		}
		if( !m_bkText.IsEmpty() && m_needbktext && ((m_uButtonState&UISTATE_FOCUSED)!=UISTATE_FOCUSED) && GetTextLength()==0 )
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
	void ScannerEditUI::SetBkTextColor(DWORD dwColor)
	{
		m_bkTextColor = dwColor;
		this->Invalidate();
	}
	void ScannerEditUI::SetBkFont(int nI)
	{
		m_bkFont = nI;
		this->Invalidate();
	}
	void ScannerEditUI::SetBkTextPadding(RECT rc)
	{
		memcpy(&m_bkTextPadding,&rc,sizeof(RECT));
		SetTextPadding(m_bkTextPadding);
		this->Invalidate();
	}
	void ScannerEditUI::SetText(LPCTSTR pstrValue)
	{
		DuiLib::CRichEditUI::SetText(pstrValue);
		m_needbktext = true;
		m_scanText.Empty();	//扫描枪输入被打断
		this->Invalidate();
	}
	void ScannerEditUI::SetNumberOnly(bool bNumberOnly)
	{
		m_numberOnly = bNumberOnly;
	}
	bool ScannerEditUI::IsNumberOnly()
	{
		return m_numberOnly;
	}
	void ScannerEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( 0 == _tcscmp(pstrName,_T("bktext")) )
		{
			m_bkText = pstrValue;
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
		if( _tcscmp(pstrName,_T("numberonly"))==0 )
		{
			SetNumberOnly(_tcscmp(pstrValue, _T("true"))== 0);
			return ;
		}
		__super::SetAttribute(pstrName,pstrValue);
	}
	const DuiLib::CDuiString& ScannerEditUI::GetScanText() const
	{
		return m_scanText;
	}
	bool ScannerEditUI::OnScanInput(void *param)
	{
		DuiLib::TEventUI *event = static_cast<DuiLib::TEventUI*>(param);
		if(event->Type== DuiLib::UIEVENT_CHAR)
		{
			if( event->chKey>='0' && event->chKey<='9' )
				return true;
			if( event->chKey>='a' && event->chKey<='z' && m_numberOnly==false)
				return true;
			if( event->chKey>='A' && event->chKey<='Z' && m_numberOnly==false)
				return true;
		}
		return false;
	}
	HRESULT ScannerEditUI::TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult) const
	{
		HRESULT hr = 0;
		ScannerEditUI *pT = const_cast<ScannerEditUI*>(this);
		if(msg==WM_CUT || msg==WM_PASTE)
		{
			hr = __super::TxSendMessage(msg,wparam,lparam,plresult);
			pT->m_scanText = GetText();
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
				pT->m_scanText = GetText();
				if(pT->OnInputEvent)
					pT->OnInputEvent(pT);
				return hr;
			}
		}
		if( msg==WM_CHAR )
		{
			DuiLib::DUI__Trace(_T("WM_CHAR w %x l %x"),wparam,lparam);
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
				bInput = pT->OnScanInput(&ev);
			}
			bool replace = false;
			{
				long nStart=0,nEnd=0;
				GetSel(nStart,nEnd);
				replace = (nEnd > nStart);
			}
			if(replace && (wparam!=VK_RETURN)  )
			{

				//替换文本
				if(bInput)
					hr = __super::TxSendMessage(msg,wparam,lparam,plresult);
				pT->m_scanText = GetText();
				if(pT->OnInputEvent)
					pT->OnInputEvent(pT);
				return hr;
			}
			if(wparam==VK_RETURN)
			{
				//解析输入的数据
				if( pT->OnScanComplete )
					pT->OnScanComplete(pT);
				pT->m_scanText.Empty();
				if(pT->OnEnter)
					pT->OnEnter(pT);
			}
			//不知道22是个什么玩意，但这个值输入进去是不对滴
			if(wparam!=VK_BACK && wparam!=22)
			{
				//快捷键
				pT->m_scanText += (TCHAR)wparam;
			}
			if(bInput)
			{
				hr = __super::TxSendMessage(msg,wparam,lparam,plresult);
				if(pT->OnInputEvent)
					pT->OnInputEvent(pT);
			}
			return hr;
		}
		return __super::TxSendMessage(msg,wparam,lparam,plresult);
	}
}