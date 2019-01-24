#pragma once

//广告控件，用于显示广告信息并支持动态切换以及点击事件
namespace CustomUI
{
class Advert : public DuiLib::CVerticalLayoutUI
{
public:
	Advert(void);
	~Advert(void);
	void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void SetNormalImage(LPCTSTR pName);
	void SetHotImage(LPCTSTR pName);
	void SetSelectedImage(LPCTSTR pName);
	void SetInterruptTimer(LONG ltm);
	void StopInterruptTimer(void);
	void AppendAdvertImage(LPCTSTR pFile);
	void SetGroup(LPCTSTR pGroup);
	bool SelectItem(int nIndex);
	LPCTSTR GetClass() const;
	LPVOID	GetInterface(LPCTSTR pstrName);
	int	 ReloadRes(LPCTSTR pDir);
	void SetPos(RECT rc, bool bNeedInvalidate = true);
	void NextPage();
	void FrontPage();
	void SetManager(DuiLib::CPaintManagerUI* pManager,DuiLib::CControlUI* pParent, bool bInit = true);
	DuiLib::CControlUI* FindControl(DuiLib::FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);
	int  GetPageNo();
	DuiLib::CEventSource OnChangePage;
protected:
	void DoPaint(HDC hDC, const RECT& rcPaint);

	void DoEvent(DuiLib::TEventUI& event);
	bool OnOptionEvent( void *param );
	bool OnShowAreaEvent(void *param);
	void UpdateOptions(const SIZE &sz);
private:
	bool						m_bStop;
	long						m_timer;
	bool						m_hasResizeShow;//需要重新计算显示区高度
	bool						m_hasResizeBar;
	SIZE						m_szOption;		//按钮宽度和高度
	DuiLib::CDuiString			m_pichot;
	DuiLib::CDuiString			m_picnormal;
	DuiLib::CDuiString			m_picselected;
	DuiLib::CDuiString			m_picdir;		//广告图片目录

	DuiLib::CDuiString			m_optGroup;
	DuiLib::CTabLayoutUI		*m_lstAdvert;	//广告列表
	DuiLib::CHorizontalLayoutUI *m_lstBtns;		//按钮容器

	bool m_bHaltOnMouse;
};
}