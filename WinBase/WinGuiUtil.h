#pragma once
#include <vector>
#include <Psapi.h>
#include <Shlobj.h>
#include <shellapi.h>
namespace WinGuiUtil
{
	class FindFilter
	{
	public:
		virtual ~FindFilter(){}
		virtual bool Handle(HWND win)
		{
			return true;
		}
	};
	std::vector<HWND> GetAllChildren(HWND win);

	std::vector<HWND> GetChildren(HWND win);

	bool FindChildByFilter(FindFilter *filter,HWND win=NULL);

	bool FindUnderDesktop(FindFilter *filter);

	CAtlString GetWinText(HWND win);

	CAtlString GetExePathByWin(HWND hwnd);

	void ManClickIn(HWND win,int ptX,int ptY);
	void ManClickOut(HWND win,int ptX,int ptY,bool lr=false,bool tb=true);
}