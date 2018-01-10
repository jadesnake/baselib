#include "StdAfx.h"
#include "AdvertUI.h"
#include "../MacroX.h"
#define TM_ADVERT	4672

namespace CustomUI
{
Advert::Advert(void)
		:m_optGroup(_T("AdvertGroup")),m_timer(3000),m_bHaltOnMouse(true),
		m_hasResizeBar(false),m_hasResizeShow(false),m_bStop(false)
{
	m_lstAdvert = new DuiLib::CTabLayoutUI;
	m_lstBtns	= new DuiLib::CHorizontalLayoutUI;
	
	RECT rcIn = m_lstBtns->GetInset();
	rcIn.top  = 5;
	m_lstBtns->SetInset(rcIn);
	m_lstBtns->SetChildPadding(8);
	m_lstBtns->SetFixedHeight(10);
	m_lstBtns->SetVisible(false);
	this->Add(m_lstAdvert);
	//this->Add(m_lstBtns);
}
Advert::~Advert(void)
{
	if( m_lstBtns )
	{
		delete m_lstBtns;
		m_lstBtns = NULL;
	}
}
LPCTSTR Advert::GetClass() const
{
	return _T("AdvertUI");
}
LPVOID Advert::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName,GetClass()) == 0 )
		return static_cast<Advert*>(this);
	return __super::GetInterface(pstrName);
}
void Advert::SetNormalImage(LPCTSTR pName)
{
	if( pName == NULL )	return ;
	for(int nI=0;nI < m_lstAdvert->GetCount();nI++)
	{
		DuiLib::CControlUI *pCtrl = m_lstAdvert->GetItemAt(nI);
		if( pCtrl == NULL )	break;
		DuiLib::CControlUI *pTag = (DuiLib::CControlUI*)(void*)pCtrl->GetTag();
		if( pTag )
		{
			DuiLib::COptionUI* btn = static_cast<DuiLib::COptionUI*>(
									pTag->GetInterface(DUI_CTR_OPTION));
			if( btn == NULL )	break;
			if( _tcscmp(btn->GetNormalImage(),pName) )
				btn->SetNormalImage(pName);
		}
	}
}
void Advert::SetHotImage(LPCTSTR pName)
{
	if( pName == NULL )	return ;
	for(int nI=0;nI < m_lstAdvert->GetCount();nI++)
	{
		DuiLib::CControlUI *pCtrl = m_lstAdvert->GetItemAt(nI);
		if( pCtrl == NULL )	break;
		DuiLib::CControlUI *pTag = (DuiLib::CControlUI*)(void*)pCtrl->GetTag();
		if( pTag )
		{
			DuiLib::COptionUI* btn = static_cast<DuiLib::COptionUI*>(pTag->GetInterface(DUI_CTR_OPTION));
			if( btn == NULL )	break;
			if( _tcscmp(btn->GetHotImage(),pName) )
				btn->SetHotImage(pName);
		}
	}
}
void Advert::SetSelectedImage(LPCTSTR pName)
{
	if( pName == NULL )	return ;
	for(int nI=0;nI < m_lstAdvert->GetCount();nI++)
	{
		DuiLib::CControlUI *pCtrl = m_lstAdvert->GetItemAt(nI);
		if( pCtrl == NULL )	break;
		DuiLib::CControlUI *pTag = (DuiLib::CControlUI*)(void*)pCtrl->GetTag();
		if( pTag )
		{
			DuiLib::COptionUI* btn = static_cast<DuiLib::COptionUI*>(pTag->GetInterface(DUI_CTR_OPTION));
			if( btn == NULL )	break;
			if( _tcscmp(btn->GetSelectedImage(),pName) )
				btn->SetSelectedImage(pName);
		}
	}
}
void Advert::SetInterruptTimer(LONG ltm)
{
	DuiLib::CPaintManagerUI *pm = this->GetManager();
	if( pm == NULL )	return ;
	m_timer = ltm;
	m_bStop = false;
	pm->SetTimer(this,TM_ADVERT,ltm);
}
void Advert::StopInterruptTimer(void)
{
	DuiLib::CPaintManagerUI *pm = this->GetManager();
	if( pm == NULL )	return ;
	m_bStop = true;
}
void Advert::AppendAdvertImage(LPCTSTR pFile)
{
	DuiLib::CLabelUI  *pLabel = NULL;
	DuiLib::COptionUI *opt	  = NULL;
	NOTHROW_NEW(DuiLib::CLabelUI,pLabel);
	NOTHROW_NEW(DuiLib::COptionUI,opt);
	pLabel->SetTag( (UINT_PTR)(void*)opt );
	pLabel->SetBkImage(pFile);
	pLabel->OnEvent += DuiLib::MakeDelegate(this,&Advert::OnShowAreaEvent);

	opt->SetNormalImage(m_picnormal);
	opt->SetHotImage(m_pichot);
	opt->SetSelectedImage(m_picselected);
	opt->SetFixedWidth(m_szOption.cx);
	opt->SetFixedHeight(m_szOption.cy);
	opt->OnEvent += DuiLib::MakeDelegate(this,&Advert::OnOptionEvent);
	opt->SetTag( (UINT_PTR)(void*)pLabel );
	opt->SetGroup(m_optGroup);

	m_lstAdvert->Add(pLabel);
	m_lstBtns->Add(opt);
	if( 2 <= m_lstBtns->GetCount() && false==m_lstBtns->IsVisible() )
		m_lstBtns->SetVisible(true);
	m_hasResizeBar = true;
	m_hasResizeShow= true;
}
void Advert::SetPos(RECT rc)
{
	__super::SetPos(rc);
	if( m_lstBtns == NULL)
		return ;
	RECT rcOpts;
	memcpy(&rcOpts,&rc,sizeof(RECT));
	rcOpts.left   += m_lstBtns->GetPadding().left;
	rcOpts.right  -= m_lstBtns->GetPadding().right;
	rcOpts.bottom -= m_lstBtns->GetPadding().bottom;
	rcOpts.top	  = rcOpts.bottom - m_lstBtns->GetFixedHeight();
	m_lstBtns->SetPos(rcOpts);
	//
	UpdateOptions(m_szOption);
	rcOpts.bottom -= m_lstBtns->GetPadding().bottom;
	rcOpts.top	  = rcOpts.bottom - m_lstBtns->GetFixedHeight();
	m_lstBtns->SetPos(rcOpts);

	if( m_hasResizeShow && m_lstBtns->GetFixedHeight() )
	{
		for(int nI=0;nI < m_lstAdvert->GetCount();nI++)
		{
			DuiLib::CControlUI *pCtrl = m_lstAdvert->GetItemAt(nI);
			if( pCtrl )
			{
				pCtrl->SetFixedHeight(GetHeight());
			}
		}
		m_hasResizeShow = false;
	}
}
bool Advert::OnOptionEvent( void *param )
{
	DuiLib::TEventUI *pEvent = (DuiLib::TEventUI*)param;
	if( pEvent->Type == DuiLib::UIEVENT_BUTTONUP )
	{
		DuiLib::COptionUI  *pOpt  = static_cast<DuiLib::COptionUI*>(pEvent->pSender);
		DuiLib::CControlUI *pCtrl = static_cast<DuiLib::CControlUI*>((void*)(pOpt->GetTag()));
		int nIndex = m_lstAdvert->GetItemIndex(pCtrl);
		SelectItem(nIndex);
		pOpt->Selected(true);
		return false;
	}
	return true;
}
bool Advert::OnShowAreaEvent(void *param)
{
	DuiLib::TEventUI *pEvent = (DuiLib::TEventUI*)param;
	if( pEvent->Type == DuiLib::UIEVENT_MOUSEENTER ||
		pEvent->Type == DuiLib::UIEVENT_MOUSEHOVER || 
		pEvent->Type == DuiLib::UIEVENT_MOUSEMOVE )
	{
		if (m_bHaltOnMouse)
		{
			StopInterruptTimer();
		}
		return false;
	}
	if( pEvent->Type == DuiLib::UIEVENT_MOUSELEAVE )
	{
		if (m_bHaltOnMouse)
		{
			SetInterruptTimer(m_timer);
		}
		return false;
	}
	return true;
}
void Advert::SetGroup(LPCTSTR pGroup)
{
	if( pGroup == NULL )	return ;
	if( 0 == m_optGroup.CompareNoCase(pGroup) )	return ;
	m_optGroup = pGroup;
	for(int nI=0;nI < m_lstAdvert->GetCount();nI++)
	{
		DuiLib::CControlUI *pCtrl = m_lstAdvert->GetItemAt(nI);
		if( pCtrl == NULL )	break;
		DuiLib::CControlUI *pTag = (DuiLib::CControlUI*)(void*)pCtrl->GetTag();
		if( pTag )
		{
			DuiLib::COptionUI* btn = static_cast<DuiLib::COptionUI*>(pTag->GetInterface(DUI_CTR_OPTION));
			if( btn == NULL )	break;
			btn->SetGroup(m_optGroup);
		}
	}
}
void Advert::UpdateOptions(const SIZE &sz)
{
	int unTotalW = 0;				//
	for(int nI=0;nI < m_lstBtns->GetCount();nI++)
	{
		DuiLib::CControlUI *pCtrl = m_lstBtns->GetItemAt(nI);
		RECT pad = pCtrl->GetPadding();
		unTotalW += pad.left + pad.right + pCtrl->GetFixedWidth();
		if( (nI+1) < m_lstBtns->GetCount() )		
			unTotalW += m_lstBtns->GetChildPadding();
	}
	if( unTotalW && (unTotalW < m_lstBtns->GetWidth()) && m_hasResizeBar )
	{
		int nMovW = (m_lstBtns->GetWidth() - unTotalW)/2;		
		RECT rc = m_lstBtns->GetInset();
		rc.left = nMovW;
		rc.right= nMovW;
		m_lstBtns->SetInset(rc);
		m_lstBtns->SetFixedHeight(sz.cy+rc.bottom+rc.top);
		m_hasResizeBar = false;
	}
	else if( unTotalW && (unTotalW > m_lstBtns->GetWidth()) && m_hasResizeBar )
	{
		//缩小每个控件之间的间距
		RECT rc = m_lstBtns->GetInset();
		int nOver = unTotalW - m_lstBtns->GetWidth();						//超出的空间
		int nCut = (m_lstBtns->GetCount()-1)*m_lstBtns->GetChildPadding();	//可以节省的空间
		if( nOver < nCut )
		{
			m_lstBtns->SetChildPadding( (nCut-nOver)/(m_lstBtns->GetCount()-1) );
			unTotalW -= m_lstBtns->GetChildPadding()*(m_lstBtns->GetCount()-1);
			rc.left = (m_lstBtns->GetWidth() - unTotalW)/2;
			rc.right= (m_lstBtns->GetWidth() - unTotalW)/2;
			m_lstBtns->SetInset(rc);
			m_lstBtns->SetFixedHeight(sz.cy+rc.bottom+rc.top);
		}
		m_hasResizeBar = false;
	}
	return ;
}
int	 Advert::ReloadRes(LPCTSTR pDir)
{
	TCHAR	sSearch[MAX_PATH+1];
	int	nRet = 0;
	WIN32_FIND_DATA	fdata;
	HANDLE	hSearch = NULL;
	if( NULL == m_lstAdvert )	
		return 0;
	if( NULL == m_lstBtns )		
		return 0;
	memset(sSearch,0,sizeof(sSearch));
	memset(&fdata,0,sizeof(WIN32_FIND_DATA));
	m_lstBtns->RemoveAll();
	m_lstAdvert->RemoveAll();
	::PathCombine(sSearch,pDir,_T("*.*"));
	hSearch = FindFirstFile(sSearch,&fdata);
	if( hSearch == INVALID_HANDLE_VALUE )	
		return 0;
	CAtlString filename;
	do 
	{
		//过滤掉目录和0字节文件
		if( (fdata.dwFileAttributes|FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
			continue;
		if( (fdata.dwFileAttributes|FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN )
			continue;
		if( fdata.nFileSizeLow == 0 )
			continue;
		filename=fdata.cFileName;
		if(filename.Find(_T(".db"))!=-1)
			continue;  //////过滤掉目录下的db文件
		memset(sSearch,0,sizeof(sSearch));
		::PathCombine(sSearch,pDir,fdata.cFileName);
		AppendAdvertImage(sSearch);
		nRet++;
	}while( FindNextFile(hSearch,&fdata) );	
	FindClose(hSearch);
	return nRet;
}
void Advert::SetManager(DuiLib::CPaintManagerUI* pManager,DuiLib::CControlUI* pParent, bool bInit)
{
	__super::SetManager(pManager,pParent,bInit);
	if( m_lstBtns )
	{
		m_lstBtns->SetManager(pManager,this,bInit);
	}
}
DuiLib::CControlUI* Advert::FindControl(DuiLib::FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
	DuiLib::CControlUI*	pRet = NULL;
	if( m_lstBtns )
	{
		pRet = m_lstBtns->FindControl(Proc,pData,uFlags);
	}
	if( pRet == NULL )
	{
		pRet = __super::FindControl(Proc,pData,uFlags);
	}
	return pRet;	
}
void Advert::DoPaint(HDC hDC, const RECT& rcPaint)
{
	__super::DoPaint(hDC,rcPaint);
	if( m_lstBtns )
	{
		m_lstBtns->DoPaint(hDC,m_lstBtns->GetPos());
	}
}
void Advert::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcscmp(pstrName,_T("picdir")) == 0 )
	{
		TCHAR	chPath[MAX_PATH+1];
		memset(chPath,0,sizeof(chPath));
		DuiLib::CPaintManagerUI *pm = GetManager();
		::PathCombine(chPath,pm->GetResourcePath().GetData(),pstrValue);
		ReloadRes(chPath);
		return ;
	}
	if( _tcscmp(pstrName,_T("opts_padding"))==0 && m_lstBtns)
	{
		m_lstBtns->SetAttribute(_T("padding"),pstrValue);
		return ;
	}
	if( _tcscmp(pstrName, _T("group")) == 0 ) 
	{
		SetGroup(pstrValue);
		return ;
	}
	if( 0 == _tcscmp(pstrName,_T("InterruptTimer")) )
	{
		SetInterruptTimer(_ttol(pstrValue));
		return ;
	}
	if( _tcscmp(pstrName, _T("opt_hotimage")) == 0 )
	{
		m_pichot = pstrValue;
		this->SetHotImage(m_pichot);
		return ;
	}
	if( _tcscmp(pstrName, _T("opt_selectedimage")) == 0 )
	{
		m_picselected = pstrValue;
		this->SetSelectedImage(m_picselected);
		return ;
	}
	if( _tcscmp(pstrName, _T("opt_normalimage")) == 0 )
	{
		m_picnormal = pstrValue;
		this->SetNormalImage(m_picnormal);
		return ;
	}
	if( _tcscmp(pstrName,_T("opt_groupname")) == 0 )
	{
		m_optGroup = pstrValue;
		this->SetGroup(m_optGroup);
	}
	if( _tcscmp(pstrName, _T("selectedid")) == 0 )
	{
		SelectItem(_ttoi(pstrValue));
		return ;
	}
	if( _tcscmp(pstrName, _T("opt_size")) == 0 )
	{
		SIZE chw = { 0 };
		LPTSTR pstr = NULL;
		chw.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		chw.cy = _tcstol(pstr + 1, &pstr, 10);   ASSERT(pstr);
		memcpy(&m_szOption,&chw,sizeof(SIZE));
		UpdateOptions(chw);
		return ;
	}
	if (_tcscmp(pstrName,_T("HaltOnMouse")) == 0 )
	{
		m_bHaltOnMouse = (_tcscmp(pstrValue, _T("true")) == 0);
	}
	__super::SetAttribute(pstrName,pstrValue);
}
bool Advert::SelectItem(int nIndex)
{
	DuiLib::CControlUI *pCtrl = m_lstAdvert->GetItemAt(nIndex);
	if( pCtrl == NULL )	return false;
	DuiLib::CControlUI *pTag = (DuiLib::CControlUI*)(void*)pCtrl->GetTag();
	if( pTag )
	{
		if( !m_lstAdvert->SelectItem(pCtrl) )
			return false;
		OnChangePage(this);
		DuiLib::COptionUI* btn = static_cast<DuiLib::COptionUI*>(pTag->GetInterface(DUI_CTR_OPTION));
		if( btn )
			btn->Selected(true);
		return true;
	}
	return false;
}
void Advert::DoEvent(DuiLib::TEventUI& event)
{
	if( (event.pSender == this) && (event.Type ==DuiLib::UIEVENT_TIMER) && (event.wParam==TM_ADVERT) )
	{
		int nI = 0;
		if( !m_bStop && IsVisible() )
		{
			nI = m_lstAdvert->GetCurSel();
			if( nI == -1 )
				nI = 0;
			else if( (nI+1) == m_lstAdvert->GetCount() )
				nI = 0;
			else
				nI++;
			SelectItem(nI);
		}
		return ;
	}
	__super::DoEvent(event);
}
void Advert::NextPage()
{
	int nI = m_lstAdvert->GetCurSel();
	if( nI == -1 )
		nI = 0;
	else if( (nI+1) == m_lstAdvert->GetCount() )
		nI = 0;
	else
		nI++;
	StopInterruptTimer();
	SelectItem(nI);
	SetInterruptTimer(m_timer);
}
void Advert::FrontPage()
{
	int nI = m_lstAdvert->GetCurSel();
	if( nI == -1 )
		nI = m_lstAdvert->GetCount()-1;
	else if( (nI-1) == -1 )
		nI = 0;
	else
		nI-=1;
	StopInterruptTimer();
	SelectItem(nI);
	SetInterruptTimer(m_timer);
}
int Advert::GetPageNo()
{
	return m_lstAdvert->GetCurSel();
}

}