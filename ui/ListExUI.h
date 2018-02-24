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
		DUI_DECLARE(CustomUI::ListUI,DuiLib::CListUI)
	protected:

	};
}