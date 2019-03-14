#pragma once

#include "../Base/MacroX.h"
namespace CustomUI
{
	template<class Control>
	class ControlClick : public Control
	{
	public:
		ControlClick()
		{
			m_Correct = 0;
			m_uButtonState = 0;
			m_bButtonDBState = false;
			m_dwHotBkColor = 0;
			m_dwHotTextColor = 0;
			m_dwPushedTextColor = 0;
			m_dwFocusedTextColor = 0;
			m_dwHotBorderColor = 0;
			m_dwHotBorderColor  = 0;
		}
		virtual ~ControlClick()
		{

		}
		void SetHotBkColor( DWORD dwColor )
		{
			m_dwHotBkColor = dwColor;
		}
		DWORD GetHotBkColor() const
		{
			return m_dwHotBkColor;
		}
		void SetHotTextColor(DWORD dwColor)
		{
			m_dwHotTextColor = dwColor;
		}
		DWORD GetHotTextColor() const
		{
			return m_dwHotTextColor;
		}
		void SetPushedTextColor(DWORD dwColor)
		{
			m_dwPushedTextColor = dwColor;
		}
		DWORD GetPushedTextColor() const
		{
			return m_dwPushedTextColor;
		}
		void SetFocusedTextColor(DWORD dwColor)
		{
			m_dwFocusedTextColor = dwColor;
		}
		DWORD GetFocusedTextColor() const
		{
			return m_dwFocusedTextColor;
		}
		void SetHotBorderColor(DWORD dwColor)
		{
			m_dwHotBorderColor=dwColor; 
		}
		DWORD GetHotBorderColor() const
		{
			return m_dwHotBorderColor;
		}
		LPCTSTR GetNormalImage()
		{
			return m_sNormalImage;
		}
		void SetNormalImage(LPCTSTR pStrImage)
		{
			m_sNormalImage = pStrImage;
			Invalidate();
		}
		LPCTSTR GetHotImage()
		{
			return m_sHotImage;
		}
		void SetHotImage(LPCTSTR pStrImage)
		{
			m_sHotImage = pStrImage;
			Invalidate();
		}
		LPCTSTR GetPushedImage()
		{
			return m_sPushedImage;
		}
		void SetPushedImage(LPCTSTR pStrImage)
		{
			m_sPushedImage = pStrImage;
			Invalidate();
		}
		LPCTSTR GetFocusedImage()
		{
			return m_sFocusedImage;
		}
		void SetFocusedImage(LPCTSTR pStrImage)
		{
			m_sFocusedImage = pStrImage;
			Invalidate();
		}
		LPCTSTR GetDisabledImage()
		{
			return m_sDisabledImage;
		}
		void SetDisabledImage(LPCTSTR pStrImage)
		{
			m_sDisabledImage = pStrImage;
			Invalidate();
		}
		LPCTSTR GetForeImage()
		{
			return m_sForeImage;
		}
		void SetForeImage( LPCTSTR pStrImage )
		{
			m_sForeImage = pStrImage;
			Invalidate();
		}
		LPCTSTR GetHotForeImage()
		{
			return m_sHotForeImage;
		}
		void SetHotForeImage( LPCTSTR pStrImage )
		{
			m_sHotForeImage = pStrImage;
			Invalidate();
		}
		bool Activate()
		{
			if( !Control::Activate() )
				return false;
			if( m_pManager != NULL )
			{
				if (m_bButtonDBState == FALSE)
				{
					m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
				}
				else
				{
					m_bButtonDBState = FALSE ;
					m_pManager->SendNotify(this, DUI_MSGTYPE_DBCLICK);				
				}
			}
			return true;
		}
		void SetEnabled(bool bEnable)
		{
			Control::SetEnabled(bEnable);
			if( !IsEnabled() ) 
			{
				m_uButtonState = 0;
			}
		}
		UINT  GetControlFlags() const
		{
			return UIFLAG_SETCURSOR;
		}
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
		{
			if( _tcscmp(pstrName,_T("correct"))==0 )	m_Correct = _ttoi(pstrValue);
			else if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
			else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
			else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
			else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
			else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
			else if( _tcscmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
			else if( _tcscmp(pstrName, _T("hotforeimage")) == 0 ) SetHotForeImage(pstrValue);
			else if( _tcscmp(pstrName, _T("hotbkcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetHotBkColor(clrColor);
			}
			else if( _tcscmp(pstrName, _T("hottextcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetHotTextColor(clrColor);
			}
			else if( _tcscmp(pstrName, _T("pushedtextcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetPushedTextColor(clrColor);
			}
			else if( _tcscmp(pstrName, _T("focusedtextcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetFocusedTextColor(clrColor);
			}
			else if( _tcscmp(pstrName, _T("hotbordercolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetHotBorderColor(clrColor);
			}
			else
				Control::SetAttribute(pstrName,pstrValue);
		}
		LPVOID GetInterface(LPCTSTR pstrName)
		{
			if( _tcscmp(pstrName,GetClass())==0) 
				return this; 
			return Control::GetInterface(pstrName); 
		}
		LPCTSTR GetClass() const
		{
			return _T("ButtonUI");
		}
	protected:
		void DoEvent(DuiLib::TEventUI& event)
		{
			if( !IsMouseEnabled() && event.Type > DuiLib::UIEVENT__MOUSEBEGIN && event.Type < DuiLib::UIEVENT__MOUSEEND )
			{
				if( m_pParent != NULL )
					m_pParent->DoEvent(event);
				else 
					Control::DoEvent(event);
				return;
			}
			if( event.Type == DuiLib::UIEVENT_SETFOCUS ) 
			{
				Invalidate();
			}
			if( event.Type == DuiLib::UIEVENT_KILLFOCUS ) 
			{
				Invalidate();
			}
			if( event.Type == DuiLib::UIEVENT_KEYDOWN )
			{
				if (IsKeyboardEnabled())
				{
					if( event.chKey == VK_SPACE || event.chKey == VK_RETURN )
					{
						Activate();
						return;
					}
				}
			}		
			if( event.Type == DuiLib::UIEVENT_BUTTONDOWN )
			{
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) 
				{
					m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
					Invalidate();
				}
				return;
			}
			if( event.Type == DuiLib::UIEVENT_DBLCLICK) 
			{
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) 
				{
					m_bButtonDBState = TRUE ;
					m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
					Invalidate();
				}
				return;
			}	
			if( event.Type == DuiLib::UIEVENT_MOUSEMOVE )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 )
				{
					if( ::PtInRect(&m_rcItem, event.ptMouse) )
						m_uButtonState |= UISTATE_PUSHED;
					else
						m_uButtonState &= ~UISTATE_PUSHED;
					Invalidate();
				}
				return;
			}
			if( event.Type == DuiLib::UIEVENT_BUTTONUP )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) 
				{
					m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
					Invalidate();
					if( ::PtInRect(&m_rcItem, event.ptMouse) )
						Activate();
				}
				return;
			}
			if( event.Type == DuiLib::UIEVENT_CONTEXTMENU )
			{
				if( IsContextMenuUsed() )
				{
					m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
				}
				return;
			}
			if( event.Type == DuiLib::UIEVENT_MOUSEENTER )
			{
				if( IsEnabled() ) 
				{
					m_uButtonState |= UISTATE_HOT;
					Invalidate();
				}
				// return;
			}
			if( event.Type == DuiLib::UIEVENT_MOUSELEAVE )
			{
				if( IsEnabled() ) {
					m_uButtonState &= ~UISTATE_HOT;
					Invalidate();
				}
				// return;
			}
			if( event.Type == DuiLib::UIEVENT_SETCURSOR )
			{
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				return;
			}
			Control::DoEvent(event);
		}
		void PaintBorder(HDC hDC)
		{
			if(((m_uButtonState & UISTATE_HOT) != 0) && (GetHotBorderColor()!=0))
			{
				if(m_nBorderSize > 0 && ( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 ))//画圆角边框
				{
					DuiLib::CRenderEngine::DrawRoundRect(hDC, m_rcItem, m_nBorderSize, m_cxyBorderRound.cx, m_cxyBorderRound.cy, GetAdjustColor(m_dwHotBorderColor));
				}
				else
				{
					if (m_dwFocusBorderColor != 0 && m_nBorderSize > 0)
						DuiLib::CRenderEngine::DrawRect(hDC, m_rcItem, m_nBorderSize, GetAdjustColor(m_dwHotBorderColor),m_nBorderStyle);
					else if(m_rcBorderSize.left > 0 || m_rcBorderSize.top > 0 || m_rcBorderSize.right > 0 || m_rcBorderSize.bottom > 0)
					{
						RECT rcBorder;

						if(m_rcBorderSize.left > 0){
							rcBorder		= m_rcItem;
							rcBorder.right	= m_rcItem.left;
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.left,GetAdjustColor(m_dwHotBorderColor),m_nBorderStyle);
						}
						if(m_rcBorderSize.top > 0){
							rcBorder		= m_rcItem;
							rcBorder.bottom	= m_rcItem.top;
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.top,GetAdjustColor(m_dwHotBorderColor),m_nBorderStyle);
						}
						if(m_rcBorderSize.right > 0){
							rcBorder		= m_rcItem;
							rcBorder.left	= m_rcItem.right;
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.right,GetAdjustColor(m_dwHotBorderColor),m_nBorderStyle);
						}
						if(m_rcBorderSize.bottom > 0){
							rcBorder		= m_rcItem;
							rcBorder.top	= m_rcItem.bottom;
							DuiLib::CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.bottom,GetAdjustColor(m_dwHotBorderColor),m_nBorderStyle);
						}
					}
					else if(m_nBorderSize > 0)
						DuiLib::CRenderEngine::DrawRect(hDC, m_rcItem, m_nBorderSize, GetAdjustColor(m_dwHotBorderColor),m_nBorderStyle);
				}
			}
			else
			{
				Control::PaintBorder(hDC);
			}
		}
		void PaintStatusImage(HDC hDC)
		{
			if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
			else m_uButtonState &= ~ UISTATE_FOCUSED;
			if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
			else m_uButtonState &= ~ UISTATE_DISABLED;

			if( (m_uButtonState & UISTATE_DISABLED) != 0 )
			{
				if( !m_sDisabledImage.IsEmpty() )
				{
					if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
					else goto Label_ForeImage;
				}
			}
			else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) 
			{
				if( !m_sPushedImage.IsEmpty() )
				{
					if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage) )
					{
						m_sPushedImage.Empty();
					}
					if( !m_sPushedForeImage.IsEmpty() )
					{
						if( !DrawImage(hDC, (LPCTSTR)m_sPushedForeImage) )
							m_sPushedForeImage.Empty();
						return;
					}
					else goto Label_ForeImage;
				}
			}
			else if( (m_uButtonState & UISTATE_HOT) != 0 )
			{
				if( !m_sHotImage.IsEmpty() ) {
					if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) )
					{
						m_sHotImage.Empty();
					}
					if( !m_sHotForeImage.IsEmpty() ) 
					{
						if( !DrawImage(hDC, (LPCTSTR)m_sHotForeImage) )
							m_sHotForeImage.Empty();
						return;
					}
					else goto Label_ForeImage;
				}
				else if(m_dwHotBkColor != 0) 
				{
					DuiLib::CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwHotBkColor));
					return;
				}
			}
			else if( (m_uButtonState & UISTATE_FOCUSED) != 0 )
			{
				if( !m_sFocusedImage.IsEmpty() ) 
				{
					if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
					else goto Label_ForeImage;
				}
			}
			if( !m_sNormalImage.IsEmpty() ) 
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
				else goto Label_ForeImage;
			}
			if(!m_sForeImage.IsEmpty() )
				goto Label_ForeImage;
			return;
