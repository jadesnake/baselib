#include "StdAfx.h"
#include <shlobj.h>
#include <io.h>
#include <ShellAPI.h>
#include "Dir.h"
#pragma comment(lib, "shell32.lib")

CAtlString	GetCurDir(void)
{
	CAtlString strRet;
	LPTSTR	   pTmp = NULL;
	//缓存当前工作目录
	DWORD dwDirlen = GetCurrentDirectory(0,NULL);
	pTmp = new TCHAR[dwDirlen+1];
	ZeroMemory(pTmp,dwDirlen+1);
	GetCurrentDirectory(dwDirlen,pTmp);
	strRet=pTmp;
	delete []pTmp;
	return strRet;
}
/*------------------------------------------------------------------------*/
int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
	LPCTSTR pDefault = (LPCTSTR)(void*)lpData;
	switch(uMsg)
	{
	case BFFM_INITIALIZED:
		{
			if( pDefault )
				SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)(void*)pDefault);

		}
		break;
	case BFFM_SELCHANGED:    //选择路径变化，
		{
			TCHAR curr[MAX_PATH];   
			SHGetPathFromIDList((LPCITEMIDLIST)lParam,curr);   
			::SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)curr);   
		}
		break;
	case BFFM_VALIDATEFAILED:
		{
			LPCTSTR pValid = (LPCTSTR)(void*)lParam;
			CAtlString	strMsg;
			strMsg.Format(_T("%s 无效"),pValid);
			::MessageBox(hwnd,strMsg,_T("提示"),MB_OK|MB_ICONERROR);
		}
		break;
	default:
		break;
	}
	return 0;   
}
/*------------------------------------------------------------------------*/
LONGLONG  QueryFileSize( LPCTSTR strPath )
{
	LARGE_INTEGER	lRet;
	lRet.QuadPart = 0;
	HANDLE hFile = CreateFile(strPath,GENERIC_READ|FILE_SHARE_READ|FILE_SHARE_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if( hFile == INVALID_HANDLE_VALUE )
	{
		WIN32_FIND_DATA	pData = { 0 };
		HANDLE hFind = INVALID_HANDLE_VALUE;	
		hFind = FindFirstFile(strPath,&pData);
		if( hFind == INVALID_HANDLE_VALUE )
			return 0;
		FindClose(hFind);
		hFind = INVALID_HANDLE_VALUE;
		lRet.LowPart = pData.nFileSizeLow;
		lRet.HighPart= pData.nFileSizeHigh;
		return lRet.QuadPart;
	}
	GetFileSizeEx(hFile,&lRet);
	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;
	return lRet.QuadPart;
}
/*------------------------------------------------------------------------*/
bool  GetFilePath(LPCTSTR pV,LPTSTR pOut,size_t outNum)
{
	TCHAR chPath[MAX_PATH+1];
	if( pV == NULL )			return NULL;
	if( ::PathIsDirectory(pV) )	
	{
		_tcscpy_s(pOut,outNum,pV);
		return true;
	}
	size_t nLen = _tcsclen(pV);
	for(nLen;nLen > 0;nLen--)
	{
		memset(chPath,0,sizeof(chPath));
		if( ('\\' == pV[nLen])||('/' == pV[nLen]) ) 
		{
			memcpy(chPath,pV,nLen*sizeof(TCHAR));
			if( ::PathIsDirectory(chPath) )
			{
				if( 2 == _tcsclen(chPath) )
					_tcscat_s(chPath,_T("\\"));
				_tcscpy_s(pOut,outNum,chPath);
				return true;
			}
		}
	}
	return false;
}
/*------------------------------------------------------------------------*/
bool  OpenSelPathDialog(LPTSTR outSel,HWND hOwner,LPCTSTR pDefaultDir,LPCTSTR pTitle)
{
	BROWSEINFO	bi;
	bool		bRet = false;
	bi.hwndOwner = hOwner;
	bi.pidlRoot  = NULL;
	bi.pszDisplayName = NULL;
	bi.lpszTitle = pTitle;
	bi.ulFlags =  BIF_EDITBOX|BIF_NEWDIALOGSTYLE;	
	bi.lpfn	   = BrowseCallbackProc;
	bi.lParam  = (LPARAM)(void*)pDefaultDir;
	bi.iImage  = NULL;
	//初始化入口参数bi结束
	InitCommonControls();
	LPITEMIDLIST pIDList = ::SHBrowseForFolder(&bi);//调用显示选择对话框	
	if( pIDList )
	{
		if( !::SHGetPathFromIDList(pIDList,outSel) )
			bRet = false;
		else
			bRet = true;
		IMalloc * imalloc = 0;
		if( SUCCEEDED(SHGetMalloc( &imalloc)) )
		{
			imalloc->Free(pIDList);
			imalloc->Release();
		}
	}
	return bRet;
}
/*------------------------------------------------------------------------*/
DWORD 	EnumFloderItem(LPCTSTR lpRoot,DWORD dwAttributes,
					   LPVOID lpParam,DirFilter *filter)
{
	HANDLE	hFind	= NULL;
	DWORD	dwError	= 0;
	WIN32_FIND_DATA	tgFindData = { 0 };
	TCHAR	chFullPath[MAX_PATH+1] = { 0 };
	try
	{
		hFind	=	FindFirstFile(lpRoot,&tgFindData);
		if(hFind == INVALID_HANDLE_VALUE)
			throw (DWORD)GetLastError();
		_tcscpy_s(chFullPath,MAX_PATH,lpRoot);
		PathRemoveFileSpec(chFullPath);
		do
		{
			if(  (dwAttributes & tgFindData.dwFileAttributes) == dwAttributes)
			{
				CAtlString strItemF(chFullPath);
				PathAppend(chFullPath,tgFindData.cFileName);
				if( filter )
					dwError = filter->DirEnumItemProc(chFullPath,lpParam,&tgFindData);
				memset(chFullPath,0,sizeof(chFullPath));
				_tcscpy(chFullPath,strItemF.GetString());
				if(dwError)	throw dwError; 
			}
		}while(FindNextFile(hFind, &tgFindData));
		dwError	=	GetLastError();
		if(dwError != ERROR_NO_MORE_FILES)
			throw dwError;
		FindClose(hFind);
		hFind = NULL;
	}
	catch (DWORD dwCode)
	{
		if(hFind)
		{
			FindClose(hFind);
			hFind = NULL;
		}
		return dwCode;
	}
	return 0;
}
/*------------------------------------------------------------------------*/
DWORD  CheckDir( LPCTSTR pDir,BOOL bCreate,LPSECURITY_ATTRIBUTES lpSecurity)
{
	DWORD dwRet = 0;	
	LPTSTR pSub  = NULL,pRoot=NULL;
	CAtlString strTmp,strDir(pDir);
	strDir.Replace('/','\\');
	if(strDir.IsEmpty())
		return -1;
	int nDot = strDir.ReverseFind('.');
	if( nDot != -1 )
	{
		int nSlashes = 0;
		nSlashes = strDir.ReverseFind('\\');
		if(nSlashes==-1)
			return -1; //路径格式不正确
		strDir = strDir.Mid(0,nSlashes);
	}
	if(strDir[strDir.GetLength()-1]!='\\')
		strDir += '\\';
	pRoot = (LPTSTR)strDir.GetString();
	while(pSub = ::PathFindNextComponent(pRoot))
	{
		strTmp.Append(pRoot,pSub-pRoot);
		pRoot = pSub;
		if( !PathFileExists(strTmp) && !PathIsRoot(strTmp) )
		{
			if( bCreate )
			{
				if(!CreateDirectory(strTmp,lpSecurity))
				{
					//失败则记录日志并返回异常代码
					dwRet = GetLastError();
					break;
				}
			}
			else
			{
				dwRet = -1;
				break;
			}
		}
	}
	return dwRet;
}
/*------------------------------------------------------------------------*/
void GetFileListFromDir_internal(CAtlString path, BOOL recursive, OUT CSimpleArray<CAtlString>& ay)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	CAtlString spec = path + _T("\\*");
	hFind = FindFirstFile(spec, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			CAtlString fileName = FindFileData.cFileName;
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (recursive && fileName != _T(".") && fileName != _T(".."))
				{
					TCHAR full[MAX_PATH];
					PathCombine(full, path, fileName);
					GetFileListFromDir_internal(full, recursive, ay);
				}
			}
			else
			{
				TCHAR full[MAX_PATH];
				PathCombine(full, path, fileName);
				ay.Add(full);
			}

		} while (FindNextFile(hFind, &FindFileData));

		FindClose(hFind);
	}
}
/*------------------------------------------------------------------------*/
CSimpleArray<CAtlString> GetFileListFromDir(CAtlString path, BOOL recursive)
{
	CSimpleArray<CAtlString> ay;

	GetFileListFromDir_internal(path, recursive, ay);

	return ay;
}

