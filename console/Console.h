#if !defined(__CONSOLE_H_)
#define __CONSOLE_H_


class CConsole
{
public:
	CConsole() 
	{ 
		hConsole = NULL;
	};
public:
	enum LEVEL 
	{
		LV_ERROR,
		LV_WARRING,
		LV_TRACE,
	};
	//   控制台前景颜色
	enum ConsoleForegroundColor
	{
		enmCFC_Red          = FOREGROUND_INTENSITY | FOREGROUND_RED,
		enmCFC_Green        = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		enmCFC_Blue         = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
		enmCFC_Yellow       = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
		enmCFC_Purple       = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
		enmCFC_Cyan         = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
		enmCFC_Gray         = FOREGROUND_INTENSITY,
		enmCFC_White        = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		enmCFC_HighWhite    = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		enmCFC_Black        = 0,
	};
	enum ConsoleBackGroundColor
	{
		enmCBC_Red          = BACKGROUND_INTENSITY | BACKGROUND_RED,
		enmCBC_Green        = BACKGROUND_INTENSITY | BACKGROUND_GREEN,
		enmCBC_Blue         = BACKGROUND_INTENSITY | BACKGROUND_BLUE,
		enmCBC_Yellow       = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN,
		enmCBC_Purple       = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE,
		enmCBC_Cyan         = BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE,
		enmCBC_White        = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
		enmCBC_HighWhite    = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
		enmCBC_Black        = 0,
	};
	// create the console
	bool   Create(LPCTSTR szTitle, bool bNoClose = false);
	// set color for output
	void   Color(WORD wColor = NULL);
	// write output to console
	void   Output(LEVEL lv,LPCTSTR szOutput = NULL, ...);
	void   Output(LEVEL lv,const char* szOutput = NULL, ...);
	void   Output(LPCTSTR szOutput = NULL, ...);
	// set and get title of console
	void   SetTitle(LPCTSTR szTitle);
	LPCTSTR  GetTitle();
	// get HWND and/or HANDLE of console
	HWND   GetHWND();
	HANDLE GetHandle();

	// show/hide the console
	void   Show(bool bShow = true);
	// disable the [x] button of the console
	void   DisableClose();
	// clear all output
	void   Clear();

	// close the console and delete it
	void   Close();

private:
	HANDLE hConsole;
};

#endif 