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
	// create the console
	bool   Create(LPCTSTR szTitle, bool bNoClose = false);
	
	// set color for output
	void   Color(WORD wColor = NULL);
	// write output to console
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