#include "stdafx.h"
#include "ZipArchive.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>


FILETIME timet2filetime(const __int32 t)
{
	LONGLONG i = Int32x32To64(t, 10000000) + 116444736000000000;
	FILETIME ft;
	ft.dwLowDateTime = (DWORD)i;
	ft.dwHighDateTime = (DWORD)(i >> 32);
	return ft;
}
FILETIME dosdatetime2filetime(WORD dosdate, WORD dostime)
{ // date: bits 0-4 are day of month 1-31. Bits 5-8 are month 1..12. Bits 9-15 are year-1980
  // time: bits 0-4 are seconds/2, bits 5-10 are minute 0..59. Bits 11-15 are hour 0..23
	SYSTEMTIME st;
	st.wYear = (WORD)(((dosdate >> 9) & 0x7f) + 1980);
	st.wMonth = (WORD)((dosdate >> 5) & 0xf);
	st.wDay = (WORD)(dosdate & 0x1f);
	st.wHour = (WORD)((dostime >> 11) & 0x1f);
	st.wMinute = (WORD)((dostime >> 5) & 0x3f);
	st.wSecond = (WORD)((dostime & 0x1f) * 2);
	st.wMilliseconds = 0;
	FILETIME ft; SystemTimeToFileTime(&st, &ft);
	return ft;
}
void EnsurezipDirectory(const TCHAR *rootdir, const TCHAR *dir)
{
	if (rootdir != 0 && GetFileAttributes(rootdir) == 0xFFFFFFFF) CreateDirectory(rootdir, 0);
	if (*dir == 0) return;
	const TCHAR *lastslash = dir, *c = lastslash;
	while (*c != 0) { if (*c == '/' || *c == '\\') lastslash = c; c++; }
	const TCHAR *name = lastslash;
	if (lastslash != dir)
	{
		TCHAR tmp[MAX_PATH]; memcpy(tmp, dir, sizeof(TCHAR)*(lastslash - dir));
		tmp[lastslash - dir] = 0;
		EnsurezipDirectory(rootdir, tmp);
		name++;
	}
	TCHAR cd[MAX_PATH]; *cd = 0; if (rootdir != 0) _tcscpy_s(cd, rootdir); _tcscat_s(cd, dir);
	if (GetFileAttributes(cd) == 0xFFFFFFFF) CreateDirectory(cd, NULL);
}

ZipExtract::ZipExtract() {
	m_pZipF	  = NULL;
	m_curfile = -1;
	m_total	  = 0;
	m_unzbuf = NULL;
}
ZipExtract::~ZipExtract() {
	Close();
}
DWORD	ZipExtract::GetTotal() {
	return this->m_total;
}
ZRESULT ZipExtract::Open(const CString& file) {
	if (this->strPass.IsEmpty())
		m_pZipF = unzOpen64(CT2CA(file));
	else
		m_pZipF = unzOpen64(CT2CA(file));
	if (!m_pZipF)
		return ZR_FAILED;
	unz_global_info gi;
	DWORD nRet = unzGetGlobalInfo(m_pZipF, &gi);
	if( nRet == UNZ_OK)	{
		this->m_total = gi.number_entry;
		return ZR_OK;
	}
	return ZR_FAILED;
}
void ZipExtract::SetRootDir(const CString& root) {
	if (root.IsEmpty())
		return;
	TCHAR chEnd = root[root.GetLength() - 1];
	if (chEnd != '\\' || chEnd != '/'){
		m_rootdir = root;
		m_rootdir += '\\';
	}
	else
		m_rootdir = root;
}
ZRESULT ZipExtract::Extract(const CString& root) {
	if (m_pZipF == NULL || m_total==0)
		return ZR_ARGS;
	m_curfile = 0;
	SetRootDir(root);
	for (m_curfile; m_curfile < m_total;m_curfile++) {
		ZipExtract::ZIPENTRY entry;
		GetItem(&entry);
		UnItem(entry);
		unzGoToNextFile(m_pZipF);
	}
	return ZR_OK;
}
ZRESULT ZipExtract::CustomExtract(const CString& root,const CString FirstZipPath,const CString strvalue)
{
	if (m_pZipF == NULL || m_total==0)
		return ZR_ARGS;
	bool bHave = false;
	m_curfile = 0;
	SetRootDir(root);
	for (m_curfile; m_curfile < m_total;m_curfile++)
	{
		ZipExtract::ZIPENTRY entry;
		GetItem(&entry);
		if(entry.name.Find(strvalue)!=-1)
		{

			UnItem(entry);
			bHave = true;
			break;
		}
		unzGoToNextFile(m_pZipF);
	}
	if(bHave)
		return ZR_OK;
	else
		return ZR_ARGS;
}

