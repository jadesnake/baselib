#pragma once
#include "zip.h"
#include "unzip.h"
enum ZRESULT : DWORD {
	ZR_OK = 0x00000000,     // nb. the pseudo-code zr-recent is never returned
	ZR_RECENT = 0x00000001,     // but can be passed to FormatZipMessage.
	ZR_GENMASK = 0x0000FF00,
	ZR_NODUPH = 0x00000100,     // couldn't duplicate the handle
	ZR_NOFILE = 0x00000200,     // couldn't create/open the file
	ZR_NOALLOC = 0x00000300,     // failed to allocate some resource
	ZR_WRITE = 0x00000400,     // a general error writing to the file
	ZR_NOTFOUND = 0x00000500,     // couldn't find that file in the zip
	ZR_MORE = 0x00000600,     // there's still more data to be unzipped
	ZR_CORRUPT = 0x00000700,     // the zipfile is corrupt or not a zipfile
	ZR_READ = 0x00000800,     // a general error reading the file
	ZR_PASSWORD = 0x00001000,     // we didn't get the right password to unzip the file
	ZR_CALLERMASK = 0x00FF0000,
	ZR_ARGS = 0x00010000,     // general mistake with the arguments
	ZR_NOTMMAP = 0x00020000,     // tried to ZipGetMemory, but that only works on mmap zipfiles, which yours wasn't
	ZR_MEMSIZE = 0x00030000,    // the memory size is too small
	ZR_FAILED = 0x00040000,     // the thing was already failed when you called this function
	ZR_ENDED = 0x00050000,    // the zip creation has already been closed
	ZR_MISSIZE = 0x00060000,     // the indicated input file size turned out mistaken
	ZR_PARTIALUNZ = 0x00070000,     // the file had already been partially unzipped
	ZR_ZMODE = 0x00080000,     // tried to mix creating/opening a zip 
	ZR_BUGMASK = 0xFF000000,
	ZR_NOTINITED = 0x01000000,     // initialisation didn't work
	ZR_SEEK = 0x02000000,    // trying to seek in an unseekable file
	ZR_NOCHANGE = 0x04000000,     // changed its mind on storage, but not allowed
	ZR_FLATE = 0x05000000    // an internal error in the de/inflation code
};

class ZipExtract {
public:
	typedef struct
	{
		int index;                  // index of this file within the zip
		CString name;				// filename within the zip
		DWORD attr;                 // attributes, as in GetFileAttributes.
		FILETIME atime, ctime, mtime;// access, create, modify filetimes
		long comp_size;             // sizes of item, compressed and uncompressed. These
		long unc_size;              // may be -1 if not yet known (e.g. being streamed in)
	} ZIPENTRY;	
	ZipExtract();
	virtual ~ZipExtract();
	ZRESULT Open(const CString& file);
	ZRESULT Extract(const CString& root);
	ZRESULT CustomExtract(const CString& root,const CString FirstZipPath,const CString strvalue);
	ZRESULT Close();
	DWORD	GetTotal();	//获取文件个数

	CString getItemName();
public:
	CString strPass;
protected:
	virtual void SetRootDir(const CString& root);
	virtual ZRESULT GetItem(ZIPENTRY *ze);
	virtual ZRESULT UnItem(const ZIPENTRY &entry);
private:	
	CString m_rootdir;
	int		m_curfile;
	void	*m_pZipF;
	DWORD	m_total;
	char	*m_unzbuf;
};
class ZipCompress
{
public:
	ZipCompress ();
	virtual ~ZipCompress();
	void ReleasezipFile();
	ZRESULT Compress(const CString& zipName);
	void SetZipFileNameAndInput(const std::string name,const std::string inStr);
	
	void AddFileToZip(zipFile zf, 
					  const char* fileNameInZip,//zip内文件名
					  const char* srcFile       // 需要写进去的文件流
					  );
	void AddDataToZip(
		zipFile zf, 
		const char* fileNameInZip,//zip内文件名
		std::string inputStream
		);
	void CollectFilesInDirToZip(zipFile zf, const CString& strPath, const CString& parentDir);
private:
	zipFile m_zipfile;
	std::string m_ZipFileName;
	std::string m_FileNameofZip;
    std::string m_inStr;
};