#include "StdAfx.h"
#include "UIlib.h"
#include "ListExUI.h"
#include "../baseX.h"
namespace CustomUI
{
	ListUI::ListUI(void)
	{
		rcBkImg.left=0;
		rcBkImg.top =0;
		rcBkImg.right=0;
		rcBkImg.bottom=0;
	}
	RECT ListUI::GetImgDest(LPCTSTR img)
	{
		CAtlString stBkImage(img);
		RECT ret={0,0,0,0};
		long nDest = stBkImage.Find(_T("dest"));
		if(nDest==-1)	return ret;
		LPTSTR pstr = NULL;
		CAtlString sDest;
		for(int n=nDest;1;n++)
		{
			TCHAR d = stBkImage[n];
			if( (d>='0' && d<='9')||d==',' )
			{
				sDest += d;
				continue;
			}
			if(d==' ' || n>=stBkImage.GetLength())
				break;
		}
		ret.left = _tcstol(sDest, &pstr, 10);  ASSERT(pstr);    
		ret.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
		ret.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		ret.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		return ret;
	}
	void ListUI::SetBkImage(LPCTSTR pStrImage)
	{
		rcBkImg = GetImgDest(pStrImage);
		__super::SetBkImage(pStrImage);
	}
	//计算宽高比例
	void ListUI::RatioW(RECT rc,double inW,double &outW)
	{
		double rcW = rc.right - rc.left;
		if( rc.right == rc.left )
			return ;
		outW = inW/rcW;
	}
	void ListUI::RatioH(RECT rc,double inH,double &outH)
	{
		double rcH = rc.bottom - rc.top;
		if( rc.bottom == rc.top )
			return ;
		outH = inH/rcH;
	}
	void ListUI::ReBkImgDest(RECT rc)
	{
		long nDest = m_sBkImage.Find(_T("dest"));
		DuiLib::CDuiString sFile = m_sBkImage.Mid(0,nDest);
		CAtlString sDest;
		sDest.Format(_T("dest='%d,%d,%d,%d'"),rc.left,rc.top,rc.right,rc.bottom);
		sFile += sDest.GetString();
		m_sBkImage = sFile;
	}
	void ListUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		RECT rcCur = GetPos();
		RECT rcBkImgAd = rcBkImg;
		double bkBlL=0.0f,bkBlT=0.0f;
		if(rcBkImgAd.left!=0)
		{
			RatioW(rcCur,rcBkImgAd.left,bkBlL);
		}
		if(rcBkImgAd.top!=0)
		{
			RatioH(rcCur,rcBkImgAd.top,bkBlT);
		}
		//
		__super::SetPos(rc);
		m_pList->SetPos(m_pList->GetPos());