//获取动态连接库(dll）所在路径
//pszFileName:dll文件名(不包含路径)
CAtlString GetDllPath(LPCTSTR pszFileName)
{
	TCHAR sFilename[_MAX_PATH];
	TCHAR sDrive[_MAX_DRIVE];
	TCHAR sDir[_MAX_DIR];
	TCHAR sFname[_MAX_FNAME];
	TCHAR sExt[_MAX_EXT];
	GetModuleFileName(GetModuleHandle(pszFileName), sFilename, _MAX_PATH);
	_tsplitpath_s(sFilename, sDrive, sDir, sFname, sExt);
	CAtlString strFilePath(CAtlString(sDrive) + CAtlString(sDir));
	return strFilePath;
}

//获取程序所在目录
CAtlString GetAppPath()
{
	TCHAR sFilename[_MAX_PATH];
	TCHAR sDrive[_MAX_DRIVE];
	TCHAR sDir[_MAX_DIR];
	TCHAR sFname[_MAX_FNAME];
	TCHAR sExt[_MAX_EXT];
	//GetModuleFileName( GetModuleHandle(NULL), sFilename, _MAX_PATH);
	GetModuleFileName( NULL, sFilename, _MAX_PATH);
	_tsplitpath_s(sFilename, sDrive, sDir, sFname, sExt);
	CAtlString filename(CAtlString(sDrive) + CAtlString(sDir));
	return filename;
}

