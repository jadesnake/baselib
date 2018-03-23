#pragma once

#include <Commdlg.h>
namespace CustomUI
{
	class FileDlgEvent
	{
	public:
		virtual bool OnSave( LPOFNOTIFY lpNotify ) = 0;
		virtual bool OnOK( LPOFNOTIFY lpNotify ) = 0;
	};
	class CFileDialog
	{
	public:
		CFileDialog(bool bOpenFileDialog,LPCTSTR lpszDefExt = NULL,
					LPCTSTR lpszFileName = NULL,DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
					LPCTSTR lpszFilter = NULL,HWND hParent=NULL,DWORD dwSize = 0,BOOL bVistaStyle = TRUE,
					LPCTSTR lpszIniDir = NULL);
		~CFileDialog();
		UINT DoModal();
		CAtlString GetFileExt()  const;
		CAtlString GetFileName() const;
		CAtlString GetNextPathName(void* &n);
		CAtlString GetPathName() const;
		void	SetHook(FileDlgEvent *pEvent);
		void	SetShowName( LPCTSTR lpszFileName,bool bExtern = true );
	public:
		OPENFILENAME m_of;
	private:
		bool	m_bOpenFileDialog;
		TCHAR	m_szFileName[MAX_PATH+1];
		TCHAR	m_szFileTitle[64];
		CAtlString	m_strFilter;
	};
};
