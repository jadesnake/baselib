#include "stdafx.h"
#include "UIlib.h"
#include "CustomControl.h"
namespace CustomUI
{
	BOOL CALLBACK ChildAllProc(HWND win,LPARAM param)
	{
		std::vector<HWND> *out = reinterpret_cast<std::vector<HWND>*>((void*)param);
		if(::IsWindow(win))
			out->push_back(win);
		return TRUE;
	}
	std::vector<HWND> GetAllChildren(HWND win)
	{
		std::vector<HWND> ret;
		EnumChildWindows(win,ChildAllProc,(LPARAM)&ret);
		return ret;
	}
}