CAtlString getAppFilePath()
{
	TCHAR sFilename[_MAX_PATH];
	TCHAR sDrive[_MAX_DRIVE];
	TCHAR sDir[_MAX_DIR];
	TCHAR sFname[_MAX_FNAME];
	TCHAR sExt[_MAX_EXT];
	//GetModuleFileName( GetModuleHandle(NULL), sFilename, _MAX_PATH);
	GetModuleFileName( NULL, sFilename, _MAX_PATH);
	return (CAtlString)sFilename;
}
//判断文件或者目录是否存在
BOOL IsPathFind( const CAtlString& strPath )
{
	return (GetFileAttributes(strPath) != INVALID_FILE_ATTRIBUTES);
}
CAtlString FindFrontPath(const CAtlString& path)
{
	CAtlString rEmpty(path);
	if(path.IsEmpty())
		return rEmpty;
	rEmpty.Replace('/','\\');
	int endpos=0;
	do 
	{
		endpos = rEmpty.ReverseFind('\\');
		if(endpos==0||endpos==-1)
		{
			rEmpty.Empty();
			return rEmpty;
		}
		if((endpos+1)==rEmpty.GetLength())
		{
			rEmpty.Delete(endpos);
			continue;
		}
		break;
	}while(1);
	return rEmpty.Mid(0,endpos);
}
/*-----------------------------------------------------------------------------------*/
CAtlString GetSystemPath(DWORD sp)
{
	TCHAR lpszDefaultDir[MAX_PATH];
	TCHAR szDocument[MAX_PATH] = { 0 };
	memset(lpszDefaultDir, 0, sizeof(lpszDefaultDir));
	LPITEMIDLIST pidl = NULL;
	::SHGetSpecialFolderLocation(NULL, sp, &pidl);
	if (pidl && SHGetPathFromIDList(pidl, szDocument))
	{
		GetShortPathName(szDocument, lpszDefaultDir, _MAX_PATH);
	}
	CString ret = lpszDefaultDir;
	TCHAR end = ret[ret.GetLength() - 1];
	if ('\\' != end && end != '/') {
		ret += '\\';
	}
	if (_waccess(CT2CW(ret), 0) == -1) {
		_wmkdir(CT2CW(ret));
	}
	return ret;
}

