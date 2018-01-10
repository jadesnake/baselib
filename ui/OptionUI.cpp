#include "StdAfx.h"
#include "OptionUI.h"
namespace CustomUI
{
	OptionUI::OptionUI(void)
	{
	}
	OptionUI::~OptionUI(void)
	{

	}
	void OptionUI::PaintStatusImage(HDC hDC)
	{
		if( (m_uButtonState & UISTATE_PUSHED) != 0 && IsSelected() && !m_sSelectedPushedImage.IsEmpty())
		{
			if( !__super::DrawImage(hDC, (LPCTSTR)m_sSelectedPushedImage) )
				m_sSelectedPushedImage.Empty();
			else goto Label_ForeImage;
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 && IsSelected() && !m_sSelectedHotImage.IsEmpty()) 
		{
			if( !__super::DrawImage(hDC, (LPCTSTR)m_sSelectedHotImage) )
				m_sSelectedHotImage.Empty();
			else goto Label_ForeImage;
		}
		else if( (m_uButtonState & UISTATE_SELECTED) != 0 )
		{
			if(m_dwSelectedBkColor != 0)
			{
				DuiLib::CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwSelectedBkColor));
			}
			if( !m_sSelectedImage.IsEmpty() ) 
			{
				if( !__super::DrawImage(hDC, (LPCTSTR)m_sSelectedImage) ) 
					m_sSelectedImage.Empty();
				else
					goto Label_ForeImage;
			}			
		}
		__super::PaintStatusImage(hDC);
Label_ForeImage:
		if( !m_sForeImage.IsEmpty() )
		{
			if( !__super::DrawImage(hDC, (LPCTSTR)m_sForeImage) )
				m_sForeImage.Empty();
		}
	}
}
