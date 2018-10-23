#include "stdafx.h"
#include "WinGuiUtil.h"
#pragma comment(lib,"Psapi.lib")

namespace WinGuiUtil
{
	BOOL CALLBACK FilterWinProc(HWND win,LPARAM param)
	{
		FindFilter *filter = reinterpret_cast<FindFilter*>((void*)param);
		if(!filter->Handle(win))
			return FALSE;
		return TRUE;	
	}
	bool FindChildByFilter(FindFilter *filter,HWND win)
	{
		if(win==NULL)
			win = ::GetDesktopWindow();
		EnumChildWindows(win,FilterWinProc,(LPARAM)(void*)filter);
		return true;
	}
	//
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
	//
	std::vector<HWND> GetChildren(HWND win)
	{
		std::vector<HWND> ret;
		HWND hChild = GetWindow(win,GW_CHILD);
		ret.push_back(hChild);
		while(hChild)
		{
			hChild = ::GetWindow(hChild, GW_HWNDNEXT);
			if(hChild==NULL)
				break;
			ret.push_back(hChild);
		} 
		return ret;
	}

	bool FindUnderDesktop(FindFilter *filter)
	{
		return ::EnumWindows(FilterWinProc,(LPARAM)(void*)filter);
	}

	CAtlString GetWinText(HWND win)
	{
		CAtlString ret;
		TCHAR chT[2048];
		memset(chT,0,sizeof(chT));
		int szNeed = ::GetWindowText(win,chT,sizeof(chT));
		if(szNeed > sizeof(chT))
		{
			szNeed += 1;
			TCHAR *pMem = (TCHAR*)malloc(szNeed*sizeof(TCHAR));
			memset(pMem,0,szNeed);
			szNeed = ::GetWindowText(win,pMem,szNeed);
			ret = pMem;		
			free(pMem);
		}
		else
		{
			ret = chT;
		}
		return ret;
	}
	CAtlString GetExePathByWin(HWND hwnd) 
	{
		CAtlString ExePath;

		TCHAR szPath[MAX_PATH]; 
		int ErrorCode = 0; 
		DWORD idProcess; 
		::GetWindowThreadProcessId(hwnd, &idProcess); 
		HANDLE hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, idProcess ); 
		if( NULL != hProcess ) 
		{ 
			HMODULE hMod; 
			DWORD cbNeeded; 
			if( ::EnumProcessModules( hProcess, &hMod, sizeof( hMod ), &cbNeeded ) ) 
			{ 
				DWORD dw = ::GetModuleFileNameEx( hProcess, hMod, szPath, MAX_PATH ); 
				if(dw > 0) 
				{ 
					ExePath = szPath; 
				} 
				else 
				{ 
					ErrorCode = GetLastError(); 
				} 
			} 
			CloseHandle( hProcess ); 
		}
		return ExePath; 
	}
	void ManClickIn(HWND win,int ptX,int ptY)
	{
		RECT rcWin={0,0,0,0};
		::GetWindowRect(win,&rcWin);
		::SetCursorPos(rcWin.left+ptX,rcWin.top+ptY);
		::mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);		 
		::mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);		
	}
	void ManClickOut(HWND win,int ptX,int ptY,bool lr,bool tb)
	{
		POINT pt = {0,0};
		RECT rcWin={0,0,0,0};
		::GetWindowRect(win,&rcWin);
		if(lr)
			pt.x = rcWin.right + ptX;
		else
			pt.x = rcWin.left + ptX;
		if(tb)
			pt.y = rcWin.bottom + ptY;
		else
			pt.y = rcWin.top + ptY;
		::SetCursorPos(pt.x,pt.y);
		::mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);		 
		::mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);		
	}
}