CAtlString GetLocalAppDataPath()
{
	return GetSystemPath(CSIDL_LOCAL_APPDATA);
}

CAtlString GetDocPath()
{
	return GetSystemPath(CSIDL_COMMON_DOCUMENTS);
}

CAtlString GetDesktopPath()
{
	return GetSystemPath(CSIDL_DESKTOP);
}
CAtlString GetProgramsPath()
{
	return GetSystemPath(CSIDL_PROGRAMS);
}
CAtlString GetUsrDesktopPath()
{
	return GetSystemPath(CSIDL_COMMON_DESKTOPDIRECTORY);
}
CAtlString GetCommonProgramsPath()
{
	return GetSystemPath(CSIDL_COMMON_PROGRAMS);
}

/*-----------------------------------------------------------------------------------*/
#if defined(_UNICODE)
typedef DWORD(WINAPI* GetProcessFileNameW)(HANDLE, HMODULE, LPWSTR, DWORD);
#else
typedef DWORD(WINAPI* GetProcessFileNameA)(HANDLE, HMODULE, LPASTR, DWORD);
#endif
CString GetProcessFullName(HANDLE h) {
	CString ret;
	HMODULE hPSapi = ::LoadLibrary(_T("Psapi.DLL"));
	if (hPSapi == NULL)
		return ret;
#if defined(_UNICODE)
	GetProcessFileNameW procfilename = (GetProcessFileNameW)::GetProcAddress(hPSapi, "GetModuleFileNameExW");
	if (procfilename == NULL)
		return ret;
	WCHAR path[MAX_PATH + 1] = { 0 };
	memset(path, 0, sizeof(path));
	procfilename(h, NULL, path, MAX_PATH);
	ret = CW2CT(path);
#else
	GetProcessFileNameA procfilename = (GetProcessFileNameW)::GetProcAddress(hPSapi, "GetModuleFileNameExA");
	if (procfilename == NULL)
		return ret;
	CHAR path[MAX_PATH + 1] = { 0 };
	memset(path, 0, sizeof(path));
	procfilename(h, NULL, path, MAX_PATH);
	ret = CA2CT(path);
#endif
	::FreeLibrary(hPSapi);
	return ret;
}
/*---------------------------------------------------------------------------------------------*/
bool DeleteDir(const CAtlString &dir)
{
	SHFILEOPSTRUCT  shDelFile;
	memset(&shDelFile, 0, sizeof(SHFILEOPSTRUCT));
	shDelFile.fFlags |= FOF_SILENT;				//不显示进度
	shDelFile.fFlags |= FOF_NOERRORUI;			//不报告错误信息
	shDelFile.fFlags |= FOF_NOCONFIRMATION;		//直接删除，不进行确认
	// 复制路径到一个以双NULL结束的string变量里
	TCHAR buf[_MAX_PATH + 1];
	_tcscpy_s(buf,_MAX_PATH,dir);				// 复制路径
	buf[_tcslen(buf) + 1] = 0;		// 在末尾加两个NULL
	// 设置SHFILEOPSTRUCT的参数为删除做准备
	shDelFile.wFunc = FO_DELETE;	// 执行的操作
	shDelFile.pFrom = buf;			// 操作的对象，也就是目录
	shDelFile.pTo = NULL;			// 必须设置为NULL
	shDelFile.fFlags &= ~FOF_ALLOWUNDO;		//直接删除，不进入回收站
	return SHFileOperation(&shDelFile) == 0;
}
/*---------------------------------------------------------------------------------------------*/
CAtlString AppendUrl(const CAtlString& a, const CAtlString& b)
{
	CAtlString ret(a);
	bool hFlag = false;
	TCHAR ch = 0;
	if (!a.IsEmpty()) {
		ch = a[a.GetLength() - 1];
		if (ch == '/' || ch == '\\')
			hFlag = true;
	}
	if (b.IsEmpty())
		return ret;
	ch = b[0];
	if (ch == '/' || ch == '\\') {
		if (hFlag)
			ret.Delete(ret.GetLength() - 1, 1);
		ret += b;
		return ret;
	}
	if (hFlag) {
		ret += b;
		return ret;
	}
	ret += '\\' + b;
	return ret;
}
/*---------------------------------------------------------------------------------------------*/
CAtlString PathGetFileName(const CAtlString& f) 
{
	CAtlString ret(f);
	int nEnd = 0;
	int nEnd_L = ret.ReverseFind('\\');
	int nEnd_R = ret.ReverseFind('/');
	int nDot = ret.ReverseFind('.');
	if (nEnd_L == -1 && nEnd_R == -1 && nDot == -1)
		return ret;
	if (nEnd_L == -1 && nEnd_R == -1 && nDot)
	{
		ret = ret.Mid(0, nDot);
		return ret;
	}
	nEnd = nEnd_L > nEnd_R ? nEnd_L : nEnd_R;
	ret = ret.Mid(nEnd + 1, nDot - nEnd - 1);
	return ret;
}
/*---------------------------------------------------------------------------------------------*/
CAtlString FindFilePath(const CAtlString& path) 
{
	CAtlString ret(path);
	int nEnd = 0;

	int nEnd_L = ret.ReverseFind('\\');
	int nEnd_R = ret.ReverseFind('/');
	if (nEnd_L == -1 && nEnd_R == -1)
		return ret;
	nEnd = nEnd_L > nEnd_R ? nEnd_L : nEnd_R;
	if ( (nEnd+1) == ret.GetLength()) {
		ret.Delete(nEnd,1);
		return FindFilePath(ret);
	}
	ret = ret.Mid(0, nEnd + 1);
	return ret;
}
/*---------------------------------------------------------------------------------------------*/
CAtlString GetAppName()
{
	TCHAR sFilename[_MAX_PATH];
	TCHAR sDrive[_MAX_DRIVE];
	TCHAR sDir[_MAX_DIR];
	TCHAR sFname[_MAX_FNAME];
	TCHAR sExt[_MAX_EXT];
	GetModuleFileName(NULL, sFilename, _MAX_PATH);
	_tsplitpath_s(sFilename, sDrive, sDir, sFname, sExt);
	CAtlString filename(sFname);
	return filename;
}
CAtlString GetFullAppName()
{
	TCHAR sFilename[_MAX_PATH];
	TCHAR sDrive[_MAX_DRIVE];
	TCHAR sDir[_MAX_DIR];
	TCHAR sFname[_MAX_FNAME];
	TCHAR sExt[_MAX_EXT];
 	GetModuleFileName( NULL, sFilename, _MAX_PATH);
	return (CAtlString)sFilename;
}
/*-----------------------------------------------------------------------------------------------*/
bool MoveFileInernal(const CAtlString& a, const CAtlString& b)
{
	::SetFileAttributes(a, FILE_ATTRIBUTE_NORMAL);
	if (::MoveFileEx(a, b, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))	
	{
		if( a.CompareNoCase(b) )
			::DeleteFile(a);
		return true;
	}
	DWORD dwCode = ::GetLastError();
	return false;
}
bool CopyFileInernal(const CAtlString& a, const CAtlString& b)
{
	::SetFileAttributes(a, FILE_ATTRIBUTE_NORMAL);
	CheckDir(b,true,NULL);
	if (::CopyFile(a, b,FALSE)) {
		return true;
	}
	DWORD dwCode = ::GetLastError();
	return false;
}
bool	CopyDir(const CAtlString& dir, const CAtlString& dst,DirFilter *fun)
{
	bool bRet = true;
	WIN32_FIND_DATA	pData = { 0 };
	HANDLE hFind = INVALID_HANDLE_VALUE;
	CAtlString a;
	//文件移动使用move
	if (dir.IsEmpty())
		return false;
	a = AppendUrl(dir, _T("*.*"));
	hFind = ::FindFirstFile(a, &pData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	//确保目的肯定存在
	if (CheckDir(dst, true,NULL)) {
		return false;
	}
	do
	{
		CAtlString name = pData.cFileName;
		CAtlString srcF, tarF;
		if (name == '.' || name == _T("..")) {
			//跳过系统默认目录
			continue;
		}
		bool subRet = false;
		if ((pData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			srcF = AppendUrl(dir, name);
			tarF = AppendUrl(dst, name);
			subRet = CopyDir(srcF, tarF, fun);
		}
		else {
			srcF = AppendUrl(dir, name);
			tarF = AppendUrl(dst, name);
			subRet = CopyFileInernal(srcF, tarF);
			if (subRet&&fun)
				fun->DirFileProc(tarF);
		}
		if (bRet)
			bRet = subRet;
	} while (FindNextFile(hFind, &pData));
	::FindClose(hFind);
	//清空移动过的目录
	return bRet;
}
bool	CopyDirByFilter(const CAtlString& dir, const CAtlString& dst,DirFilter *fun)
{
	bool bRet = true;
	WIN32_FIND_DATA	pData = { 0 };
	HANDLE hFind = INVALID_HANDLE_VALUE;
	CAtlString a;
	//文件移动使用move
	if (dir.IsEmpty())
		return false;
	a = AppendUrl(dir, _T("*.*"));
	hFind = ::FindFirstFile(a, &pData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	//确保目的肯定存在
	if (CheckDir(dst, true,NULL)) 
	{
		return false;
	}
	do
	{
		CAtlString name = pData.cFileName;
		CAtlString srcF, tarF;
		if (name == '.' || name == _T("..")) {
			//跳过系统默认目录
			continue;
		}
		bool subRet = false;
		if ((pData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			srcF = AppendUrl(dir, name);
			tarF = AppendUrl(dst, name);
			subRet = CopyDir(srcF, tarF, fun);
		}
		else 
		{
			srcF = AppendUrl(dir, name);
			tarF = AppendUrl(dst, name);
			if (fun)
			{
				if (fun->DirFileProc(srcF))
				{
					subRet = CopyFileInernal(srcF, tarF);
				}
			}
			else
				subRet = CopyFileInernal(srcF, tarF);
		}
		if (bRet)
			bRet = subRet;
	} while (FindNextFile(hFind, &pData));
	::FindClose(hFind);
	//清空移动过的目录
	return bRet;
}

bool MoveDir(const CAtlString& dir, const CAtlString& dst)
{
	bool bRet = true;
	WIN32_FIND_DATA	pData = { 0 };
	HANDLE hFind = INVALID_HANDLE_VALUE;
	CAtlString a;
	//文件移动使用move
	if ( dir.IsEmpty() )
		return false;
	a = AppendUrl(dir, _T("*.*"));
	hFind = ::FindFirstFile(a, &pData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	//确保目的肯定存在
	if (CheckDir(dst, true,NULL)) {
		return false;
	}
	do
	{
		CAtlString name = pData.cFileName;
		CAtlString srcF, tarF;
		if (name == '.' || name == _T("..")) {
			//跳过系统默认目录
			continue;
		}
		bool subRet = false;
		if ((pData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			srcF = AppendUrl(dir, name);
			tarF = AppendUrl(dst, name);
			subRet = MoveDir(srcF, tarF);
		}
		else {
			srcF = AppendUrl(dir, name);
			tarF = AppendUrl(dst, name);
			subRet = MoveFileInernal(srcF, tarF);
		}
		if (bRet)
			bRet = subRet;
	} while (FindNextFile(hFind, &pData));
	::FindClose(hFind);
	//清空移动过的目录
	::RemoveDirectory(dir);
	return bRet;
}

DWORD	CountDirFiles(const CAtlString& dir)
{
	DWORD dwRet = 0;
	WIN32_FIND_DATA	pData = { 0 };
	HANDLE hFind = INVALID_HANDLE_VALUE;
	CAtlString a;
	//文件移动使用move
	if (dir.IsEmpty())
		return dwRet;
	a = AppendUrl(dir, _T("*.*"));
	hFind = ::FindFirstFile(a, &pData);
	if (hFind == INVALID_HANDLE_VALUE)
		return dwRet;
	do
	{
		CAtlString name = pData.cFileName;
		CAtlString srcF;
		if (name == '.' || name == _T("..")) {
			//跳过系统默认目录
			continue;
		}
		bool subRet = false;
		if ((pData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			srcF  = AppendUrl(dir, name);
			dwRet += CountDirFiles(srcF);
		}
		else {
			dwRet++;
		}
	} while (FindNextFile(hFind, &pData));
	::FindClose(hFind);
	//清空移动过的目录
	return dwRet;
}

bool RenameFile(const CAtlString& src, const CAtlString& dst)
{
	return MoveFileEx(src,dst,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH|MOVEFILE_COPY_ALLOWED)?true:false;
}
/*------------------------------------------------------------------------------------------*/
Shortcut::Shortcut()
{
	wHotKey = 0;
	showCmd = SW_SHOWNORMAL;
}
bool ResetShortcut(LPCWSTR lnkfile,Shortcut &outInfo)
{
	HRESULT hr = S_OK;
	IShellLink     *pLink=NULL;  //IShellLink对象指针
	IPersistFile   *ppf=NULL; //IPersisFil对象指针
	hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pLink);
	if(FAILED(hr))
		return false;
	//从IShellLink对象中获取IPersistFile接口
	hr = pLink->QueryInterface(IID_IPersistFile, (void**)&ppf);
	if (FAILED(hr))
	{
		pLink->Release();
		return false;
	}
	hr = ppf->Load(lnkfile, STGM_WRITE);
	if(!SUCCEEDED(hr))
		return false;
	if(outInfo.wHotKey)
		pLink->SetHotkey(outInfo.wHotKey);
	if(!outInfo.description.IsEmpty())
		pLink->SetDescription(outInfo.description);
	if(!outInfo.arguments.IsEmpty())
		pLink->SetArguments(outInfo.arguments);
	if(!outInfo.icon.IsEmpty())
		pLink->SetIconLocation(outInfo.icon, 0);
	if(!outInfo.workDir.IsEmpty())
		pLink->SetWorkingDirectory(outInfo.workDir);
	if(!outInfo.exefile.IsEmpty())
		pLink->SetPath(outInfo.exefile);
	pLink->SetShowCmd(outInfo.showCmd);
	ppf->Save(lnkfile,TRUE);
	ppf->Release();
	pLink->Release();
	return true;
}
bool GetShortcut(LPCWSTR lnkfile,Shortcut &outInfo)
{
	HRESULT hr = S_OK;
	IShellLink     *pLink=NULL;  //IShellLink对象指针
	IPersistFile   *ppf=NULL; //IPersisFil对象指针
	hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pLink);
	if(FAILED(hr))
		return false;
	//从IShellLink对象中获取IPersistFile接口
	hr = pLink->QueryInterface(IID_IPersistFile, (void**)&ppf);
	if (FAILED(hr))
	{
		pLink->Release();
		return false;
	}
	hr = ppf->Load(lnkfile, STGM_READ);
	if(!SUCCEEDED(hr))
		return false;
	TCHAR chTmp[20480];
	memset(chTmp,0,sizeof(chTmp));
	pLink->GetHotkey(&outInfo.wHotKey);
	if(SUCCEEDED(pLink->GetDescription(chTmp, 20480)))
		outInfo.description = chTmp;
	memset(chTmp,0,sizeof(chTmp));
	if(SUCCEEDED(pLink->GetArguments(chTmp, 20480)))
		outInfo.arguments = chTmp;
	memset(chTmp,0,sizeof(chTmp));
	int idx = 0;
	if(SUCCEEDED(pLink->GetIconLocation(chTmp, 20480, &idx)))
		outInfo.icon = chTmp;
	if(SUCCEEDED(pLink->GetWorkingDirectory(chTmp, 20480)))
		outInfo.workDir = chTmp;
	WIN32_FIND_DATA wfd;
	if(SUCCEEDED(pLink->GetPath(chTmp, 20480,&wfd, SLGP_SHORTPATH)))
		outInfo.exefile = chTmp;
	pLink->GetShowCmd(&outInfo.showCmd);
	ppf->Release();
	pLink->Release();
	return true;
}
bool CreateFileShortcut(LPCWSTR lpszSaveTo,const Shortcut &inInfo)
{
	HRESULT hr = S_OK;
	IShellLink     *pLink=NULL;  //IShellLink对象指针
	IPersistFile   *ppf=NULL; //IPersisFil对象指针
	hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pLink);
	if(FAILED(hr))
		return false;
	//从IShellLink对象中获取IPersistFile接口
	hr = pLink->QueryInterface(IID_IPersistFile, (void**)&ppf);
	if (FAILED(hr))
	{
		pLink->Release();
		return false;
	}
	if(inInfo.exefile.IsEmpty())
		pLink->SetPath(_wpgmptr);
	else
		pLink->SetPath(inInfo.exefile);
	//工作目录
	if (!inInfo.workDir.IsEmpty())
		pLink->SetWorkingDirectory(inInfo.workDir);
	if (!inInfo.icon.IsEmpty())
		pLink->SetIconLocation(inInfo.icon,0);
	//快捷键
	if (inInfo.wHotKey)
		pLink->SetHotkey(inInfo.wHotKey);
	//备注
	if (!inInfo.description.IsEmpty())
		pLink->SetDescription(inInfo.description);
	//显示方式
	pLink->SetShowCmd(inInfo.showCmd);
	//快捷方式的路径 + 名称
	CAtlString linkDir(lpszSaveTo);
	CAtlString saveto;
	if(linkDir.IsEmpty())
		linkDir = GetDesktopPath();
	wchar_t szBuffer[MAX_PATH];
	if (!inInfo.linkName.IsEmpty()) //指定了快捷方式的名称
 		saveto = AppendUrl(linkDir, inInfo.linkName);
 	else
	{
 		saveto = AppendUrl(linkDir, _wpgmptr);
		saveto.Replace(L".exe",L"");
	}
 	saveto += L".lnk";
	//保存快捷方式到指定目录下
	hr = ppf->Save(saveto, TRUE);
	ppf->Release();
	pLink->Release();
	return SUCCEEDED(hr);
}
/*---------------------------------------------------------------------------------------------*/
double GetDriveFreeGB(LPSTR drive)
{
	DWORDLONG i64FreeBytesToCaller = 0;
	DWORDLONG i64TotalBytes = 0;
	DWORDLONG i64FreeBytes = 0;
	GetDiskFreeSpaceExA(drive, (PULARGE_INTEGER)&i64FreeBytesToCaller,  
		(PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes);
	double dFreeGBs=0;
	dFreeGBs = i64FreeBytes*9.3132e-10;
	return dFreeGBs;
}
CAtlString GetMaxBetysDrive()
{
	int driveCount = 0;
	DWORD driveInfo = GetLogicalDrives();
	while (driveInfo) 
	{
		if (driveInfo & 1) {
			driveCount++;
		}
		driveInfo >>= 1;
	}
	int driveStrLen = GetLogicalDriveStrings(0, NULL);
	TCHAR *driveStr = new TCHAR[driveStrLen];
	GetLogicalDriveStrings(driveStrLen, driveStr);
	TCHAR *lpDriveStr = driveStr;
	double dMax = 0.00;
	CAtlString strCurDrive;
	for (int i = 0; i < driveCount; i++) 
	{
		UINT driveType = GetDriveType(lpDriveStr);
		DWORD dwAttrs = GetFileAttributes(lpDriveStr);
		if(driveType==DRIVE_FIXED && ((dwAttrs&FILE_ATTRIBUTE_READONLY)!=FILE_ATTRIBUTE_READONLY) )
		{
			CAtlString str ;
			double dTemp = 0.00;
			str = lpDriveStr;
			dTemp = GetDriveFreeGB((char*)CT2CA(str));
			if(dTemp>dMax)
			{
				dMax = dTemp;
				strCurDrive = str;
			}
		}		
		lpDriveStr += 4;		
	}
	delete []driveStr;
	return strCurDrive;
}