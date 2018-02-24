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
	}
	void ListUI::ScrollAt(DuiLib::CControlUI *item)
	{
		SIZE szScrollPos = GetScrollPos();
		if( this->GetScrollRange().cy<=0 )
		{
			this->SetScrollPos(szScrollPos);
			return ;
		}
		bool bHas = false;
		int nAdjustables = 0;
		int nEstimateNum = 0;
		long cyFixed = 0;
		//计算有效区域
		RECT rcPos = GetPos();
		rcPos.left += GetInset().left;
		rcPos.top += GetInset().top;
		rcPos.right -= GetInset().right;
		rcPos.bottom -= GetInset().bottom;
		SIZE szAvailable = { rcPos.right - rcPos.left, rcPos.bottom - rcPos.top };
		//计算高度
		for(int n=0;n<GetCount();n++)
		{
			DuiLib::CControlUI *inner = GetItemAt(n);
			if(inner==item)
			{
				bHas = true;
				break;
			}
			if( !inner->IsVisible() ) continue;
			if( inner->IsFloat() ) continue;
			SIZE sz = inner->EstimateSize(szAvailable);
			if( sz.cy == 0 ) {
				nAdjustables++;
			}
			else {
				if( sz.cy < inner->GetMinHeight() ) sz.cy = inner->GetMinHeight();
				if( sz.cy > inner->GetMaxHeight() ) sz.cy = inner->GetMaxHeight();
			}
			cyFixed += sz.cy + inner->GetPadding().top + inner->GetPadding().bottom;
			nEstimateNum++;
		}
		cyFixed += (nEstimateNum - 1) * GetChildPadding();
		if(bHas)
		{
			szScrollPos.cy = cyFixed;
			this->SetScrollPos(szScrollPos);
		}		
	}
}