#include "stdafx.h"
#include "UIlib.h"
#include "MsgBox.h"
#include "uiControl/CustomControl.h"

MsgBox::MsgBox()
	:user(NULL),bEscClose(true),moveX(0)
{
	memset(&szWinInit,0,sizeof(SIZE));
}
MsgBox::~MsgBox()
{
	
}

UINT MsgBox::ShowModal(HWND parent,User *user,bool bV)
{
    this->user = user;
	DWORD style = UI_WNDSTYLE_DIALOG;
	if(bV==false)
		style &= ~WS_VISIBLE;
	Create(parent,_T("ÌבÊ¾"),UI_WNDSTYLE_DIALOG,0);
	user->DoInit(this);
	if(szWinInit.cx && szWinInit.cy)
	{
		m_PaintManager.SetInitSize(szWinInit.cx,szWinInit.cy);
		::SetWindowPos(m_hWnd,NULL,0,0,szWinInit.cx,szWinInit.cy,SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
	}
	CenterWindow();
	if(moveX)
	{
		RECT rcWin;
		GetWindowRect(m_hWnd,&rcWin);
		::SetWindowPos(m_hWnd,NULL,rcWin.left + moveX,rcWin.top,-1,-1,SWP_NOZORDER|SWP_NOSIZE);
	}
	return WindowImplBase::ShowModal();
}
DuiLib::CControlUI* MsgBox::CreateControl(LPCTSTR pstrClass)
{
	return CustomUI::CreateCustomUI(pstrClass,static_cast<DuiDefaultWinRes*>(this));
}
DuiLib::CDuiString MsgBox::GetSkinFile()
{
	return DuiLib::CDuiString(user->GetSkinFile());
}
LPCTSTR MsgBox::GetWindowClassName(void) const
{
	return _T("MessageBoxTip");
}
void MsgBox::OnClick(DuiLib::TNotifyUI& msg)
{
	if(user)
		user->DoClick(msg);
	if(msg.pSender->GetName()==_T("close"))
	{
		Close(IDCANCEL);
		return ;
	}
	if(msg.pSender->GetName()==_T("ok"))
	{
		Close(IDOK);
		return ;
	}
}
void MsgBox::Notify(DuiLib::TNotifyUI& msg)
{
	__super::Notify(msg);
	if(msg.sType==DUI_MSGTYPE_WINDOWINIT&&user)
	{
		return ;
	}
	if(user)
		user->DoNotify(msg);
}
void MsgBox::OnFinalMessage( HWND hWnd )
{
	if(user)
	{
		user->OnRelease();
		user = NULL;
	}
	__super::OnFinalMessage(hWnd);
}
LRESULT MsgBox::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg==WM_KEYDOWN && wParam==VK_ESCAPE && bEscClose)
	{
		Close(IDCANCEL);
		return 0;
	}
	if(uMsg==WM_PAINT && user)
	{
		LRESULT ret = __super::HandleMessage(uMsg,wParam,lParam);
		user->OnPaint();
		return ret;
	}
	return __super::HandleMessage(uMsg,wParam,lParam);
}
void MsgBox::EnableEscClose(bool b)
{
	bEscClose = b;
}
void MsgBox::SetInitSize(SIZE szWin)
{
	szWinInit = szWin;
}
void MsgBox::SetFixedSize(long w,long h)
{
	m_PaintManager.SetInitSize(w,h);
	m_PaintManager.SetMaxInfo(w,h);
	m_PaintManager.SetMinInfo(w,h);
	::SetWindowPos(m_hWnd,NULL,0,0,w,h,SWP_NOMOVE|SWP_NOZORDER|SWP_HIDEWINDOW);
	CenterWindow();
	if(moveX)
	{
		RECT rcWin;
		GetWindowRect(m_hWnd,&rcWin);
		::SetWindowPos(m_hWnd,NULL,rcWin.left + moveX,rcWin.top,-1,-1,SWP_NOZORDER|SWP_NOSIZE);
	}
	ShowWindow();
}
void MsgBox::MoveX(long x)
{
	moveX = x;
}
