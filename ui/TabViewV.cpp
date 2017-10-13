#include "stdafx.h"
#include "TabViewV.h"
#include "../base/baseX.h"
namespace CustomUI
{
	DuiLib::CControlUI*	GetLayoutNeighbor(DuiLib::CContainerUI *layout,DuiLib::CControlUI* pCtrl)
	{
		if( layout == NULL || pCtrl == NULL)
			return NULL;
		int nNum = layout->GetCount();
		for(int nI = 0;nI < nNum;nI++)
		{
			if( pCtrl == layout->GetItemAt(nI) )
			{
				if( nI == 0 )
					return layout->GetItemAt(nI+1);
				return layout->GetItemAt(nI-1);
			}
		}
		return NULL;
	}

	TabViewV::TabViewV()
	{
		m_pLable = new DuiLib::CHorizontalLayoutUI;
		m_pTable = new DuiLib::CTabLayoutUI;
		m_pTable->OnNotify += DuiLib::MakeDelegate(this,&TabViewV::OnTablayoutChange);
		__super::Add(m_pLable);
		__super::Add(m_pTable);
	}
	TabViewV::~TabViewV()
	{

	}
	DuiLib::CTabLayoutUI*	TabViewV::GetTabLayout()
	{
		return m_pTable;
	}
	bool TabViewV::OnTablayoutChange(void *param)
	{
		DuiLib::TNotifyUI *nt = static_cast<DuiLib::TNotifyUI*>(param);
		if( nt->sType == DUI_MSGTYPE_TABSELECT )
		{
			DuiLib::CControlUI *ctrl = nt->pSender;
			nt->pSender = this;
			OnSwitch(param);
			nt->pSender = ctrl;
		}
		return true;
	}
	DuiLib::CHorizontalLayoutUI*	TabViewV::GetLableLayout()
	{
		return m_pLable;
	}
	bool TabViewV::Add(DuiLib::CControlUI *pCtl)
	{
		if( (pCtl == m_pLable) || (pCtl == m_pTable) )
			return true;
		if( 0 == _tcscmp(pCtl->GetClass(),_T("Page"))  )
		{
			TabViewV::Page *pPage = static_cast<TabViewV::Page*>(pCtl);
			pPage->SetTabView(this);
			return m_pLable->Add(pCtl);
		}
		else
			return m_pTable->Add(pCtl);
	}
	void TabViewV::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( 0 == _tcscmp(pstrName,_T("tabattr")) )
			m_pTable->ApplyAttributeList(pstrValue);
		else if( 0 == _tcscmp(pstrName,_T("titleattr")) )
			m_pLable->ApplyAttributeList(pstrValue);
		else if( 0 == _tcscmp(pstrName,_T("scrollbar")) )
			m_pLable->SetAttribute(pstrName,pstrValue);
		else if( 0 == _tcscmp(pstrName,_T("scrollbtn1")) )
			m_pLable->SetAttribute(_T("scrollbtn1"),pstrValue);
		else if( 0 == _tcscmp(pstrName,_T("scrollbtn2")) )
			m_pLable->SetAttribute(_T("scrollbtn2"),pstrValue);
		else
			__super::SetAttribute(pstrName,pstrValue);
	}
	void TabViewV::AddPageView(DuiLib::CControlUI *pView)
	{
		m_pTable->Add(pView);
		Invalidate();
	}
	void TabViewV::ClearPageView(DuiLib::CControlUI *pView)
	{
		m_pTable->Remove(pView);
		Invalidate();
	}
	TabViewV::Page*	TabViewV::AddPage(DuiLib::CControlUI *pView)
	{
		Page *pVal = 0;
		pVal = new Page(this,pView);
		if( pVal == NULL )
			return NULL;
		m_pLable->Add(pVal);
		return pVal;
	}
	bool	TabViewV::Switch( Page *page,bool bFocus)
	{
		if( m_pTable && page )
		{
			if( false == page->IsVisible() )
			{
				page->SetVisible(true);
			}
			page->GetOptionUI()->Selected(true);
			//m_pTable->SelectItem( page->GetView(),bFocus );
			m_pTable->SelectItem( page->GetView());
			Invalidate();
			return true;
		}
		return false;
	}
	bool	TabViewV::Switch(LPCTSTR pstrName,bool bFocus)
	{
		TabViewV::Page *page = base::SafeConvert<TabViewV::Page>(m_pLable,pstrName,_T("TabViewV::Page"));
		if( page && m_pTable )
		{
			if( false == page->IsVisible() )
			{
				page->SetVisible(true);
			}
			page->GetOptionUI()->Selected(true);
			//m_pTable->SelectItem( page->GetView(),bFocus );
			m_pTable->SelectItem( page->GetView() );
			Invalidate();
			return true;
		}
		else if( m_pTable )
		{
			//m_pTable->SelectItem(m_pTable->FindSubControl(pstrName),bFocus);
			m_pTable->SelectItem(m_pTable->FindSubControl(pstrName));
			Invalidate();
		}
		return false;
	}
	int		TabViewV::GetPageCount()
	{
		if( m_pLable == NULL )
			return NULL;
		return m_pLable->GetCount();
	}
	TabViewV::Page*	TabViewV::GetPage(int nI)
	{
		if( m_pTable == NULL )
			return NULL;
		return base::SafeConvert<TabViewV::Page>(m_pLable->GetItemAt(nI),_T("TabViewV::Page"));
	}
	TabViewV::Page*	TabViewV::FindPage(LPCTSTR pstrName)
	{
		return base::SafeConvert<TabViewV::Page>(m_pLable,pstrName,_T("TabViewV::Page"));
	}
	TabViewV::Page*	TabViewV::GetCurPage()
	{
		if( m_pTable==NULL || m_pLable==NULL )
			return NULL;
		return base::SafeConvert<TabViewV::Page>(m_pLable->GetItemAt(m_pTable->GetCurSel()),_T("TabViewV::Page"));
	}
	TabViewV::Page*	TabViewV::RemoveUI( Page *page )
	{
		TabViewV::Page* pRet = 0;
		DuiLib::CHorizontalLayoutUI *pCustomLayout = static_cast<DuiLib::CHorizontalLayoutUI*>(m_pLable);
		if( m_pTable && page && m_pLable )
		{
			DuiLib::CControlUI* pCtrl = GetLayoutNeighbor(pCustomLayout,page);
			if( pCtrl )
			{
				if( 0 == _tcscmp(_T("TabViewV::Page"),pCtrl->GetClass()) )
					pRet = static_cast<TabViewV::Page*>(pCtrl);
			}
			m_pTable->Remove( page->GetView() );
			m_pLable->Remove( page );
		}
		return pRet;
	}
	void TabViewV::ResetPageView(Page *page,DuiLib::CControlUI *pView)
	{
		ClearPageView(page->GetView());
		page->SetView(NULL);
		page->SetView(pView);
		if( -1 == m_pTable->GetItemIndex(pView) )
		{
			AddPageView(pView);
		}
		Invalidate();
	}
	void TabViewV::PaintBorder(HDC hDC)
	{
		//
		int	  nTop=0,nLeft=0,nBottom=0,nRight=0;
		DWORD dwColor = IsFocused()?m_dwFocusBorderColor:m_dwBorderColor;
		nTop  = GetBorderSize();
		nLeft = GetBorderSize();
		nRight  = GetBorderSize();
		nBottom = GetBorderSize();
		if( nTop==0 )
			nTop   = GetTopBorderSize();

		if( nLeft==0 )
			nLeft  = GetLeftBorderSize();

		if( nRight==0 )
			nRight = GetRightBorderSize();

		if( nBottom==0 )
			nBottom= GetBottomBorderSize();

		Page *page = GetCurPage();
		if( page && m_pTable && dwColor)
		{
			RECT rcTab = m_pTable->GetPos();
			if( nLeft )
			{
				//Draw left
				rcTab = m_pTable->GetPos();
				rcTab.right = rcTab.left;
				DuiLib::CRenderEngine::DrawLine(hDC,rcTab,nLeft,GetAdjustColor(dwColor),m_nBorderStyle);
			}
			if( nRight )
			{
				//Draw right
				rcTab = m_pTable->GetPos();
				rcTab.right = m_pTable->GetPos().left;
				DuiLib::CRenderEngine::DrawLine(hDC,rcTab,nRight,GetAdjustColor(dwColor),m_nBorderStyle);
			}
			if( nBottom )
			{
				//DrawBottom
				rcTab = m_pTable->GetPos();
				rcTab.top = m_pTable->GetPos().bottom;
				DuiLib::CRenderEngine::DrawLine(hDC,rcTab,nBottom,GetAdjustColor(dwColor),m_nBorderStyle);
			}
			if( nTop )
			{
				//绘制突起部分
				//DrawLeft
				rcTab = m_pTable->GetPos();
				rcTab.right = page->GetPos().left;
				rcTab.bottom= m_pTable->GetPos().top;
				DuiLib::CRenderEngine::DrawLine(hDC,rcTab,nTop,GetAdjustColor(dwColor),m_nBorderStyle);

				rcTab = page->GetPos();
				rcTab.right = page->GetPos().left;
				rcTab.bottom = m_pTable->GetPos().top;
				DuiLib::CRenderEngine::DrawLine(hDC,rcTab,nTop,GetAdjustColor(dwColor),m_nBorderStyle);

				rcTab = page->GetPos();
				rcTab.bottom= page->GetPos().top;
				DuiLib::CRenderEngine::DrawLine(hDC,rcTab,nTop,GetAdjustColor(dwColor),m_nBorderStyle);

				rcTab = page->GetPos();
				rcTab.left  = page->GetPos().right;
				rcTab.bottom= m_pTable->GetPos().top;
				DuiLib::CRenderEngine::DrawLine(hDC,rcTab,nTop,GetAdjustColor(dwColor),m_nBorderStyle);

				//DrawRight
				rcTab = m_pTable->GetPos();
				rcTab.left  = page->GetPos().right;
				rcTab.bottom= m_pTable->GetPos().top;
				DuiLib::CRenderEngine::DrawLine(hDC,rcTab,nTop,GetAdjustColor(dwColor),m_nBorderStyle);

			}
		}
		else
		{
			__super::PaintBorder(hDC);
		}
	}
	/*------------------------------------------------------------------------*/
	TabViewV::Page::Page(TabViewV *pParent,DuiLib::CControlUI *pView)
		:m_pParent(pParent),m_pView(pView),m_pOptCtrl(NULL)
	{
		memset(&m_szPos,0,sizeof(SIZE));
		m_pOptCtrl = new DuiLib::COptionUI;
		Add(m_pOptCtrl);
		m_pOptCtrl->OnNotify += MakeDelegate(this,&TabViewV::Page::OnChangeView);
	}
	TabViewV::Page::Page()
		:m_pParent(0),m_pView(0),m_pOptCtrl(NULL)
	{
		memset(&m_szPos,0,sizeof(SIZE));
		m_pOptCtrl = new DuiLib::COptionUI;
		Add(m_pOptCtrl);
		m_pOptCtrl->OnNotify += MakeDelegate(this,&TabViewV::Page::OnChangeView);
	}
	TabViewV::Page::~Page()
	{

	}
	bool TabViewV::Page::OnChangeView(void *param)
	{
		DuiLib::TNotifyUI *pE = static_cast<DuiLib::TNotifyUI*>(param);
		if( pE == NULL )
			return true;
		if( pE->sType == DUI_MSGTYPE_CLICK && m_pParent )
		{
			m_pParent->Switch(this);
		}
		return true;
	}
	bool TabViewV::Page::IsSelected() const
	{
		if( m_pOptCtrl )
			return m_pOptCtrl->IsSelected();
		return false;
	}
	void TabViewV::Page::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( 0 == _tcscmp(_T("viewclass"),pstrName)  )
			m_strViewClass = pstrValue;
		if( 0 == _tcscmp(_T("optionattr"),pstrName) && m_pOptCtrl )
			m_pOptCtrl->ApplyAttributeList(pstrValue);
		return __super::SetAttribute(pstrName,pstrValue);
	}
	void TabViewV::Page::SetView(DuiLib::CControlUI* pView)
	{
		if( m_pView && m_pParent )
		{
			m_pParent->ClearPageView(m_pView);
		}
		m_pView = pView;
		m_pParent->AddPageView(m_pView);
		Invalidate();
	}
	DuiLib::CControlUI*	TabViewV::Page::GetView() const
	{
		return m_pView;
	}
	DuiLib::COptionUI*	TabViewV::Page::GetOptionUI() const
	{
		return m_pOptCtrl;
	}
	void TabViewV::Page::SetScrollPosAt( SIZE szPos )
	{
		m_szPos = szPos;
	}
	SIZE TabViewV::Page::GetScrollPosAt()
	{
		return m_szPos;
	}

	void TabViewV::Page::SetTabView( TabViewV *pV )
	{
		if( m_pParent == NULL )
			m_pParent = pV;
		if( m_pParent != pV )
		{
			m_pParent->RemoveUI(this);
			m_pParent = pV;
		}
	}
	TabViewV*	TabViewV::Page::GetTabView() const
	{
		return m_pParent;
	}
	void TabViewV::Page::SetVisible(bool bVisible)
	{
		__super::SetVisible(bVisible);
		if( m_pView )
		{
			m_pView->SetVisible(bVisible);
		}
		if( m_pParent )
		{
			m_pParent->Invalidate();
		}
	}

}