ZRESULT ZipExtract::GetItem(ZipExtract::ZIPENTRY *ze) {
	ZRESULT ret = ZR_OK;
	unz_file_info ufi; 
	CString tfn;
	if (!ze)
		return ZR_OK;
	char fn[MAX_PATH];
	unzGetCurrentFileInfo(m_pZipF, &ufi, fn, MAX_PATH, NULL, 0, NULL, 0);
	{
		const char *pfn = fn;
		for (;;)
		{
			if (pfn[0] != 0 && pfn[1] == ':') { pfn += 2; continue; }
			if (pfn[0] == '\\') { pfn++; continue; }
			if (pfn[0] == '/') { pfn++; continue; }
			const char *c;
			c = strstr(pfn,"\\..\\"); if (c != 0) { pfn = c + 4; continue; }
			c = strstr(pfn,"\\../"); if (c != 0) { pfn = c + 4; continue; }
			c = strstr(pfn,"/../"); if (c != 0) { pfn = c + 4; continue; }
			c = strstr(pfn,"/..\\"); if (c != 0) { pfn = c + 4; continue; }
			break;
		}
	}
	ze->name = CA2CT(fn);
	// zip has an 'attribute' 32bit value. Its lower half is windows stuff
	// its upper half is standard unix stat.st_mode. We'll start trying
	// to read it in unix mode
	unsigned long a = ufi.external_fa;
	bool isdir = (a & 0x40000000) != 0;
	bool readonly = (a & 0x00800000) == 0;
	bool hidden = false, system = false, archive = true;
	//解压长度为0的是目录
	if (ufi.uncompressed_size == 0)
		isdir = true;
	// but in normal hostmodes these are overridden by the lower half...
	int host = ufi.version >> 8;
	if (host == 0 || host == 7 || host == 11 || host == 14)
	{
		readonly = (a & 0x00000001) != 0;
		hidden = (a & 0x00000002) != 0;
		system = (a & 0x00000004) != 0;
		isdir = (a & 0x00000010) != 0;
		archive = (a & 0x00000020) != 0;
	}
	ze->attr = 0;
	if (isdir) ze->attr |= FILE_ATTRIBUTE_DIRECTORY;
	if (archive) ze->attr |= FILE_ATTRIBUTE_ARCHIVE;
	if (hidden) ze->attr |= FILE_ATTRIBUTE_HIDDEN;
	if (readonly) ze->attr |= FILE_ATTRIBUTE_READONLY;
	if (system) ze->attr |= FILE_ATTRIBUTE_SYSTEM;
	ze->comp_size = ufi.compressed_size;
	ze->unc_size = ufi.uncompressed_size;
	WORD dostime = (WORD)(ufi.dosDate & 0xFFFF);
	WORD dosdate = (WORD)((ufi.dosDate >> 16) & 0xFFFF);
	FILETIME ftd = dosdatetime2filetime(dosdate, dostime);
	FILETIME ft; LocalFileTimeToFileTime(&ftd, &ft);
	ze->atime = ft; ze->ctime = ft; ze->mtime = ft;
	return ret;
}
ZRESULT ZipExtract::UnItem(const ZIPENTRY &entry) {
	// zipentry=directory is handled specially
	if ((entry.attr&FILE_ATTRIBUTE_DIRECTORY) != 0) {
		TCHAR fch = entry.name[0];
		bool isabsolute = (fch == '/' || fch == '\\' || (fch != 0 && entry.name[1] == ':'));
		if (isabsolute)
			EnsurezipDirectory(NULL, entry.name);
		else 
			EnsurezipDirectory(m_rootdir, entry.name);
		return ZR_OK;
	}
	const TCHAR *ufn = (const TCHAR*)entry.name;
	// We'll qualify all relative names to our root dir, and leave absolute names as they are
	// ufn="zipfile.txt"  dir=""  name="zipfile.txt"  fn="c:\\currentdir\\zipfile.txt"
	// ufn="dir1/dir2/subfile.txt"  dir="dir1/dir2/"  name="subfile.txt"  fn="c:\\currentdir\\dir1/dir2/subfiles.txt"
	// ufn="\z\file.txt"  dir="\z\"  name="file.txt"  fn="\z\file.txt"
	// This might be a security risk, in the case where we just use the zipentry's name as "ufn", where
	// a malicious zip could unzip itself into c:\windows. Our solution is that GetZipItem (which
	// is how the user retrieve's the file's name within the zip) never returns absolute paths.
	const TCHAR *name = ufn; const TCHAR *c = name;
	while (*c != 0) 	{ 
		if (*c == '/' || *c == '\\') name = c + 1; c++; 
	}
	TCHAR dir[MAX_PATH]; 
	_tcscpy_s(dir, ufn); 
	if (name == ufn) 
		*dir = 0; 
	else 
		dir[name - ufn] = 0;
	TCHAR fn[MAX_PATH];
	bool isabsolute = (dir[0] == '/' || dir[0] == '\\' || (dir[0] != 0 && dir[1] == ':'));
	if (isabsolute) { 
		wsprintf(fn, _T("%s%s"), dir, name); EnsurezipDirectory(0, dir);
	}
	else { 
		wsprintf(fn, _T("%s%s%s"), m_rootdir, dir, name); EnsurezipDirectory(m_rootdir, dir); 
	}
	//
	HANDLE h = CreateFile(fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, entry.attr, NULL);
	if (h == INVALID_HANDLE_VALUE) return ZR_NOFILE;
	if (strPass.IsEmpty())
		unzOpenCurrentFile(m_pZipF);
	else
		unzOpenCurrentFilePassword(m_pZipF, CT2CA(strPass));
	if (m_unzbuf == 0) 
		m_unzbuf = new char[16384];
	ZRESULT haderr = ZR_OK;
	for (; haderr == 0;)
	{
		bool reached_eof=false;
		int res = unzReadCurrentFile(m_pZipF, m_unzbuf, 16384);
		if ( res == 0 )
			reached_eof = true;
		else if (res == UNZ_PARAMERROR) 		{
			haderr = ZR_PASSWORD; break;
		}
		else if (res<0) { 
			haderr = ZR_FLATE; 
			break;
		}
		if (res>0) { 
			DWORD writ; 
			BOOL bres = WriteFile(h, m_unzbuf, res, &writ, NULL);
			if (!bres) {
				haderr = ZR_WRITE; 
				break; 
			}
		}
		if (reached_eof)
			break;
		if (res == 0) {
			haderr = ZR_FLATE; 
			break;
		}
	}
	if (!haderr) SetFileTime(h, &entry.ctime, &entry.atime, &entry.mtime); // may fail if it was a pipe
	CloseHandle(h);
	unzCloseCurrentFile(m_pZipF);
	if (haderr != 0) 
		return haderr;
	return ZR_OK;
}
ZRESULT ZipExtract::Close() {
	if (!m_pZipF)
		return ZR_ARGS;
	unzClose(m_pZipF);
	m_pZipF = NULL;
	m_curfile = -1;
	m_total = 0;
	if (m_unzbuf) {
		delete[] m_unzbuf;
		m_unzbuf = NULL;
	}
	return ZR_OK;
}

