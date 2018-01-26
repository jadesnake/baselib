#pragma once

#include <atlbase.h>
#include <atlstr.h>

class DirFilter
{
public:
	virtual ~DirFilter(){	}
public:
	virtual DWORD DirEnumItemProc(LPCTSTR lpItemName,LPVOID lpParam,PWIN32_FIND_DATA pInfo)=0;
	virtual bool  DirFileProc(const CAtlString&) = 0;
};

bool  GetFilePath(LPCTSTR pV,LPTSTR pOut,size_t outNum);

bool  OpenSelPathDialog(LPTSTR outSel,HWND hOwner=NULL,LPCTSTR pDefaultDir=NULL,LPCTSTR pTitle=_T("选择文件路径"));

/*---------------------------------------------------------------------*\
		函数功能：枚举指定目录并执行回调动作
		函数名称：EnumFloderItem
		参	  数：lpRoot目录参数；
				  dwAttributes枚举的子项属性；
				  lpParam自定义参数；
				  lpItemProc回调接口
		返 回 值：0成功，否则错误代码
\*---------------------------------------------------------------------*/
DWORD  EnumFloderItem(LPCTSTR lpRoot,DWORD dwAttributes,LPVOID lpParam,DirFilter *filter);

/*---------------------------------------------------------------------*\
		函数功能：创建N级pDir目录
		函数名称：CheckDir
		参	  数：bCreate TRUE——创建目录，FALSE——检测目录是否存在；
				  lpSecurity目录安全属性，可以为NULL
		返 回 值：0创建成功，否则错误代码,-1 bCreate==FALSE时目录不存在，其他GetLastError返回值
\*---------------------------------------------------------------------*/
DWORD  CheckDir( LPCTSTR pDir,BOOL bCreate,LPSECURITY_ATTRIBUTES lpSecurity);

/*---------------------------------------------------------------------*\
		函数功能：获取指定文件的大小
		函数名称：GetFileSize
		参	  数：strPath——文件完整路径
		返 回 值：0 —— 表示失败
\*---------------------------------------------------------------------*/
LONGLONG QueryFileSize( LPCTSTR strPath );

/*---------------------------------------------------------------------*\
		函数功能：返回目录下的所有文件的全路径
		函数名称：GetFileListFromDir
		参	  数：path 文件夹路径
		返 回 值：
		例子1: CSimpleArray<CAtlString> files = GetFileListFromDir(_T("C:\\WINDOWS\\system\\"));
		例子2: CSimpleArray<CAtlString> files = GetFileListFromDir(_T("C:\\WINDOWS\\system"));
\*---------------------------------------------------------------------*/
CSimpleArray<CAtlString> GetFileListFromDir(CAtlString path, BOOL recursive=FALSE);

//获取程序所在目录
CAtlString GetAppPath();
CAtlString getAppFilePath();
//获取动态连接库(dll）所在路径
//pszFileName:dll文件名(不包含路径)
CAtlString GetDllPath(LPCTSTR pszFileName);

//判断文件或者目录是否存在
BOOL IsPathFind(const CAtlString& strPath);

//获取local appdata 路径
CAtlString GetLocalAppDataPath();

//获取进程完整名称
CString GetProcessFullName(HANDLE h);

//删除文件夹
bool DeleteDir(const CAtlString &dir);

//追加路径
CAtlString AppendUrl(const CAtlString& a, const CAtlString& b);

//从完整路径中获取文件名
CAtlString PathGetFileName(const CAtlString& f);

//从完整路径中获取路径
CAtlString FindFilePath(const CAtlString& path);

//获取当前应用名称
CAtlString GetAppName();

bool CopyDirByFilter(const CString& dir, const CString& dst,DirFilter *fun);

bool CopyDir(const CAtlString& dir, const CAtlString& dst,DirFilter *fun);

bool MoveDir(const CAtlString& dir, const CAtlString& dst);

DWORD CountDirFiles(const CAtlString& dir);
