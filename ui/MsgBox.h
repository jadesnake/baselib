#pragma once

#include "ui/DuiDefaultWinRes.h"
#include "baseX.h"
class MsgBox : public DuiDefaultWinRes
{
public:
	class User
	{
	public:
		virtual ~User(){	}
		virtual LPCTSTR GetSkinFile()=0;
		virtual void DoInit(MsgBox *) = 0;
		virtual void DoClick(DuiLib::TNotifyUI& msg) = 0;
		virtual void DoNotify(DuiLib::TNotifyUI& msg)= 0;
		virtual void OnRelease() = 0;
		virtual void OnPaint(){		}
	};
	MsgBox();
	~MsgBox();
	void SetInitSize(SIZE szWin);
	void SetFixedSize(long w,long h);
	UINT ShowModal(HWND parent,User *user,bool bV=true);
	DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);

	template<class R>
	R* GetUser()
	{
		return static_cast<R*>(user);
	}
	template<class S>
	S* GetUI(LPCTSTR n,LPCTSTR c)	{
		return base::SafeConvert<S>(&m_PaintManager,n,c);
	}
	void EnableEscClose(bool b);
	void MoveX(long x);
protected:
	DuiLib::CDuiString GetSkinFile();
	LPCTSTR GetWindowClassName(void)const;
	void OnClick(DuiLib::TNotifyUI& msg);
	void Notify(DuiLib::TNotifyUI& msg);
	void OnFinalMessage( HWND hWnd );
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	User *user;
	bool bEscClose;
	long moveX;
	SIZE szWinInit;
};