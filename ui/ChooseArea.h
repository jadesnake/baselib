#pragma once

namespace CustomUI {

class ChooseArea : public DuiLib::WindowImplBase 
{
public:
	typedef struct _tgArea
	{
		CAtlString id;
		CAtlString name;
	}AREA;
	class Logic
	{
	public:
		virtual ~Logic(){	}
		virtual void OnRelease(CustomUI::ChooseArea *area) = 0;
		virtual void OnInitByChooseArea(CustomUI::ChooseArea *area)=0;
		virtual void OnGetSheng()=0;
		virtual void OnGetShi(const Areas::Area &area)=0;
		virtual void OnGetQuXian(const Areas::Area &area)=0;
		virtual Areas::Area FindSheng(const CAtlString &id)=0;
		virtual Areas::Area FindShi(const CAtlString &id)=0;
		virtual Areas::Area FindQuxian(const CAtlString &id)=0;
	};
	ChooseArea(HWND parent);
	virtual ~ChooseArea();
	void AttchLogic(Logic *logic);
	void Show(const POINT &pt);
	void ShowError(const CAtlString& tip);
	void HideError();

	void NotifyShengComplete(const Areas &vals);
	void NotifyShiComplete(const Areas &vals);
	void NotifyQuXianComplete(const Areas &vals);
	
	DuiLib::CEventSource fireComplete;		//通知选择完毕
protected:
	void InitWindow();
	DuiLib::CDuiString GetSkinFolder();
	DuiLib::CDuiString GetSkinFile();
	LPCTSTR GetWindowClassName(void)const;
	UILIB_RESOURCETYPE GetResourceType() const;
	LPCTSTR		GetResourceID() const;
	void OnClick(DuiLib::TNotifyUI& msg);
	void OnFinalMessage( HWND hWnd );
	void Notify(DuiLib::TNotifyUI& msg);
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
protected:
	void SetVisibleSheng(bool v);
	void SetVisibleShi(bool v);
	void SetVisibleQuxian(bool v);
protected:
	AREA	curSheng;	//选择的省
	AREA	curShi;		//选择的市
	AREA	curQuXian;	//选择的区县
	Logic	*logic;
	HWND	m_hParent;
	DuiLib::CVerticalLayoutUI *layoutSheng;
	DuiLib::CVerticalLayoutUI *layoutShi;
	DuiLib::CVerticalLayoutUI *layoutQuxian;
	DuiLib::CContainerUI *root;
};

}