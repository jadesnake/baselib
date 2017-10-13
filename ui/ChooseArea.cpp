#include "stdafx.h"
#include "ChooseArea.h"

namespace CustomUI
{
	#define SHENG_GROUP _T("Sheng")
	#define SHI_GROUP   _T("Shi")
	#define QUXIAN_GROUP _T("QuXian")
	/***************************************ChooseHeaderLayout begin****************************************************************/
	class ChooseHeaderLayout : public DuiLib::CHorizontalLayoutUI
	{
	public:
		DUI_DECLARE(CustomUI::ChooseHeaderLayout,DuiLib::CHorizontalLayoutUI)
	protected:
		void DoPaint(HDC hDC, const RECT& rcPaint)
		{
			startDrawBorder = false;
			__super::DoPaint(hDC,rcPaint);
			startDrawBorder = true;
			PaintBorder(hDC);
		}
		void PaintBorder(HDC hDC)
		{
			if(startDrawBorder==false)
				return ;
			LONG nTmp = m_rcBorderSize.bottom;
			m_rcBorderSize.bottom = 0;
			__super::PaintBorder(hDC);
			m_rcBorderSize.bottom = nTmp;
			RECT rcBorder;
			if(m_dwBorderColor != 0 || m_dwFocusBorderColor != 0 && m_rcBorderSize.bottom > 0)
			{
				DuiLib::COptionUI *choose = NULL;
				for(int n=0;n<GetCount();n++)
				{
					DuiLib::COptionUI *ctrl = base::SafeConvert<DuiLib::COptionUI>(GetItemAt(n),DUI_CTR_OPTION);
					if(ctrl && ctrl->IsSelected())
					{
						choose = ctrl;
						break;
					}
				}
				rcBorder = m_rcItem;
				rcBorder.top	= m_rcItem.bottom;
				rcBorder.top = rcBorder.top-1;
				rcBorder.bottom = rcBorder.bottom-1;
				if(choose==NULL)
				{
					if(IsFocused() && m_dwFocusBorderColor != 0)
					{
						CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.bottom,GetAdjustColor(m_dwFocusBorderColor),m_nBorderStyle);
					}
					else
					{
						CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.bottom,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
				}
				else
				{
					//left 
					rcBorder.left = m_rcItem.left;
					rcBorder.right= choose->GetPos().left;
					if(IsFocused() && m_dwFocusBorderColor != 0)
					{
						CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.bottom,GetAdjustColor(m_dwFocusBorderColor),m_nBorderStyle);
					}
					else
					{
						CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.bottom,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
					//right
					rcBorder.left = choose->GetPos().right;
					rcBorder.right= m_rcItem.right;
					if(IsFocused() && m_dwFocusBorderColor != 0)
					{
						CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.bottom,GetAdjustColor(m_dwFocusBorderColor),m_nBorderStyle);
					}
					else
					{
						CRenderEngine::DrawLine(hDC,rcBorder,m_rcBorderSize.bottom,GetAdjustColor(m_dwBorderColor),m_nBorderStyle);
					}
				}
			}
		}
	private:
		bool startDrawBorder;
	};
	/*********************************************ChooseHeaderLayout end*********************************************************/
	ChooseArea::ChooseArea(HWND parent)
		:m_hParent(parent),logic(NULL),layoutSheng(NULL),root(NULL)
	{
		Create(parent,_T("选择"),WS_POPUP,WS_EX_TOOLWINDOW);
	}
	ChooseArea::~ChooseArea()
	{

	}
	void ChooseArea::AttchLogic(Logic *logic)
	{
		if(this->logic && logic && this->logic!=logic)
		{
			this->logic->OnRelease(this);
		}
		this->logic=logic;
		if(this->logic)
			this->logic->OnInitByChooseArea(this);
	}
	void ChooseArea::Show(const POINT &pt)
	{
		DuiLib::CDuiRect rcWnd;
		GetWindowRect(m_hWnd, &rcWnd);
		int nWidth = rcWnd.GetWidth();
		int nHeight = rcWnd.GetHeight();
		rcWnd.left = pt.x;
		rcWnd.top = pt.y;
		rcWnd.right = rcWnd.left + nWidth;
		rcWnd.bottom = rcWnd.top + nHeight;
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		DuiLib::CDuiRect rcWork = oMonitor.rcMonitor;
		if (rcWnd.bottom > rcWork.bottom)
		{
			if (nHeight >= rcWork.GetHeight()) 
			{
				rcWnd.top = 0;
				rcWnd.bottom = nHeight;
			}
			else 
			{
				rcWnd.bottom = rcWork.bottom;
				rcWnd.top = rcWnd.bottom - nHeight;
			}
		}
		if (rcWnd.right > rcWork.right)
		{
			if (nWidth >= rcWork.GetWidth()) 
			{
				rcWnd.left = 0;
				rcWnd.right = nWidth;
			}
			else 
			{
				rcWnd.right = rcWork.right;
				rcWnd.left = rcWnd.right - nWidth;
			}
		}
		if(m_hParent)
			::SendMessage(m_hParent, WM_NCACTIVATE, TRUE, 0L);
		::SetWindowPos(m_hWnd, NULL, rcWnd.left, rcWnd.top, rcWnd.GetWidth(), rcWnd.GetHeight(),
					   SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE|SWP_SHOWWINDOW);
		::SetForegroundWindow(m_hWnd);
		::SetFocus(m_hWnd);
	}
	void ChooseArea::InitWindow()
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	}
	DuiLib::CDuiString ChooseArea::GetSkinFolder()
	{
		return _T("");
	}
	DuiLib::CDuiString ChooseArea::GetSkinFile()
	{
		return _T("xml/注册/chooseArea.xml");
	}
	LPCTSTR ChooseArea::GetWindowClassName(void) const
	{
		return _T("ChooseArea");
	}
	UILIB_RESOURCETYPE ChooseArea::GetResourceType() const
	{
		return UILIB_ZIPRESOURCE;
	}
	LPCTSTR ChooseArea::GetResourceID() const
	{
		return MAKEINTRESOURCE(IDR_SKINRES);
	}
	DuiLib::COptionUI*	CreateHeaderOption()
	{
		DuiLib::COptionUI *ret = new DuiLib::COptionUI();
		ret->SetGroup(_T("choose"));
		ret->SetFixedWidth(110);
		ret->SetAttribute(_T("textcolor"),_T("#323232"));
		ret->SetAttribute(_T("normalimage"),_T("file='xml/注册/圆角矩形-浅色.png' corner='10,10,10,10'"));
		ret->SetAttribute(_T("selectedimage"),_T("file='xml/注册/圆角矩形.png' corner='10,10,10,10'"));
		return ret;
	}
	void ChooseArea::OnClick(DuiLib::TNotifyUI& msg)
	{
		DuiLib::COptionUI *opt = base::SafeConvert<DuiLib::COptionUI>(msg.pSender,DUI_CTR_OPTION);
		DuiLib::CHorizontalLayoutUI *header = base::SafeConvert<DuiLib::CHorizontalLayoutUI>(&m_PaintManager,_T("header"),DUI_CTR_HORIZONTALLAYOUT);
		if( 0==_tcscmp(_T("choose"),opt->GetGroup()) && header )
		{
			if(0==header->GetItemIndex(opt))
				SetVisibleSheng(true);
			if(1==header->GetItemIndex(opt))
				SetVisibleShi(true);
			if(2==header->GetItemIndex(opt))
				SetVisibleQuxian(true);
			return ;
		}
		if( 0==_tcscmp(SHENG_GROUP,opt->GetGroup()) && header )
		{
			DuiLib::COptionUI *chooseSheng = base::SafeConvert<DuiLib::COptionUI>(header->GetItemAt(0),DUI_CTR_OPTION);
			chooseSheng->SetText(opt->GetText());
			Areas::Area area = logic->FindSheng(opt->GetUserData().GetData());			
			if(!curSheng.id.IsEmpty() && curSheng.id!=area.areaId)
			{
				if( header->GetItemAt(1) )
					header->GetItemAt(1)->SetVisible(false);
				if( header->GetItemAt(2) )
					header->GetItemAt(2)->SetVisible(false);
			}			
			curSheng.id  = area.areaId;
			curSheng.name= area.areaName;
			logic->OnGetShi(area);
			return ;
		}
		if( 0==_tcscmp(SHI_GROUP,opt->GetGroup()) && header )
		{
			DuiLib::COptionUI *chooseShi = base::SafeConvert<DuiLib::COptionUI>(header->GetItemAt(1),DUI_CTR_OPTION);
			if(chooseShi==NULL)
			{
				chooseShi = CreateHeaderOption();
				header->Add(chooseShi);
			}
			chooseShi->SetVisible(true);
			chooseShi->Selected(true);
			chooseShi->SetText(opt->GetText());
			Areas::Area area = logic->FindShi(opt->GetUserData().GetData());
			if(!curShi.id.IsEmpty() && curShi.id!=area.areaId)
			{
				if( header->GetItemAt(3) )
					header->GetItemAt(3)->SetVisible(false);
			}			
			curShi.id = area.areaId;
			curShi.name = area.areaName;
			logic->OnGetQuXian(area);
			return ;
		}
		if( 0==_tcscmp(QUXIAN_GROUP,opt->GetGroup()) && header )
		{
			DuiLib::COptionUI *chooseQu = base::SafeConvert<DuiLib::COptionUI>(header->GetItemAt(2),DUI_CTR_OPTION);
			if(chooseQu==NULL)
			{
				chooseQu = CreateHeaderOption();
				header->Add(chooseQu);
			}
			chooseQu->SetVisible(true);
			chooseQu->Selected(true);
			chooseQu->SetText(opt->GetText());
			Areas::Area area = logic->FindQuxian(opt->GetUserData().GetData());
			curQuXian.id = area.areaId;
			curQuXian.name = area.areaName;
			if( fireComplete )
			{
				CAtlString strVal;
				strVal.Format(_T("%s-%s-%s"),curSheng.name,curShi.name,curQuXian.name);
				fireComplete((void*)&strVal);
				Close();
			}
			return ;
		}
	}
	void ChooseArea::OnFinalMessage( HWND hWnd )
	{
		__super::OnFinalMessage(hWnd);
		if( logic )
			logic->OnRelease(this);
		delete this;
	}
	void ChooseArea::Notify(DuiLib::TNotifyUI& msg)
	{
		DuiLib::WindowImplBase::Notify(msg);
		if(msg.sType==DUI_MSGTYPE_WINDOWINIT)
		{
			DuiLib::CControlUI *loading = m_PaintManager.FindControl(_T("loading"));
			loading->SetVisible(true);
			if( logic==NULL )
				return ;
			//
			root = base::SafeConvert<DuiLib::CContainerUI>(loading->GetParent(),DUI_CTR_CONTAINER);
			layoutSheng = new DuiLib::CVerticalLayoutUI();
			layoutShi = new DuiLib::CVerticalLayoutUI();
			layoutQuxian = new DuiLib::CVerticalLayoutUI();
			root->Add(layoutSheng);
			root->Add(layoutShi);
			root->Add(layoutQuxian);

			logic->OnGetSheng();
		}
	}
	LRESULT ChooseArea::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if(uMsg==WM_KILLFOCUS)
		{
			Close();
			return 0;
		}
		if(uMsg==WM_KEYDOWN && wParam==VK_ESCAPE)
		{
			Close();
			return 0;
		}
		return DuiLib::WindowImplBase::HandleMessage(uMsg,wParam,lParam);
	}
	DuiLib::CControlUI* ChooseArea::CreateControl(LPCTSTR pstrClass)
	{
		if(0==_tcscmp(pstrClass,_T("ChooseHeaderLayout")))
			return new ChooseHeaderLayout();
		return __super::CreateControl(pstrClass);
	}

	void ChooseArea::ShowError(const CAtlString& tip)
	{
		DuiLib::CControlUI *uiTip = m_PaintManager.FindControl(_T("loading"));
		uiTip->SetVisible(true);
		uiTip->SetText(tip);
	}
	void ChooseArea::HideError()
	{
		DuiLib::CControlUI *uiTip = m_PaintManager.FindControl(_T("loading"));
		uiTip->SetVisible(false);
	}
	void ChooseArea::NotifyShengComplete(const Areas &vals)
	{
		//隐藏其他控件
		SetVisibleSheng(true);
		if(layoutSheng->GetCount())
		{
			layoutSheng->SetVisible(true);
			return ;
		}
		for(size_t i=0;i<vals.values.size();)
		{
			DuiLib::CHorizontalLayoutUI *row = new DuiLib::CHorizontalLayoutUI();
			for(size_t s=0;i<vals.values.size()&&s<4;i++,s++)
			{
				Areas::Area area = vals.values[i];
				DuiLib::COptionUI *opt = new DuiLib::COptionUI();
				opt->SetGroup(SHENG_GROUP);
				opt->SetFixedWidth(110);
				opt->SetText(area.areaName);
				opt->SetUserData(area.areaId);
				row->Add(opt);
			}
			row->SetFixedHeight(24);
			if(row->GetCount()==0)
			{
				delete row;
				row = NULL;
			}
			layoutSheng->Add(row);
		}
	}
	void ChooseArea::NotifyShiComplete(const Areas &vals)
	{
		SetVisibleShi(true);
		layoutShi->RemoveAll();		//清空所有数据
		for(size_t i=0;i<vals.values.size();)
		{
			DuiLib::CHorizontalLayoutUI *row = new DuiLib::CHorizontalLayoutUI();
			for(size_t s=0;i<vals.values.size()&&s<4;i++,s++)
			{
				Areas::Area area = vals.values[i];
				DuiLib::COptionUI *opt = new DuiLib::COptionUI();
				opt->SetGroup(SHI_GROUP);
				opt->SetFixedWidth(110);
				opt->SetText(area.areaName);
				opt->SetUserData(area.areaId);
				row->Add(opt);
			}
			if(row->GetCount()==0)
			{
				delete row;
				row = NULL;
			}
			row->SetFixedHeight(25);
			layoutShi->Add(row);
		}
	}
	void ChooseArea::NotifyQuXianComplete(const Areas &vals)
	{
		SetVisibleQuxian(true);
		layoutQuxian->RemoveAll();		//清空所有数据
		for(size_t i=0;i<vals.values.size();)
		{
			DuiLib::CHorizontalLayoutUI *row = new DuiLib::CHorizontalLayoutUI();
			for(size_t s=0;i<vals.values.size()&&s<4;i++,s++)
			{
				Areas::Area area = vals.values[i];
				DuiLib::COptionUI *opt = new DuiLib::COptionUI();
				opt->SetGroup(QUXIAN_GROUP);
				opt->SetFixedWidth(110);
				opt->SetText(area.areaName);
				opt->SetUserData(area.areaId);
				row->Add(opt);
			}
			if(row->GetCount()==0)
			{
				delete row;
				row = NULL;
			}
			row->SetFixedHeight(25);
			layoutQuxian->Add(row);
		}
	}
	void ChooseArea::SetVisibleSheng(bool v)
	{
		layoutSheng->SetVisible(false);
		layoutShi->SetVisible(false);
		layoutQuxian->SetVisible(false);
		layoutSheng->SetVisible(v);
	}
	void ChooseArea::SetVisibleShi(bool v)
	{
		layoutSheng->SetVisible(false);
		layoutShi->SetVisible(false);
		layoutQuxian->SetVisible(false);
		layoutShi->SetVisible(v);
	}
	void ChooseArea::SetVisibleQuxian(bool v)
	{
		layoutSheng->SetVisible(false);
		layoutShi->SetVisible(false);
		layoutQuxian->SetVisible(false);
		layoutQuxian->SetVisible(v);
	}
}