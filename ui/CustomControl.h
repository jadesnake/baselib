#pragma once

#include "ButtonUI.h"
#include "EditUI.h"
#include "ListExUI.h"
#include "MultipleText.h"
#include "ScannerEdit.h"
#include "OptionUI.h"
#include "TemplateClick.h"
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
		if(_tcscmp(pstrClass,_T("CustomUI::OptionUI"))==0)
		{
			return new CustomUI::OptionUI();
		}
		if(_tcscmp(pstrClass,_T("CustomUI::TextUI"))==0)
		{
			return new TemplateClick<DuiLib::CTextUI>();
		}
		if(_tcscmp(pstrClass,_T("CustomUI::LabelUI"))==0)
		{
			return new TemplateClick<DuiLib::CLabelUI>();
		}
		if(_tcscmp(pstrClass,_T("CustomUI::ListHeaderItemUI"))==0)
		{
			return new CustomUI::ListHeaderItemUI;
		}
		if(_tcscmp(pstrClass,_T("CustomUI::CListHeaderUI"))==0)
		{
			return new CustomUI::CListHeaderUI;
		}
		return base->CreateControl(pstrClass);
	}
	//ÊôÐÔ½Ó¿Ú
	template<class DuiWinBase>
	class WinBaseResT : public DuiWinBase
	{
	protected:
		DuiLib::CDuiString GetSkinFolder()
		{
			return DuiWinBase::GetSkinFolder();
		}
		DuiLib::UILIB_RESOURCETYPE GetResourceType() const
		{
			return DuiWinBase::GetResourceType();
		}
		LPCTSTR	GetResourceID() const
		{
			return DuiWinBase::GetResourceID();
		}
		DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass)
		{
			return CreateCustomUI(pstrClass,static_cast<DuiWinBase*>(this));
		}
	};
}