		//重新计算背景图位置
		rcCur = GetPos();
		double rcW = rcCur.right - rcCur.left;
		double rcH = rcCur.bottom - rcCur.top;
		if(rcBkImgAd.left!=0)
		{
			if(bkBlL<=0.0000000001f )
				rcBkImgAd.left = rcBkImg.left;
			else
				rcBkImgAd.left = rcW*bkBlL;
		}
		if(rcBkImgAd.top!=0)
		{
			if(bkBlT<=0.0000000001f)
				rcBkImgAd.top = rcBkImg.top;
			else
				rcBkImgAd.top = rcH*bkBlT;
		}
		if(rcBkImgAd.left || rcBkImgAd.top)
		{
			rcBkImgAd.right = rcBkImgAd.left + (rcBkImg.right-rcBkImg.left);
			rcBkImgAd.bottom= rcBkImgAd.top + (rcBkImg.bottom-rcBkImg.top);
			rcBkImg = rcBkImgAd;
			ReBkImgDest(rcBkImg);
		}
	}
	void ListUI::PaintBkImage(HDC hDC)
	{
		__super::PaintBkImage(hDC);
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
	void ListUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("bkimage")) == 0 ) 
		{
			SetBkImage(pstrValue);
			return ;
		}
		__super::SetAttribute(pstrName,pstrValue);
	}
	/*--------------------------------------------------------------------*/
	ListHeaderItemUI::ListHeaderItemUI()
	{
		mList = NULL;
		memset(&mEstSize,0,sizeof(SIZE));
		memset(&mAvailableSize,0,sizeof(SIZE));
	}
	void ListHeaderItemUI::LoopSetOwner(DuiLib::CControlUI* pControl)
	{
		DuiLib::CContainerUI *lay = base::SafeConvert<DuiLib::CContainerUI>(pControl,DUI_CTR_CONTAINER);
		if(lay)
		{
			for(int n=0;n<lay->GetCount();n++)
			{
				CustomUI::ListHeaderItemUI *headItem = base::SafeConvert<CustomUI::ListHeaderItemUI>(lay->GetItemAt(n),_T("CustomUI::ListHeaderItemUI"));
				if(headItem)
					headItem->SetOwner(mList);
				else
					LoopSetOwner(lay->GetItemAt(n));
			}
		}
	}
	void ListHeaderItemUI::SetOwner(DuiLib::CListUI *listUI)
	{
		mList = listUI;
		SetChildrenOwner();
	}
	void ListHeaderItemUI::SetChildrenOwner()
	{
		for(int n=0;n<GetCount();n++)
		{
			CustomUI::ListHeaderItemUI *headItem = base::SafeConvert<CustomUI::ListHeaderItemUI>(GetItemAt(n),_T("CustomUI::ListHeaderItemUI"));
			if(headItem)
				headItem->SetOwner(base::SafeConvert<DuiLib::CListUI>(GetParent(),DUI_CTR_LIST));
			else
				LoopSetOwner(GetItemAt(n));
		}
	}
	bool ListHeaderItemUI::Add(DuiLib::CControlUI* pControl)
	{
		CustomUI::ListHeaderItemUI *headItem = base::SafeConvert<CustomUI::ListHeaderItemUI>(pControl,_T("CustomUI::ListHeaderItemUI"));
		if(headItem)
		{
			if(mList)
				headItem->SetOwner(mList);
		}
		else
		{
			LoopSetOwner(pControl);
		}
		return __super::Add(pControl);
	}
	void ListHeaderItemUI::Init()
	{
		__super::Init();
		SetChildrenOwner();
	}
	SIZE ListHeaderItemUI::EstimateSize(SIZE szAvailable)
	{
		/*
		SIZE adjSize = m_cxyFixed;
		if(adjSize.cy==0)
			adjSize.cy =  m_pManager->GetDefaultFontInfo()->tm.tmHeight + 14;
		if(mAvailableSize.cx==0 && m_cxyMin.cx && adjSize.cx==0)
		{
			mAvailableSize = szAvailable;
			if(adjSize.cx<m_cxyMin.cx)
				adjSize.cx = m_cxyMin.cx;
			mEstSize = adjSize;
			//产生固定比
			mRatX = (float)mAvailableSize.cx/(float)m_cxyMin.cx;
			return mEstSize;
		}
		if(mAvailableSize.cx==szAvailable.cx && mEstSize.cx && adjSize.cx==0)
		{
			return mEstSize;
		}
		if(mAvailableSize.cx!=szAvailable.cx && m_cxyMin.cx && adjSize.cx==0 )
		{
			if(mRatX<0.00001)
				adjSize.cx = m_cxyMin.cx;
			else
			{
				long ratW = (szAvailable.cx - mAvailableSize.cx)/mRatX;
				if(mEstSize.cx)
					mEstSize.cx += ratW;
				else
					mEstSize.cx = m_cxyMin.cx + ratW;
				if(mEstSize.cx<m_cxyMin.cx)
					mEstSize.cx = m_cxyMin.cx;
				adjSize = mEstSize;
			}
			mAvailableSize = szAvailable;
			mEstSize = adjSize;
			return mEstSize;
		}*/
		if( m_cxyFixed.cy == 0 ) 
		{
			return DuiLib::CDuiSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 14);
		}
		return CContainerUI::EstimateSize(szAvailable);
	}
	void ListHeaderItemUI::DoEvent(DuiLib::TEventUI& event)
	{
		if( event.Type == DuiLib::UIEVENT_BUTTONUP )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) 
			{
				m_uButtonState &= ~UISTATE_CAPTURED;
				if(mList)
				{
					mList->NeedUpdate();
				}
				else if( GetParent() ) 
				{
					GetParent()->NeedParentUpdate();
				}
			}
			else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) 
			{
				m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if( event.Type == DuiLib::UIEVENT_MOUSEMOVE )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 )
			{
				RECT rc = m_rcItem;
				if( m_iSepWidth >= 0 ) 
				{
					rc.right -= ptLastMouse.x - event.ptMouse.x;
				}
				else 
				{
					rc.left -= ptLastMouse.x - event.ptMouse.x;
				}
				if( rc.right - rc.left > GetMinWidth() ) 
				{
					m_cxyFixed.cx = rc.right - rc.left;
					ptLastMouse = event.ptMouse;
					if(mList)
					{
						mList->NeedUpdate();
					}
					else if( GetParent() ) 
					{
						GetParent()->NeedParentUpdate();
					}
				}
			}
			return;
		}
		__super::DoEvent(event);
	}
	/*--------------------------------------------------------------------*/
	CListHeaderUI::CListHeaderUI()
	{

	}
	bool CListHeaderUI::Add(DuiLib::CControlUI* pControl)
	{
		CustomUI::ListHeaderItemUI *headItem = base::SafeConvert<CustomUI::ListHeaderItemUI>(pControl,_T("CustomUI::ListHeaderItemUI"));
		if(headItem)
		{
			headItem->SetOwner(base::SafeConvert<DuiLib::CListUI>(GetParent(),DUI_CTR_LIST));
		}
		return __super::Add(pControl);
	}
	void CListHeaderUI::Init()
	{
		__super::Init();
		for(int n=0;n<GetCount();n++)
		{
			CustomUI::ListHeaderItemUI *headItem = base::SafeConvert<CustomUI::ListHeaderItemUI>(GetItemAt(n),_T("CustomUI::ListHeaderItemUI"));
			if(headItem)
			{
				headItem->SetOwner( GetListUI() );
			}
		}
	}
	DuiLib::CListUI* CListHeaderUI::GetListUI()
	{
		return base::SafeConvert<DuiLib::CListUI>(GetParent(),DUI_CTR_LIST);
	}
	long CListHeaderUI::SubCtrSizeForRaw()
	{
		long ret = 0;
		std::map<DuiLib::CControlUI*,SIZE>::iterator it = mCtrRawSize.begin();
		std::vector<DuiLib::CControlUI*> remove;
		for(it;it!=mCtrRawSize.end();it++)
		{
			if(!it->first->IsFloat() && it->first->IsVisible())
				ret += it->second.cx;
		}
		return ret;
	}
	void CListHeaderUI::GetValidCtrForRaw(std::vector<NEED_SIZE> &out,SIZE szBlank)
	{
		std::map<DuiLib::CControlUI*,SIZE>::iterator it = mCtrRawSize.begin();
		std::vector<DuiLib::CControlUI*> remove;
		for(it;it!=mCtrRawSize.end();it++)
		{
			DuiLib::CControlUI *ui = it->first;
			if(!ui->IsFloat() && ui->IsVisible())
			{
				NEED_SIZE one;
				SIZE szNeed = ui->EstimateSize(szBlank);
				if(szNeed.cx < ui->GetMaxWidth() || ui->GetMaxWidth()==0)
				{
					one.sz = szNeed;
					one.ui = ui;
					out.push_back(one);
				}
			}
		}
	}
	SIZE CListHeaderUI::EstimateSize(SIZE szAvailable)
	{
		SIZE  sz = __super::EstimateSize(szAvailable);
		//记录每个控件需要的size
		if(mCtrRawSize.size()==0)
		{
			//存储原始宽度
			int nBlankX = szAvailable.cx-sz.cx;
			for(int n=0;n<GetCount();n++)
			{
				DuiLib::CControlUI *ctr = GetItemAt(n);
				SIZE szNeed = ctr->EstimateSize(szAvailable);
				//if( nBlankX && n==(GetCount()-1))
				//	szNeed.cx += nBlankX;
				mCtrRawSize.insert( std::make_pair(ctr,szNeed) );
			}
			return sz;
		}

		long remainX = szAvailable.cx - sz.cx;
		if(remainX==0)	return sz;
		long rawX = SubCtrSizeForRaw();
		if(rawX==0) return sz;
		DuiLib::CListUI* listUI = GetListUI();
		if(listUI->GetVerticalScrollBar() && listUI->GetVerticalScrollBar()->IsVisible() && remainX<0)
		{
			if(abs(remainX) == listUI->GetVerticalScrollBar()->GetFixedWidth())
				return sz;
		}
		std::vector<NEED_SIZE> adjCtr;
		GetValidCtrForRaw(adjCtr,szAvailable);
		if(adjCtr.size()==0)	return sz;

		sz.cx = szAvailable.cx;
		if(rawX==szAvailable.cx)
		{
			for(int n=0;n<adjCtr.size();n++)
			{
				NEED_SIZE *one = &adjCtr.at(n);
				SIZE szRaw = mCtrRawSize[one->ui];
				one->ui->SetFixedWidth(szRaw.cx);
			}
			return sz;
		}
		int nPerX = remainX/(int)adjCtr.size();
		if(nPerX==0 && adjCtr.size() )
		{
			nPerX = remainX%(int)adjCtr.size();
			NEED_SIZE *one = &adjCtr.at( adjCtr.size()-1 );
			SIZE szRaw = mCtrRawSize[one->ui];
			int nX = one->sz.cx+nPerX;
			if(nX < szRaw.cx)
				nX = szRaw.cx;
			one->ui->SetFixedWidth(nX);
			return sz;
		}
		else if(remainX>0)
		{
			for(int n=0;n<adjCtr.size() && remainX>0 && nPerX;n++)
			{
				NEED_SIZE *one = &adjCtr.at(n);
				if(remainX-nPerX)
				{
					one->sz.cx += nPerX;
				}
				else
				{
					one->sz.cx += remainX;
				}
				remainX -= nPerX;
				one->ui->SetFixedWidth(one->sz.cx);
			}
		}
		else
		{
			for(int n=0;n<adjCtr.size() && remainX<0 && nPerX;n++)
			{
				NEED_SIZE *one = &adjCtr.at(n);
				SIZE szRaw = mCtrRawSize[one->ui];
				if(remainX-nPerX)
				{
					one->sz.cx += nPerX;
				}
				else
				{
					one->sz.cx += remainX;
				}
				remainX -= nPerX;
				if(one->sz.cx < szRaw.cx)
					one->sz.cx = szRaw.cx;
				one->ui->SetFixedWidth(one->sz.cx);
			}
		}
		return sz;
	}
}