Label_ForeImage:
			if(!m_sForeImage.IsEmpty() ) 
			{
				if( !DrawImage(hDC, (LPCTSTR)m_sForeImage) ) m_sForeImage.Empty();
			}
		}
	protected:
		UINT m_uButtonState;
		bool m_bButtonDBState;

		int  m_Correct;	//修正值

		DWORD m_dwHotBkColor;
		DWORD m_dwHotTextColor;
		DWORD m_dwPushedTextColor;
		DWORD m_dwFocusedTextColor;
		DWORD m_dwHotBorderColor;

		CAtlString m_sNormalImage;
		CAtlString m_sHotImage;
		CAtlString m_sHotForeImage;
		CAtlString m_sPushedImage;
		CAtlString m_sPushedForeImage;
		CAtlString m_sFocusedImage;
		CAtlString m_sDisabledImage;
	};

	template<class Control>
	class TemplateClick : public ControlClick<Control>
	{
	public:
		virtual ~TemplateClick()
		{

		}
		SIZE EstimateSize(SIZE szAvailable)
		{
			//针对出现...的文本采用修正值调整宽度
			RECT rcText = { 0, 0, 9999, 9999 };
			if( IsShowHtml() )
			{
				int nLinks = 0;
				DuiLib::CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), 
					m_pManager, rcText, m_sText, m_dwTextColor, NULL, NULL, nLinks, 
					DT_CALCRECT | Control::GetTextStyle() );
			}
			else
			{
				DuiLib::CRenderEngine::DrawText(m_pManager->GetPaintDC(),
					m_pManager, rcText, Control::GetText(), 0, Control::GetFont(), DT_CALCRECT|Control::GetTextStyle());
			}
			SIZE ret;
			ret = Control::EstimateSize(szAvailable);
			long sy = szAvailable.cx - rcText.right;
			if( sy==0||sy<0 )
				ret.cx += m_Correct;
			else if( sy<abs(m_Correct) )
				ret.cx += sy+m_Correct;
			if( (rcText.right-rcText.left)<ret.cx )
				ret.cx = rcText.right-rcText.left;
			ret.cx += Control::GetTextPadding().left + Control::GetTextPadding().right;
			return ret;
		}
		void PaintText(HDC hDC)
		{
			bool bPushed = ((m_uButtonState & UISTATE_PUSHED) != 0);
			if(!bPushed)
			{
				Control::PaintText(hDC);
				return ;
			}
			if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
			else m_uButtonState &= ~ UISTATE_FOCUSED;
			if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
			else m_uButtonState &= ~ UISTATE_DISABLED;

			if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
			if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

			if( m_sText.IsEmpty() ) return;
			int nLinks = 0;
			RECT rc = m_rcItem;
			rc.left += m_rcTextPadding.left;
			rc.right -= m_rcTextPadding.right;
			rc.top += m_rcTextPadding.top;
			rc.bottom -= m_rcTextPadding.bottom;

			if( bPushed )
			{
				rc.top=rc.top+1;
				rc.bottom=rc.bottom+1;
			}
			DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;
			if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
				clrColor = GetPushedTextColor();
			else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
				clrColor = GetHotTextColor();
			else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
				clrColor = GetFocusedTextColor();
			if( m_bShowHtml )
				DuiLib::CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, clrColor, \
				NULL, NULL, nLinks, GetTextStyle() );
			else
				DuiLib::CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, clrColor, \
				m_iFont, GetTextStyle() );
		}
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
		{
			if( _tcscmp(pstrName,_T("correct"))==0 )	m_Correct = _ttoi(pstrValue);
			else if( _tcscmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
			else if( _tcscmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
			else if( _tcscmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
			else if( _tcscmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
			else if( _tcscmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
			else if( _tcscmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
			else if( _tcscmp(pstrName, _T("hotforeimage")) == 0 ) SetHotForeImage(pstrValue);
			else if( _tcscmp(pstrName, _T("hotbkcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetHotBkColor(clrColor);
			}
			else if( _tcscmp(pstrName, _T("hottextcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetHotTextColor(clrColor);
			}
			else if( _tcscmp(pstrName, _T("pushedtextcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetPushedTextColor(clrColor);
			}
			else if( _tcscmp(pstrName, _T("focusedtextcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetFocusedTextColor(clrColor);
			}
			else if( _tcscmp(pstrName, _T("hotbordercolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetHotBorderColor(clrColor);
			}
			else if( _tcscmp(pstrName, _T("singleline")) == 0 )
			{
				Control::SetTextStyle(Control::GetTextStyle()|DT_SINGLELINE);
			}
			else
				Control::SetAttribute(pstrName,pstrValue);
		}
	};
}