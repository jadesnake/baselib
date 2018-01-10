#pragma once

#include "../Base/MacroX.h"
namespace CustomUI
{
	class OptionUI : public DuiLib::COptionUI 
	{
	public:
		OptionUI();
		~OptionUI();
		DUI_DECLARE(CustomUI::OptionUI,DuiLib::COptionUI)
	protected:
		void PaintStatusImage(HDC hDC);
	};
}