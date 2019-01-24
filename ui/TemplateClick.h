#pragma once

#include "../Base/MacroX.h"
namespace CustomUI
{
	template<class Control>
	class ControlClick : public Control
	{
	public:
		ControlClick()
			: m_uButtonState(0)
			, m_iHotFont(-1)
			, m_iPushedFont(-1)
			, m_iFocusedFont(-1)
			, m_dwHotTextColor(0)
			, m_dwPushedTextColor(0)
			, m_dwFocusedTextColor(0)
			, m_dwHotBkColor(0)
			, m_dwPushedBkColor(0)
			, m_dwDisabledBkColor(0)
			, m_iBindTabIndex(-1)
			, m_nStateCount(0)
		{
			m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		}
		virtual ~ControlClick()
		{

		}
		UINT GetControlFlags() const
		{
			return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
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
		void SetNormalImage(LPCTSTR pStrImage)
		{
			m_sNormalImage = pStrImage;
			Invalidate();
		}
		LPCTSTR GetNormalImage()
		{
			return m_sNormalImage;
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
			if( !Control::Activate() ) return false;
 			if( m_pManager != NULL )
			{
				m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
				BindTriggerTabSel();
			}
			return true;
		}
		void SetEnabled(bool bEnable)
		{
			Control::SetEnabled(bEnable);
			if( !IsEnabled() ) {
				m_uButtonState = 0;
			}
		}
		void  SetHotFont(int index)
		{
			m_iHotFont = index;
			Invalidate();
		}
		int  GetHotFont() const
		{
			return m_iHotFont;
		}
		void SetFocusedFont(int index)
		{
			m_iFocusedFont = index;
			Invalidate();
		}
		int GetFocusedFont() const
		{
			return m_iFocusedFont;
		}
		void SetPushedBkColor( DWORD dwColor )
		{
			m_dwPushedBkColor = dwColor;
			Invalidate();
		}
		DWORD GetPushedBkColor() const
		{
			return m_dwPushedBkColor;
		}
		void SetDisabledBkColor( DWORD dwColor )
		{
			m_dwDisabledBkColor = dwColor;
			Invalidate();
		}
		DWORD GetDisabledBkColor() const
		{
			return m_dwDisabledBkColor;
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
		LPCTSTR GetFocusedImage()
		{
			return m_sFocusedImage;
		}
		void  SetFocusedImage(LPCTSTR pStrImage)
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
		void SetStateCount(int nCount)
		{
			m_nStateCount = nCount;
			Invalidate();
		}
		int GetStateCount() const
		{
			return m_nStateCount;
		}
		LPCTSTR GetStateImage()
		{
			return m_sStateImage;
		}
		void SetStateImage( LPCTSTR pStrImage )
		{
			m_sNormalImage.Empty();
			m_sStateImage = pStrImage;
			Invalidate();
		}
		void BindTabIndex(int _BindTabIndex )
		{
			if( _BindTabIndex >= 0)
				m_iBindTabIndex	= _BindTabIndex;
		}
		void BindTabLayoutName( LPCTSTR _TabLayoutName )
		{
			if(_TabLayoutName)
				m_sBindTabLayoutName = _TabLayoutName;
		}
		void BindTriggerTabSel( int _SetSelectIndex = -1 )
		{
			LPCTSTR pstrName = GetBindTabLayoutName();
			if(pstrName == NULL || (GetBindTabLayoutIndex() < 0 && _SetSelectIndex < 0))
				return;

			DuiLib::CTabLayoutUI* pTabLayout = static_cast<DuiLib::CTabLayoutUI*>(GetManager()->FindControl(pstrName));
			if(!pTabLayout) return;
			pTabLayout->SelectItem(_SetSelectIndex >=0?_SetSelectIndex:GetBindTabLayoutIndex());
		}
		void RemoveBindTabIndex()
		{
			m_iBindTabIndex	= -1;
			m_sBindTabLayoutName.Empty();
		}
		int GetBindTabLayoutIndex()
		{
			return m_iBindTabIndex;
		}
		LPCTSTR GetBindTabLayoutName()
		{
			return m_sBindTabLayoutName;
		}
		void SetPushedFont(int index)
		{
			m_iPushedFont = index;
			Invalidate();
		}
		int  GetPushedFont() const
		{
			return m_iPushedFont;
		}		 
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
		{
			if( _tcsicmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
			else if( _tcsicmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
			else if( _tcsicmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
			else if( _tcsicmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
			else if( _tcsicmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
			else if( _tcsicmp(pstrName, _T("hotforeimage")) == 0 ) SetHotForeImage(pstrValue);
			else if( _tcsicmp(pstrName, _T("stateimage")) == 0 ) SetStateImage(pstrValue);
			else if( _tcsicmp(pstrName, _T("statecount")) == 0 ) SetStateCount(_ttoi(pstrValue));
			else if( _tcsicmp(pstrName, _T("bindtabindex")) == 0 ) BindTabIndex(_ttoi(pstrValue));
			else if( _tcsicmp(pstrName, _T("bindtablayoutname")) == 0 ) BindTabLayoutName(pstrValue);
			else if( _tcsicmp(pstrName, _T("hotbkcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetHotBkColor(clrColor);
			}
			else if( _tcsicmp(pstrName, _T("pushedbkcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetPushedBkColor(clrColor);
			}
			else if( _tcsicmp(pstrName, _T("disabledbkcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetDisabledBkColor(clrColor);
			}
			else if( _tcsicmp(pstrName, _T("hottextcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetHotTextColor(clrColor);
			}
			else if( _tcsicmp(pstrName, _T("pushedtextcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetPushedTextColor(clrColor);
			}
			else if( _tcsicmp(pstrName, _T("focusedtextcolor")) == 0 )
			{
				if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
				LPTSTR pstr = NULL;
				DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
				SetFocusedTextColor(clrColor);
			}
			else if( _tcsicmp(pstrName, _T("hotfont")) == 0 ) SetHotFont(_ttoi(pstrValue));
			else if( _tcsicmp(pstrName, _T("pushedfont")) == 0 ) SetPushedFont(_ttoi(pstrValue));
			else if( _tcsicmp(pstrName, _T("focuedfont")) == 0 ) SetFocusedFont(_ttoi(pstrValue));
			else Control::SetAttribute(pstrName, pstrValue);
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
			if( !IsMouseEnabled() && event.Type > DuiLib::UIEVENT__MOUSEBEGIN && event.Type < DuiLib::UIEVENT__MOUSEEND ) {
				if( m_pParent != NULL ) m_pParent->DoEvent(event);
				else Control::DoEvent(event);
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
				if (IsKeyboardEnabled()) {
					if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
						Activate();
						return;
					}
				}
			}		
			if( event.Type == DuiLib::UIEVENT_BUTTONDOWN || event.Type == DuiLib::UIEVENT_DBLCLICK)
			{
				if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
					m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
					Invalidate();
				}
				return;
			}	
			if( event.Type == DuiLib::UIEVENT_MOUSEMOVE )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					if( ::PtInRect(&m_rcItem, event.ptMouse) ) m_uButtonState |= UISTATE_PUSHED;
					else m_uButtonState &= ~UISTATE_PUSHED;
					Invalidate();
				}
				return;
			}
			if( event.Type == DuiLib::UIEVENT_BUTTONUP )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
					Invalidate();
					if( ::PtInRect(&m_rcItem, event.ptMouse) ) Activate();				
				}
				return;
			}
			if( event.Type == DuiLib::UIEVENT_CONTEXTMENU )
			{
				if( IsContextMenuUsed() ) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
				}
				return;
			}
			if( event.Type == DuiLib::UIEVENT_SETCURSOR )
			{
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				return;
			}
			if( event.Type == DuiLib::UIEVENT_MOUSEENTER )
			{
				if( IsEnabled() ) {
					m_uButtonState |= UISTATE_HOT;
					Invalidate();
				}
			}
			if( event.Type == DuiLib::UIEVENT_MOUSELEAVE )
			{
				if( IsEnabled() ) {
					m_uButtonState &= ~UISTATE_HOT;
					Invalidate();
				}
			}
 			Control::DoEvent(event);
		}
		void PaintStatusImage(HDC hDC)
		{
			if(!m_sStateImage.IsEmpty() && m_nStateCount > 0)
			{
				DuiLib::TDrawInfo info;
				info.Parse(m_sStateImage, _T(""), m_pManager);
				const DuiLib::TImageInfo* pImage = m_pManager->GetImageEx(info.sImageName, info.sResType, info.dwMask, info.bHSL);
				if(m_sNormalImage.IsEmpty() && pImage != NULL)
				{
					SIZE szImage = {pImage->nX, pImage->nY};
					SIZE szStatus = {pImage->nX / m_nStateCount, pImage->nY};
					if( szImage.cx > 0 && szImage.cy > 0 )
					{
						RECT rcSrc = {0, 0, szImage.cx, szImage.cy};
						if(m_nStateCount > 0) {
							int iLeft = rcSrc.left + 0 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sNormalImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
						}
						if(m_nStateCount > 1) {
							int iLeft = rcSrc.left + 1 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sHotImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
							m_sPushedImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
						}
						if(m_nStateCount > 2) {
							int iLeft = rcSrc.left + 2 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sPushedImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
						}
						if(m_nStateCount > 3) {
							int iLeft = rcSrc.left + 3 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sDisabledImage.Format(_T("res='%s' restype='%s' dest='%d,%d,%d,%d' source='%d,%d,%d,%d'"), info.sImageName.GetData(), info.sResType.GetData(), info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom, iLeft, iTop, iRight, iBottom);
						}
					}
				}
			}
			if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
			else m_uButtonState &= ~ UISTATE_FOCUSED;
			if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
			else m_uButtonState &= ~ UISTATE_DISABLED;
			if(!::IsWindowEnabled(m_pManager->GetPaintWindow())) {
				m_uButtonState &= UISTATE_DISABLED;
			}
			if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
				if( !m_sDisabledImage.IsEmpty() ) {
					if( !DrawImage(hDC, (LPCTSTR)m_sDisabledImage) ) {}
					else return;
				}
			}
			else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
				if( !m_sPushedImage.IsEmpty() ) {
					if( !DrawImage(hDC, (LPCTSTR)m_sPushedImage) ) {}
					else return;
				}
			}
			else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
				if( !m_sHotImage.IsEmpty() ) {
					if( !DrawImage(hDC, (LPCTSTR)m_sHotImage) ) {}
					else return;
				}
			}
			else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
				if( !m_sFocusedImage.IsEmpty() ) {
					if( !DrawImage(hDC, (LPCTSTR)m_sFocusedImage) ) {}
					else return;
				}
			}
			if( !m_sNormalImage.IsEmpty() ) {
				if( !DrawImage(hDC, (LPCTSTR)m_sNormalImage) ) {}
			}
		}
		void PaintBkColor(HDC hDC)
		{
			if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
				if(m_dwDisabledBkColor != 0) {
					DuiLib::CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwDisabledBkColor));
					return;
				}
			}
			else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
				if(m_dwPushedBkColor != 0) {
					DuiLib::CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwPushedBkColor));
					return;
				}
			}
			else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
				if(m_dwHotBkColor != 0) {
					DuiLib::CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwHotBkColor));
					return;
				}
			}
			return Control::PaintBkColor(hDC);
		}
	protected:
		UINT m_uButtonState;
		UINT m_uTextStyle;
		int	 m_iHotFont;
		int	 m_iPushedFont;
		int	 m_iFocusedFont;

		DWORD m_dwHotBkColor;
		DWORD m_dwPushedBkColor;
		DWORD m_dwDisabledBkColor;
		DWORD m_dwHotTextColor;
		DWORD m_dwPushedTextColor;
		DWORD m_dwFocusedTextColor;

		CAtlString m_sNormalImage;
		CAtlString m_sHotImage;
		CAtlString m_sHotForeImage;
		CAtlString m_sPushedImage;
		CAtlString m_sPushedForeImage;
		CAtlString m_sFocusedImage;
		CAtlString m_sDisabledImage;
		int m_nStateCount;
		CAtlString m_sStateImage;

		int			m_iBindTabIndex;
		CAtlString	m_sBindTabLayoutName;
	};

	template<class Control>
	class TemplateClick : public ControlClick<Control>
	{
	public:
		TemplateClick()
		{
			Control::SetAttribute(_T("autocalcwidth"),_T("true"));
		}
		virtual ~TemplateClick()
		{

		}
		SIZE EstimateSize(SIZE szAvailable)
		{
			return Control::EstimateSize(szAvailable);
		}
		void PaintText(HDC hDC)
		{
			bool bPushed = ((m_uButtonState & UISTATE_PUSHED) != 0);
			if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
			else m_uButtonState &= ~ UISTATE_FOCUSED;
			if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
			else m_uButtonState &= ~ UISTATE_DISABLED;

			if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
			if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

			DuiLib::CDuiString sText = GetText();
			if( sText.IsEmpty() ) return;

			RECT m_rcTextPadding =  Control::m_rcTextPadding;
			GetManager()->GetDPIObj()->Scale(&m_rcTextPadding);
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

			int iFont = GetFont();
			if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedFont() != -1) )
				iFont = GetPushedFont();
			else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotFont() != -1) )
				iFont = GetHotFont();
			else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedFont() != -1) )
				iFont = GetFocusedFont();

			if( m_bShowHtml )
				DuiLib::CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, sText, clrColor, \
				NULL, NULL, nLinks, iFont, m_uTextStyle);
			else
				DuiLib::CRenderEngine::DrawText(hDC, m_pManager, rc, sText, clrColor, \
				iFont, m_uTextStyle);
		}
 	};
}