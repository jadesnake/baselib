#pragma once

#include "ButtonUI.h"
#include "EditUI.h"
#include "ListExUI.h"
#include "CalendarUI.h"
#include "MultipleText.h"
#include "ScannerEdit.h"
namespace CustomUI
{
	template<class T>
	DuiLib::CControlUI* CreateCustomUI(LPCTSTR pstrClass,T *base)
	{
		if(_tcscmp(pstrClass,_T("CustomUI::Edit"))==0)
		{
			return new CustomUI::EditUI();
		}
		if(_tcscmp(pstrClass,_T("CustomUI::Button"))==0)
		{
			return new CustomUI::ButtonUI();
		}
		if(_tcscmp(pstrClass,_T("CustomUI::List"))==0)
		{
			return new CustomUI::ListUI();
		}
		if(_tcscmp(pstrClass,_T("CustomUI::MultipleText"))==0)
		{
			return new CustomUI::MultipleText();
		}
		if(_tcscmp(pstrClass,_T("CustomUI::ScannerEdit"))==0)
		{
			return new CustomUI::ScannerEditUI();
		}
		return base->CreateControl(pstrClass);
	}
}