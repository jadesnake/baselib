#include "stdafx.h"
#include <assert.h>
#include "FileDialog.h"
namespace CustomUI
{
	UINT_PTR CALLBACK OFN_HOOKProc(HWND hWnd,UINT unMsg,WPARAM wParam,LPARAM lParam)
	{
		if( unMsg == WM_INITDIALOG )
		{
			LPOPENFILENAME lpName = (LPOPENFILENAME)lParam;

			return 0;
		}
		if(unMsg == WM_NOTIFY)
		{
			LPOFNOTIFY lpOfNotify = (LPOFNOTIFY)lParam;
			if( lpOfNotify->hdr.code == CDN_FILEOK )
			{
				FileDlgEvent *pDlg = (FileDlgEvent*)(void*)(lpOfNotify->lpOFN->lCustData);
				if( pDlg->OnOK(lpOfNotify) )
					return 0;
				SetWindowLong(hWnd,DWL_MSGRESULT,-1);
				return -1;
			}
			if( lpOfNotify->hdr.code == CDN_SHAREVIOLATION )
			{
				FileDlgEvent *pDlg = (FileDlgEvent*)(void*)(lpOfNotify->lpOFN->lCustData);
				if( pDlg->OnSave(lpOfNotify) )
					return 0;
				SetWindowLong(hWnd,DWL_MSGRESULT,-1);
				return -1;
			}
			return 0;
		}
		return 0;
	}
	CFileDialog::CFileDialog(bool bOpenFileDialog,LPCTSTR lpszDefExt,LPCTSTR lpszFileName,DWORD dwFlags,
		LPCTSTR lpszFilter,HWND hParent,DWORD dwSize,BOOL bVistaStyle,LPCTSTR lpszIniDir)
				:m_bOpenFileDialog(bOpenFileDialog)
	{
		dwFlags |=  OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
		memset(m_szFileName,0,sizeof(m_szFileName));
		memset(m_szFileTitle,0,sizeof(m_szFileTitle));
		memset(&m_of,0,sizeof(OPENFILENAME));
		m_of.lStructSize = sizeof(OPENFILENAME);
		m_of.lpstrDefExt = lpszDefExt;
		m_of.lpstrFilter = lpszFilter;
		m_of.hwndOwner	 = hParent;
		m_of.lpstrFile	 = m_szFileName;
		m_of.lpstrFileTitle= (LPTSTR)m_szFileTitle;
		m_of.nMaxFile	   = _countof(m_szFileName);
		m_of.nMaxFileTitle = _countof(m_szFileTitle);
		//m_of.lpstrInitialDir = lpszFileName;
		if (bOpenFileDialog)
		{
			m_of.lpstrInitialDir = lpszFileName;
		}
		else
		{
			m_of.lpstrInitialDir = lpszIniDir;
		}
		if(lpszFilter != NULL)
		{
			m_strFilter = lpszFilter;
			LPTSTR pch = m_strFilter.GetBuffer(0); // modify the buffer in place
			while ((pch = _tcschr(pch,'|')) != NULL)
				*pch++ = '\0';
			m_of.lpstrFilter = m_strFilter;
			// do not call ReleaseBuffer() since the string contains ¡¯\0¡¯ characters
		}
	}
	CFileDialog::~CFileDialog()
	{

	}
	void	CFileDialog::SetShowName( LPCTSTR lpszFileName,bool bExtern )
	{
		if( lpszFileName == 0 )
			return ;
		StrCpy(m_szFileName,lpszFileName);
		PathStripPath(m_szFileName);
		if( bExtern == false )
			PathRemoveExtension(m_szFileName);
	}
	CAtlString CFileDialog::GetFileExt()  const
	{
		return CAtlString(m_of.lpstrFile + m_of.nFileExtension);
	}
	CAtlString CFileDialog::GetFileName() const
	{
		return CAtlString(m_of.lpstrFileTitle);
	}
	CAtlString CFileDialog::GetNextPathName(void* &pos)
	{
		BOOL bExplorer = m_of.Flags & OFN_EXPLORER;
		TCHAR chDelimiter;
		if (bExplorer)
			chDelimiter = '\0';
		else
			chDelimiter = ' ';
		LPTSTR lpsz = (LPTSTR)pos;
		if (lpsz == m_of.lpstrFile) // first time
		{
			if ((m_of.Flags & OFN_ALLOWMULTISELECT) == 0)
			{
				pos = NULL;
				return m_of.lpstrFile;
			}
			// find char pos after first Delimiter
			while(*lpsz != chDelimiter && *lpsz != '\0')
				lpsz = _tcsinc(lpsz);
			lpsz = _tcsinc(lpsz);
			// if single selection then return only selection
			if (*lpsz == 0)
			{
				pos = NULL;
				return m_of.lpstrFile;
			}
		}
		CString strPath = m_of.lpstrFile;
		if (!bExplorer)
		{
			LPTSTR lpszPath = m_of.lpstrFile;
			while(*lpszPath != chDelimiter)
				lpszPath = _tcsinc(lpszPath);
			strPath = strPath.Left(lpszPath - m_of.lpstrFile);
		}
		LPTSTR lpszFileName = lpsz;
		CString strFileName = lpsz;
		// find char pos at next Delimiter
		while(*lpsz != chDelimiter && *lpsz != '\0')
			lpsz = _tcsinc(lpsz);
		if (!bExplorer && *lpsz == '\0')
			pos = NULL;
		else
		{
			if (!bExplorer)
				strFileName = strFileName.Left(lpsz - lpszFileName);
			lpsz = _tcsinc(lpsz);
			if (*lpsz == '\0') // if double terminated then done
				pos = NULL;
			else
				pos = (void*)lpsz;
		}
		// only add ¡¯\\¡¯ if it is needed
		if (!strPath.IsEmpty())
		{
			// check for last back-slash or forward slash (handles DBCS)
			LPCTSTR lpszTmp = _tcsrchr(strPath, '\\');
			if (lpszTmp == NULL)
				lpszTmp = _tcsrchr(strPath, '/');
			// if it is also the last character, then we don¡¯t need an extra
			if (lpszTmp != NULL &&
				(lpszTmp - (LPCTSTR)strPath) == strPath.GetLength()-1)
			{
				//ASSERT(*lpszTmp == '\\' || *lpszTmp == '/');
				return strPath + strFileName;
			}
		}
		return strPath + '\\' + strFileName;
	}
	CAtlString CFileDialog::GetPathName() const
	{	return CAtlString(m_of.lpstrFile);	}
	UINT CFileDialog::DoModal()
	{
		int nResult = 0;
		DWORD nOffset = lstrlen(m_of.lpstrFile)+1;
		memset(m_of.lpstrFile+nOffset, 0, (m_of.nMaxFile-nOffset)*sizeof(TCHAR));
		HWND hWndFocus = ::GetFocus();
		BOOL bEnableParent = FALSE;
		if(m_of.hwndOwner != NULL && ::IsWindowEnabled(m_of.hwndOwner))
		{
			bEnableParent = TRUE;
			::EnableWindow(m_of.hwndOwner, FALSE);
		}
		TCHAR curPath[MAX_PATH+1]={0};
		::GetCurrentDirectory(MAX_PATH,curPath);
		if (m_bOpenFileDialog)
			nResult = ::GetOpenFileName(&m_of);
		else
			nResult = ::GetSaveFileName(&m_of);
		if (bEnableParent)
			::EnableWindow(m_of.hwndOwner, TRUE);
		if (::IsWindow(hWndFocus))
			::SetFocus(hWndFocus);
		::SetCurrentDirectory(curPath);
		return nResult ? nResult : IDCANCEL;
	}
	void CFileDialog::SetHook(FileDlgEvent *pEvent)
	{
		m_of.Flags |= OFN_ENABLEHOOK|OFN_EXPLORER;
		m_of.lpfnHook = OFN_HOOKProc;
		m_of.lCustData= (LPARAM)(void*)pEvent;
	}

};