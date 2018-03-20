#pragma once

#include "../Base/MacroX.h"
namespace CustomUI
{
	class ListUI : public DuiLib::CListUI
	{
	public:
		ListUI(void);
		//修正 优先计算listbody item，而此时并没有计算Column导致listbody item pos计算不正确
		void SetPos(RECT rc);
		void ScrollAt(DuiLib::CControlUI *item);
		void SetBkImage(LPCTSTR pStrImage);
		void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
		DUI_DECLARE(CustomUI::ListUI,DuiLib::CListUI)
	protected:
		void PaintBkImage(HDC hDC);
		RECT GetImgDest(LPCTSTR img);
		void RatioW(RECT rc,double inW,double &outW);
		void RatioH(RECT rc,double inH,double &outH);
		void ReBkImgDest(RECT rc);
	protected:
		RECT rcBkImg;	//背景图的左右间距原始相对位置和宽高
	};
	//
	class ListHeaderItemUI : public DuiLib::CListHeaderItemUI
	{
	public:
		ListHeaderItemUI();
		void SetOwner(DuiLib::CListUI *listUI);
		bool Add(DuiLib::CControlUI* pControl);
		DUI_DECLARE(CustomUI::ListHeaderItemUI,DuiLib::CListHeaderItemUI)
	protected:
		void DoEvent(DuiLib::TEventUI& event);
		void Init();
		void LoopSetOwner(DuiLib::CControlUI* pControl);
		void SetChildrenOwner();
	private:
		DuiLib::CListUI *mList;
	};
	//
	class CListHeaderUI : public DuiLib::CListHeaderUI
	{
	public:
		CListHeaderUI();
		bool Add(DuiLib::CControlUI* pControl);
		DUI_DECLARE(CustomUI::CListHeaderUI,DuiLib::CListHeaderUI)
	protected:
		void Init();
	};
}