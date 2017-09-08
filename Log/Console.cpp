/* 

Author: 
     Gregor S.
 
E-Mail: 
     foxhunter@hlla.net
 
Date: 
     08/20/2002 
 
Description: 
     This class is a simple wrapper for some console functions.
	 The main goal is to keep it that simple so that even
	 beginners have no problems implementing it in their apps.
	 You can write debug outout or anything else you wish into
	 the console in different colors.

Usage:
	 Copy Console.cpp (this file) and Console.h into the directory
	 of your application. Insert them into the workspace if you wish.
	 Next, add the line '#include "Console.h"' into one of your source
	 files and add a member variable of tyle CConsole.
 
Copyright & disclaimer: 
     Do want you want to do with this class: Modify it, extend it to your
	 needs. Use it in both Freeware, Shareware or commercial applications,
	 but make sure to leave these comments at the top of this source file 
	 intact and don't remove my name and E-mail address.

*/


#include "stdafx.h"
#include "Console.h"

// macro for the Clear() function
#define ERR(bSuccess) { if(!bSuccess) return; }
// macro to check whether hConsole is valid
#define CHECK(hHandle) { if(hHandle == NULL) return; };

bool CConsole::Create(LPCTSTR szTitle, bool bNoClose)
{
	// Has console been already created?
	if(hConsole != NULL)
		return false;
	// Allocate a new console for our app
	if(!AllocConsole())
		return false;
	
	// Create the actual console
	hConsole = CreateFile(_T("CONOUT$"), GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
	if(hConsole == INVALID_HANDLE_VALUE)
		return false;
	
	if(SetConsoleMode(hConsole, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT ) == 0)
		return false;
	
	// if true, disable the [x] button of the console
	if(bNoClose)
		DisableClose();
				
	// set the console title
	if(szTitle != NULL)
		SetConsoleTitle(szTitle);

	return true;
}

void CConsole::Color(WORD wColor)
{
	CHECK(hConsole);

	// no color specified, reset to defaults (white font on black background)
	if(wColor != NULL)
		SetConsoleTextAttribute(hConsole, wColor );
	// change font and/or background color
	else
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // white text on black bg
}

void CConsole::Output(LPCTSTR szOutput, ...)
{
	CHECK(hConsole);

	DWORD		dwWritten;
	TCHAR		out[512];
	va_list		va;
	
	// if not parameter set, write a new line
	if(szOutput == NULL) 
		_stprintf(out,_T("\n"));
	// process arguments
	else
	{
		va_start(va, szOutput);
			_vstprintf(out, szOutput, va);
		va_end(va);	
	}		
   
	// write to the console
	WriteConsole(hConsole,out,_tcslen(out),&dwWritten,0);
}

void CConsole::SetTitle(LPCTSTR title)
{
	// self-explanatory
	SetConsoleTitle(title);
}

LPCTSTR CConsole::GetTitle()
{
	// get the title of our console and return it
	static TCHAR szWindowTitle[512] = _T("");
	GetConsoleTitle(szWindowTitle,sizeof(szWindowTitle));

	return szWindowTitle;
}


HWND CConsole::GetHWND()
{
	if(hConsole == NULL) 
		return NULL;

	// try to find our console window and return its HWND
	return FindWindow(_T("ConsoleWindowClass"),GetTitle());
}

void CConsole::Show(bool bShow)
{
	CHECK(hConsole);

	// get out console's HWND and show/hide the console
	HWND hWnd = GetHWND();
	if(hWnd != NULL)
		ShowWindow(hWnd, SW_HIDE ? SW_SHOW : bShow);
}

void CConsole::DisableClose()
{
	CHECK(hConsole);

	HWND hWnd = GetHWND();
	
	// disable the [x] button if we found our console
	if(hWnd != NULL)
	{
		HMENU hMenu = GetSystemMenu(hWnd,0);
		if(hMenu != NULL)
		{
			DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
			DrawMenuBar(hWnd);
		}
	}
}


void CConsole::Clear()
{
	CHECK(hConsole);

	/***************************************/
	// This code is from one of Microsoft's
	// knowledge base articles, you can find it at 
    // http://support.microsoft.com/default.aspx?scid=KB;EN-US;q99261&
	/***************************************/

	COORD coordScreen = { 0, 0 };

	BOOL bSuccess;
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */ 
    DWORD dwConSize; 

	/* get the number of character cells in the current buffer */ 

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    ERR(bSuccess);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    /* fill the entire screen with blanks */ 

    bSuccess = FillConsoleOutputCharacter( hConsole, (TCHAR) ' ', dwConSize, coordScreen, &cCharsWritten );
    ERR(bSuccess);

    /* get the current text attribute */ 

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    ERR(bSuccess);

    /* now set the buffer's attributes accordingly */ 

    bSuccess = FillConsoleOutputAttribute( hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten );
    ERR(bSuccess);

    /* put the cursor at (0, 0) */ 

    bSuccess = SetConsoleCursorPosition( hConsole, coordScreen );
    ERR(bSuccess);
}


HANDLE CConsole::GetHandle()
{
	// simply return the handle to our console
	return hConsole;
}

void CConsole::Close()
{
	// free the console, now it can't be used anymore until we Create() it again
	FreeConsole();
	hConsole = NULL;
}