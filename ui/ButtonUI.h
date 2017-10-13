#pragma once

#include "../Base/MacroX.h"
namespace CustomUI
{
	class ButtonUI : public DuiLib::CButtonUI 
	{
	public:
		ButtonUI();
		~ButtonUI();
		DUI_DECLARE(CustomUI::ButtonUI,DuiLib::CButtonUI)
	protected:
		void PaintText(HDC hDC);
	};
}