CString ZipExtract::getItemName()
{
	CString strName;
	ZipExtract::ZIPENTRY entry;
	ZRESULT zRes = GetItem(&entry);
	if(zRes == ZR_OK)
		strName = entry.name;
	UnItem(entry);
	unzGoToNextFile(m_pZipF);
	return strName;
}
/************************************************************************/
/*                            压缩                                      */
/************************************************************************/
ZipCompress::ZipCompress()
{
	m_zipfile = NULL;
}

ZipCompress::~ZipCompress()
{
	//zipClose(m_zipfile,NULL);
}

ZRESULT ZipCompress::Compress( const CString& zipName )
{
	
	m_zipfile = zipOpen((char*)CT2CA(zipName),APPEND_STATUS_CREATE);
	if(m_zipfile==NULL)
		return ZR_NOFILE;
	AddDataToZip(m_zipfile,m_FileNameofZip.c_str(),m_inStr.c_str());
	zipClose(m_zipfile,NULL);
	return ZR_OK; 
}

void ZipCompress::SetZipFileNameAndInput( const std::string name,const std::string inStr)
{
	m_FileNameofZip = name;
	m_inStr =inStr;
}

void ZipCompress::CollectFilesInDirToZip( zipFile zf, const CString& strPath, const CString& parentDir )
{
	USES_CONVERSION; //for W2CA  

	//CString strRelativePath;  
	//CFileFind finder;   
	//BOOL bWorking = finder.FindFile(strPath + _T("//*.*"));  
	//while(bWorking)   
	//{   
	//	bWorking = finder.FindNextFile();   
	//	if(finder.IsDots())  
	//		continue;   

	//	if (parentDir == _T(""))  
	//		strRelativePath = finder.GetFileName();  
	//	else  
	//		strRelativePath = parentDir + _T("//") + finder.GetFileName(); //生成在zip文件中的相对路径  

	//	if(finder.IsDirectory())  
	//	{  
	//		AddFileToZip(zf, W2CA(strRelativePath), NULL); //在zip文件中生成目录结构  
	//		CollectFilesInDirToZip(zf, finder.GetFilePath(), strRelativePath); //递归收集子目录文件  
	//		continue;  
	//	}  

	//	AddFileToZip(zf, W2CA(strRelativePath), W2CA(finder.GetFilePath())); //将文件添加到zip文件中  
	//} 
}
void ZipCompress::AddFileToZip(zipFile zf, const char* fileNameInZip, const char* srcFile)  
{  
	FILE* srcfp = NULL;  

	//初始化写入zip的文件信息  
	zip_fileinfo zi;  
	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =  
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;  
	zi.dosDate = 0;  
	zi.internal_fa = 0;  
	zi.external_fa = 0;  

	//如果srcFile为空，加入空目录  
	char new_file_name[MAX_PATH];  
	memset(new_file_name, 0, sizeof(new_file_name));  
	
	strcat(new_file_name, fileNameInZip);  
	
	if (srcFile == NULL)  
	{  
		strcat(new_file_name, "/");  
	}  

	//在zip文件中创建新文件  
	zipOpenNewFileInZip(zf, new_file_name, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);  

	if (srcFile != NULL)  
	{  
		//打开源文件  
		srcfp = fopen(srcFile, "rb");  
		if (srcfp == NULL)  
		{  
			zipCloseFileInZip(zf); //关闭zip文件  
			return;  
		}  

		//读入源文件并写入zip文件  
		char buf[100*1024]; //buffer  
		int numBytes = 0;  
		while( !feof(srcfp) )  
		{  
			numBytes = fread(buf, 1, sizeof(buf), srcfp);  
			zipWriteInFileInZip(zf, buf, numBytes);  
			if( ferror(srcfp) )  
				break;  
		}  

		//关闭源文件  
		fclose(srcfp);  
	}  

	//关闭zip文件  
	zipCloseFileInZip(zf);  
}

void ZipCompress::AddDataToZip( zipFile zf, const char* fileNameInZip,/*zip内文件名 */ std::string inputStream )
{
	//初始化写入zip的文件信息  
	zip_fileinfo zi;  
	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =  
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;  
	zi.dosDate = 0;  
	zi.internal_fa = 0;  
	zi.external_fa = 0;  

	//如果srcFile为空，加入空目录  
	char new_file_name[MAX_PATH];  
	memset(new_file_name, 0, sizeof(new_file_name));  

	strcat(new_file_name, fileNameInZip);  

	//在zip文件中创建新文件  
	zipOpenNewFileInZip(zf, new_file_name, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);  
	

	if(!inputStream.empty())
	{
		char *buf = (char*)inputStream.data(); //buffer  
		
		int numBytes = strlen(buf);
		zipWriteInFileInZip(zf, buf, numBytes);  

	}	
	//关闭zip文件  
	zipCloseFileInZip(zf);  

}

void ZipCompress::ReleasezipFile()
{
	zipClose(m_zipfile,NULL);
}