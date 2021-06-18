// ***************************************************************
//  模块名称： sqlite封装  
//  版本号： 1.0
//  创建日期： 2012-07-11
//  创建者：hwz
//  -------------------------------------------------------------
//  功能描述： 对sqlite接口进行封装
//  -------------------------------------------------------------
//  版权信息：Copyright (C) 2012 - All Rights Reserved
// ***************************************************************
//  修改日期 2013-04-18 版本号  v1.0.016
//  修改描述 去掉抛出异常  modify by hwz
// ***************************************************************

#include "stdafx.h"
#include "CppSQLite3U.h"
#include "SQLiteTool.h"
#include <string>
#include <atlconv.h>


CppSQLite3ExceptionU::CppSQLite3ExceptionU(CSQLiteTool *sqlDB,
										   const int nErrCode,LPTSTR szErrMess,
										   bool bDeleteMsg/*=true*/) 
		:mnErrCode(nErrCode)
{
	int ss = szErrMess ? _tcslen(szErrMess)+50 : 50;
	mpszErrMess=new TCHAR[ss];
	_stprintf_s(mpszErrMess, ss, _T("%s[%d]: %s"),
								errorCodeAsString(nErrCode),
								nErrCode,
								szErrMess ? szErrMess : _T(""));

	if (bDeleteMsg && szErrMess)
	{
		sqlDB->SqliteFree(szErrMess);
	}
}


CppSQLite3ExceptionU::CppSQLite3ExceptionU(const CppSQLite3ExceptionU&  e) :
									mnErrCode(e.mnErrCode)
{
	mpszErrMess = 0;
	if (e.mpszErrMess)
	{
		int ss = _tcslen(e.mpszErrMess)+10;
		mpszErrMess=new TCHAR[ss];
		_stprintf_s(mpszErrMess, ss, _T("%s"), e.mpszErrMess);
	}
}


LPCTSTR CppSQLite3ExceptionU::errorCodeAsString(int nErrCode)
{
	switch (nErrCode)
	{
		case SQLITE_OK          : return _T("SQLITE_OK");
		case SQLITE_ERROR       : return _T("SQLITE_ERROR");
		case SQLITE_INTERNAL    : return _T("SQLITE_INTERNAL");
		case SQLITE_PERM        : return _T("SQLITE_PERM");
		case SQLITE_ABORT       : return _T("SQLITE_ABORT");
		case SQLITE_BUSY        : return _T("SQLITE_BUSY");
		case SQLITE_LOCKED      : return _T("SQLITE_LOCKED");
		case SQLITE_NOMEM       : return _T("SQLITE_NOMEM");
		case SQLITE_READONLY    : return _T("SQLITE_READONLY");
		case SQLITE_INTERRUPT   : return _T("SQLITE_INTERRUPT");
		case SQLITE_IOERR       : return _T("SQLITE_IOERR");
		case SQLITE_CORRUPT     : return _T("SQLITE_CORRUPT");
		case SQLITE_NOTFOUND    : return _T("SQLITE_NOTFOUND");
		case SQLITE_FULL        : return _T("SQLITE_FULL");
		case SQLITE_CANTOPEN    : return _T("SQLITE_CANTOPEN");
		case SQLITE_PROTOCOL    : return _T("SQLITE_PROTOCOL");
		case SQLITE_EMPTY       : return _T("SQLITE_EMPTY");
		case SQLITE_SCHEMA      : return _T("SQLITE_SCHEMA");
		case SQLITE_TOOBIG      : return _T("SQLITE_TOOBIG");
		case SQLITE_CONSTRAINT  : return _T("SQLITE_CONSTRAINT");
		case SQLITE_MISMATCH    : return _T("SQLITE_MISMATCH");
		case SQLITE_MISUSE      : return _T("SQLITE_MISUSE");
		case SQLITE_NOLFS       : return _T("SQLITE_NOLFS");
		case SQLITE_AUTH        : return _T("SQLITE_AUTH");
		case SQLITE_FORMAT      : return _T("SQLITE_FORMAT");
		case SQLITE_RANGE       : return _T("SQLITE_RANGE");
		case SQLITE_ROW         : return _T("SQLITE_ROW");
		case SQLITE_DONE        : return _T("SQLITE_DONE");
		case CPPSQLITE_ERROR    : return _T("CPPSQLITE_ERROR");
		default: return _T("UNKNOWN_ERROR");
	}
}


CppSQLite3ExceptionU::~CppSQLite3ExceptionU()
{
	if (mpszErrMess)
	{
 		delete [] mpszErrMess;
		mpszErrMess = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CppSQLite3DBU

CppSQLite3DBU::CppSQLite3DBU(CSQLiteTool *sqlTool)
{
	mpDB = 0;
	mnBusyTimeoutMs = 60000; // 60 seconds
	mpVM = NULL;
	autoShutDown = true;
	mynewtool = false;
	if(sqlTool==NULL)
	{
		mynewtool = true;
		mSqlTool = new CSQLiteTool;
	}
	else
		mSqlTool = sqlTool;
}

CppSQLite3DBU::CppSQLite3DBU(const CppSQLite3DBU& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
	mpVM=NULL;
	autoShutDown = false;
	mSqlTool = new CSQLiteTool;
	mynewtool = true;
	mSqlTool->Initialize( const_cast<CppSQLite3DBU&>(db).GetSqlTool() );
}
void CppSQLite3DBU::setAutoShutDown(bool autoShutDown)
{
	this->autoShutDown = autoShutDown;
}
CppSQLite3DBU::~CppSQLite3DBU()
{
	close();
	if(autoShutDown && mSqlTool)
		mSqlTool->Shutdown();
	if(mSqlTool && mynewtool)
		delete mSqlTool;
}

////////////////////////////////////////////////////////////////////////////////

CppSQLite3DBU& CppSQLite3DBU::operator=(const CppSQLite3DBU& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
	return *this;
}

bool CppSQLite3DBU::open(LPCTSTR szFile)
{
	if(mSqlTool==NULL)
		return false;
    int nRet = mSqlTool->OpenDB(CT2CA(szFile,CP_UTF8), &mpDB); 
	if (nRet != SQLITE_OK)
	{
		GetLastMsg();
		return false;
	}
	InitParam();
	return true;
}
bool CppSQLite3DBU::openEx( LPCTSTR pszMemory )
{
	int nRet;
	if(mSqlTool==NULL)
		return false;
	nRet = mSqlTool->OpenDB(CT2A(pszMemory,CP_UTF8), &mpDB); 
	if (nRet != SQLITE_OK)
	{
		GetLastMsg();
		return false;
	}
	InitParam();
	return true;
}
void CppSQLite3DBU::InitParam()
{
	mSqlTool->SetBusyTimeout(mpDB,mnBusyTimeoutMs);
	//CSQLiteTool::Get()->ExecuteSQL(mpDB, "PRAGMA journal_mode = WAL;", NULL, 0, 0); 
	mSqlTool->ExecuteSQL(mpDB, "PRAGMA encoding=UTF-8;", NULL, 0, 0); 
	mSqlTool->ExecuteSQL(mpDB, "synchronous = FULL;", NULL, 0, 0); 	
}
bool CppSQLite3DBU::backup(const CppSQLite3DBU &from,LPCTSTR dbNM)
{
	return mSqlTool->Backup(from.mpDB,CT2CA(dbNM),mpDB,CT2CA(dbNM));
}
void CppSQLite3DBU::GetLastMsg()
{
	LPCTSTR szError = (LPCTSTR) mSqlTool->GetErrMsg16(mpDB);
	lastMsg = szError;
	//CSQLiteTool::Get()->SqliteFree(szError);
}
bool CppSQLite3DBU::SetKeyDB(LPCTSTR szKey)
{
	if(!mpDB)
	{
		return false;
	}
	std::string pwd = (char*)CT2CA(szKey,CP_UTF8);
	int nRet = mSqlTool->SetKey(mpDB,pwd.c_str(),pwd.length());
	if(nRet != SQLITE_OK)
	{
		GetLastMsg();
		close();
		return false;
	}
	return true;
}

bool CppSQLite3DBU::EncryptDB(LPCTSTR szDBFile,LPCTSTR szKey)
{
	int nRet = mSqlTool->OpenDB(CT2CA(szDBFile,CP_UTF8),&mpDB);
	if (nRet != SQLITE_OK)
	{
		GetLastMsg();
		return false;	
	}
	else
	{
		std::string pwd = (char*)CT2CA(szKey,CP_UTF8);
		nRet = mSqlTool->SetKey(mpDB,pwd.c_str(),pwd.length());
		if(nRet != SQLITE_OK)
		{
			GetLastMsg();
			close();
			return false;
		}
	}
	InitParam();
	return true;
}
bool CppSQLite3DBU::ResetKeyDB(LPCTSTR szKey)
{
	int nRet = 0;
	std::string key = (char*)CT2CA(szKey,CP_UTF8);
	if( nRet != mSqlTool->ResetKey(mpDB,key.c_str(),key.length()) )
	{
		GetLastMsg();
		close();
		return false;
	}
	return true;
}

void CppSQLite3DBU::close()
{
	if (mpDB)
	{
		finalize();
		
		int nRet = mSqlTool->CloseDB(mpDB);
		if (nRet != SQLITE_OK)
		{
			GetLastMsg();
		}
		mpDB = 0;
	}
}

CppSQLite3StatementU CppSQLite3DBU::compileStatement(LPCTSTR szSQL)
{
	if (!mpDB)
	{
		return CppSQLite3StatementU(this);
	}	

	sqlite3_stmt* pVM = compile(szSQL);

	return CppSQLite3StatementU(this,pVM);
}


bool CppSQLite3DBU::tableExists(LPCTSTR szTable)
{
	TCHAR szSQL[128];

	_stprintf_s(szSQL, _T("select count(*) from sqlite_master where type='table' and name='%s'"),	szTable);

	int nRet = execScalar(szSQL);

	return (nRet > 0);
}

bool CppSQLite3DBU::finalize()
{
	if (mpVM)
	{
		int nRet = mSqlTool->Finalize(mpVM);
		mpVM = 0;
		if (nRet != SQLITE_OK)
		{
			GetLastMsg();
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

int CppSQLite3DBU::execDML(LPCTSTR szSQL)
{
	int nRet;
	sqlite3_stmt* pVM; 

	if (!mpDB)
	{
		return SQLITE_ERROR;
	}

	CString strSQL=szSQL;

	if(strSQL.IsEmpty())
	{
		return SQLITE_ERROR;
	}
	lastMsg.Empty();
	do{ 
		pVM = compile(szSQL);
		if(pVM==NULL)
		{
			nRet=SQLITE_ERROR;
			break;
		}
		nRet = mSqlTool->DBStep(pVM);
		if (nRet == SQLITE_ERROR)
		{
			GetLastMsg();
			//记录日志
			if(NULL!=pVM)
			{
				mSqlTool->Finalize(pVM);
				pVM=NULL;
			}			
			//close();
			nRet=SQLITE_ERROR;
			break;
		}
		nRet = mSqlTool->Finalize(pVM);
	} 
	while( nRet == SQLITE_SCHEMA );
	if(nRet != SQLITE_OK)
		GetLastMsg();
	return nRet;
}

CppSQLite3QueryU CppSQLite3DBU::execQuery(LPCTSTR szSQL)
{
	if (!mpDB)
	{
		return CppSQLite3QueryU(this);
	}
	lastMsg.Empty();
	int nRet;
	sqlite3_stmt* pVM; 
	do{ 
		pVM = compile(szSQL);
		if(pVM==NULL)
		{
			nRet=SQLITE_ERROR;
			break;
		}
		nRet = mSqlTool->DBStep(pVM);
		if (nRet == SQLITE_DONE)
		{	
			return CppSQLite3QueryU(this, pVM, true/*eof*/);
		}
		else if (nRet == SQLITE_ROW)
		{	// at least 1 row
			return CppSQLite3QueryU(this, pVM, false/*eof*/);
		}
		else if(nRet==SQLITE_ERROR){
			GetLastMsg();
		}
		nRet = mSqlTool->Finalize(pVM);
		Sleep(1);
	} 
	while( nRet == SQLITE_SCHEMA ); 
	GetLastMsg();
	return CppSQLite3QueryU(this);
}


int CppSQLite3DBU::execScalar(LPCTSTR szSQL)
{
	CppSQLite3QueryU q = execQuery(szSQL);

	//记录日志

	if (q.eof() || q.numFields() < 1)
	{
		//记录日志

		return -1;
	}		

	return _tstoi(q.fieldValue(0));
}

CString CppSQLite3DBU::execScalarStr(LPCTSTR szSQL)
{
	CppSQLite3QueryU q = execQuery(szSQL);

	//记录日志

	if (q.eof() || q.numFields() < 1)
	{
		//记录日志

		return _T("");
	}	
	
	return (CString)q.getStringField(0);
}

sqlite_int64 CppSQLite3DBU::lastRowId()
{
	return mSqlTool->GetLastInsertRowID(mpDB);
}


void CppSQLite3DBU::setBusyTimeout(int nMillisecs)
{
	mnBusyTimeoutMs = nMillisecs;
	mSqlTool->SetBusyTimeout(mpDB, mnBusyTimeoutMs);
}

sqlite3_stmt* CppSQLite3DBU::compile(LPCTSTR szSQL)
{	
	if (!mpDB)
	{
		return NULL;
	}
	lastMsg.Empty();
	sqlite3_stmt* pVM;
	int nRet = mSqlTool->Prepare16_V2(mpDB,szSQL,-1,&pVM,NULL);
	if (nRet != SQLITE_OK)
	{		
		GetLastMsg();
		//记录日志
		if(NULL!=pVM)
		{
			mSqlTool->Finalize(pVM);
		}
		//close();
		pVM=NULL;
	}
	return pVM;
}

int CppSQLite3DBU::execSys(LPCTSTR szSQL,CAtlString & stRet )
{
	int nRet=0;

	std::string  strtemp;
#ifdef _UNICODE
	strtemp = (char*)CT2CA(szSQL,CP_UTF8);
#else
	strtemp=szSQL;
#endif
	char *msg = NULL;
	nRet = mSqlTool->ExecuteSQL(mpDB, strtemp.c_str(), NULL, 0, &msg); 
    stRet = msg;
	if( msg )
	{
		mSqlTool->SqliteFree(msg);
	}
	if(nRet!=SQLITE_OK)
	{
		lastMsg = stRet;
	}
	return nRet;
}

//////////////////////// CppSQLite3StatementU  ///////////////////////////////////////////
CppSQLite3StatementU::CppSQLite3StatementU(CppSQLite3DBU *sqlDB)
{
	mpDB = 0;
	mpVM = 0;
	mSqliteDB = sqlDB;
}

CppSQLite3StatementU::CppSQLite3StatementU(const CppSQLite3StatementU& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	mSqliteDB = rStatement.mSqliteDB;
	// Only one object can own VM
	const_cast<CppSQLite3StatementU&>(rStatement).mpVM = 0;
}

CppSQLite3StatementU::CppSQLite3StatementU(CppSQLite3DBU *sqlDB,sqlite3_stmt* pVM)
{
	mpDB = sqlDB->mpDB;
	mpVM = pVM;
}

CppSQLite3StatementU::~CppSQLite3StatementU()
{
	try
	{
		finalize();
	}
	catch (...)	{}
}

CppSQLite3StatementU& CppSQLite3StatementU::operator=(const CppSQLite3StatementU& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	mSqliteDB = rStatement.mSqliteDB;
	const_cast<CppSQLite3StatementU&>(rStatement).mpVM = 0;
	return *this;
}

int CppSQLite3StatementU::execDML()
{
	if (!mpDB || mpVM == 0)
	{
		return SQLITE_ERROR;
	}

	int nRet = mSqliteDB->GetSqlTool()->DBStep(mpVM);

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = mSqliteDB->GetSqlTool()->SqliteChanges(mpDB);

		nRet = mSqliteDB->GetSqlTool()->Reset(mpVM);
		if (nRet != SQLITE_OK)
		{
			LPCTSTR szError = (LPCTSTR) mSqliteDB->GetSqlTool()->GetErrMsg16(mpDB);
			lastMsg = szError;
			mSqliteDB->GetSqlTool()->SqliteFree(szError);
			return SQLITE_ERROR;
		}
		return nRowsChanged;
	}
	else
	{
		nRet = mSqliteDB->GetSqlTool()->Reset(mpVM);

		LPCTSTR szError = (LPCTSTR) mSqliteDB->GetSqlTool()->GetErrMsg16(mpDB);
		lastMsg = szError;
		mSqliteDB->GetSqlTool()->SqliteFree(szError);

		return SQLITE_ERROR;
	}

	return SQLITE_ERROR;
}


bool CppSQLite3StatementU::bind(int nParam, LPCTSTR szValue)
{
	if(mpVM==NULL)
	{
		return false;
	}

	int nRes = mSqliteDB->GetSqlTool()->BindText16(mpVM, nParam, szValue, -1, SQLITE_TRANSIENT);

	if (nRes != SQLITE_OK)
		return false;

	return true;
}


bool CppSQLite3StatementU::bind(int nParam, const int nValue)
{
	if(mpVM==NULL)
	{
		return false;
	}

	int nRes = mSqliteDB->GetSqlTool()->BindInt(mpVM, nParam, nValue);
	if (nRes != SQLITE_OK)
		return false;

	return true;
}


bool CppSQLite3StatementU::bind(int nParam, const double dValue)
{
	if(mpVM==NULL)
	{
		return false;
	}
	int nRes = mSqliteDB->GetSqlTool()->BindDouble(mpVM, nParam, dValue);
	if (nRes != SQLITE_OK)
		return false;

	return true;
}


bool CppSQLite3StatementU::bind(int nParam, const unsigned char* blobValue, int nLen)
{
	if(mpVM==NULL)
	{
		return false;
	}
	int nRes = mSqliteDB->GetSqlTool()->BindBlob(mpVM, nParam,(const void*)blobValue, nLen, SQLITE_TRANSIENT);
	if (nRes != SQLITE_OK)
	{
		LPCTSTR szError = (LPCTSTR) mSqliteDB->GetSqlTool()->GetErrMsg16(mpDB);
		lastMsg = szError;
		mSqliteDB->GetSqlTool()->SqliteFree(szError);

		return false;
	}
	return true;
}

bool CppSQLite3StatementU::bindNull(int nParam)
{
	if(mpVM==NULL)
	{
		return false;
	}
	int nRes = mSqliteDB->GetSqlTool()->BindNull(mpVM, nParam);
	if (nRes != SQLITE_OK)
	{
		LPCTSTR szError = (LPCTSTR) mSqliteDB->GetSqlTool()->GetErrMsg16(mpDB);
		lastMsg = szError;
		mSqliteDB->GetSqlTool()->SqliteFree(szError);

  		return false;
	}
	return true;
}


bool CppSQLite3StatementU::reset()
{
	if (mpVM)
	{
		int nRet = mSqliteDB->GetSqlTool()->Reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			LPCTSTR szError = (LPCTSTR) mSqliteDB->GetSqlTool()->GetErrMsg16(mpDB);
			lastMsg = szError;
			mSqliteDB->GetSqlTool()->SqliteFree(szError);

			return false;
		}
	}

	return true;
}

bool CppSQLite3StatementU::finalize()
{
	if (mpVM)
	{
		int nRet = mSqliteDB->GetSqlTool()->Finalize(mpVM);
		mpVM = 0;
		if (nRet != SQLITE_OK)
		{
			LPCTSTR szError = (LPCTSTR)mSqliteDB->GetSqlTool()->GetErrMsg16(mpDB);
			lastMsg = szError;
			mSqliteDB->GetSqlTool()->SqliteFree(szError);
			return false;
		}
	}

	return true;
}

bool CppSQLite3StatementU::checkDB()
{
	if (mpDB == 0) 
		return false;
	return true;
}

bool CppSQLite3StatementU::checkVM()
{
	if (mpVM == 0)
		return false;
	return true;
}

/////////////////////  CppSQLite3QueryU  //////////////////////////////////////////////////
CppSQLite3QueryU::CppSQLite3QueryU(CppSQLite3DBU *sqlDB)
{
	mSqlDB = sqlDB;
	mpVM = 0;
	mbEof = true;
	mnCols = 0;
	mbOwnVM = false;
}


CppSQLite3QueryU::CppSQLite3QueryU(const CppSQLite3QueryU& rQuery)
{
	mpVM = rQuery.mpVM;
	const_cast<CppSQLite3QueryU&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
	mSqlDB = rQuery.mSqlDB;
}


CppSQLite3QueryU::CppSQLite3QueryU(CppSQLite3DBU *sqlDB,sqlite3_stmt* pVM,
								 bool bEof,	 bool bOwnVM/*=true*/)
{
	mSqlDB=sqlDB;
	mpDB = sqlDB->mpDB;
	mpVM = pVM;
	mbEof = bEof;
	mnCols = mSqlDB->GetSqlTool()->GetColumnCount(mpVM);
	mbOwnVM = bOwnVM;
}

CppSQLite3QueryU::~CppSQLite3QueryU()
{
	try
	{
		finalize();
	}
	catch (...) {}
}


CppSQLite3QueryU& CppSQLite3QueryU::operator=(const CppSQLite3QueryU& rQuery)
{
	try
	{
		finalize();
	}
	catch (...)	{ }
	mSqlDB = rQuery.mSqlDB;
	mpVM = rQuery.mpVM;
	const_cast<CppSQLite3QueryU&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
	return *this;
}


int CppSQLite3QueryU::numFields()
{
	if(mpVM==NULL)
	{
		return 0;
	}

	return mnCols;
}


LPCTSTR CppSQLite3QueryU::fieldValue(int nField)
{
	if(mpVM==NULL)
	{
		return _T("");
	}

	if (nField < 0 || nField > mnCols-1)
	{
		//记录日志

		return _T("");
	}		

	return (LPCTSTR)mSqlDB->GetSqlTool()->GetColumnUnicodeText(mpVM, nField);
}


LPCTSTR CppSQLite3QueryU::fieldValue(LPCTSTR szField)
{
	int nField = fieldIndex(szField);

	return (LPCTSTR)mSqlDB->GetSqlTool()->GetColumnUnicodeText(mpVM, nField);
}


int CppSQLite3QueryU::getIntField(int nField, int nNullValue/*=0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return mSqlDB->GetSqlTool()->GetColumnInt(mpVM, nField);
	}
}


int CppSQLite3QueryU::getIntField(LPCTSTR szField, int nNullValue/*=0*/)
{
	int nField = fieldIndex(szField);
	return getIntField(nField, nNullValue);
}

__int64 CppSQLite3QueryU::getInt64Field(LPCTSTR szField,__int64 nNullValue)
{
	int nField = fieldIndex(szField);
	return getInt64Field(nField, nNullValue);
}
__int64 CppSQLite3QueryU::getInt64Field(int nField,__int64 nNullValue)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return mSqlDB->GetSqlTool()->GetColumnInt64(mpVM, nField);
	}
}


double CppSQLite3QueryU::getFloatField(int nField, double fNullValue/*=0.0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return fNullValue;
	}
	else
	{
		return mSqlDB->GetSqlTool()->GetColumnDouble(mpVM, nField);
	}
}


double CppSQLite3QueryU::getFloatField(LPCTSTR szField, double fNullValue/*=0.0*/)
{
	int nField = fieldIndex(szField);

	return getFloatField(nField, fNullValue);
}


LPCTSTR CppSQLite3QueryU::getStringField(int nField, LPCTSTR szNullValue/*=""*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return szNullValue;
	}
	else
	{
		return (LPCTSTR) mSqlDB->GetSqlTool()->GetColumnUnicodeText(mpVM, nField);
	}
}

LPCTSTR CppSQLite3QueryU::getStringField(LPCTSTR szField, LPCTSTR szNullValue/*=""*/)
{
	int nField = fieldIndex(szField);
	return getStringField(nField, szNullValue);
}

LPCSTR CppSQLite3QueryU::getAsciiFieldIdx(int nField)
{
	return (LPCSTR)mSqlDB->GetSqlTool()->GetColumnText(mpVM, nField);
}
LPCSTR CppSQLite3QueryU::getAsciiStrField(LPCTSTR szField, LPCSTR szNullValue)
{
	int nField = fieldIndex(szField);
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return szNullValue;
	}
	return (LPCSTR)mSqlDB->GetSqlTool()->GetColumnText(mpVM, nField);
}


const unsigned char* CppSQLite3QueryU::getBlobField(int nField, int& nLen)
{
	if(mpVM==NULL)
	{
		return NULL;
	}

	if (nField < 0 || nField > mnCols-1)
	{
		return NULL;
	}		

	nLen = mSqlDB->GetSqlTool()->GetColumnByteLen16(mpVM, nField);
	return (const unsigned char*)mSqlDB->GetSqlTool()->GetColumnBlob(mpVM, nField);
}


const unsigned char* CppSQLite3QueryU::getBlobField(LPCTSTR szField, int& nLen)
{
	int nField = fieldIndex(szField);
	return getBlobField(nField, nLen);
}


bool CppSQLite3QueryU::fieldIsNull(int nField)
{
	return (fieldDataType(nField) == SQLITE_NULL);
}


bool CppSQLite3QueryU::fieldIsNull(LPCTSTR szField)
{
	int nField = fieldIndex(szField);
	return (fieldDataType(nField) == SQLITE_NULL);
}
int CppSQLite3QueryU::fieldIndex(LPCTSTR szField)
{
	if(mpVM==NULL)
	{
		return -1;
	}
	if (szField)
	{
		for (int nField = 0; nField < mnCols; nField++)
		{
			LPCTSTR szTemp = (LPCTSTR)mSqlDB->GetSqlTool()->GetColumnUnicodeName(mpVM, nField);

			if (_tcscmp(szField, szTemp) == 0)
			{
				return nField;
			}
		}
	}
	//记录日志
	return -1;
}


LPCTSTR CppSQLite3QueryU::fieldName(int nCol)
{
	if(mpVM==NULL)
	{
		return _T("");
	}
	if (nCol < 0 || nCol > mnCols-1)
	{
		//记录日志
		
		return _T("");
	}
	return (LPCTSTR)mSqlDB->GetSqlTool()->GetColumnUnicodeName(mpVM, nCol);
}


LPCTSTR CppSQLite3QueryU::fieldDeclType(int nCol)
{
	if(mpVM==NULL)
	{
		return _T("");
	}
	if (nCol < 0 || nCol > mnCols-1)
	{
		//记录日志
		
		return _T("");
	}
	return (LPCTSTR) mSqlDB->GetSqlTool()->GetColumnDeclType16(mpVM, nCol);
}


int CppSQLite3QueryU::fieldDataType(int nCol)
{
	if(mpVM==NULL)
	{
		return -1;
	}
	if (nCol < 0 || nCol > mnCols-1)
	{
		//记录日志
		
		return -1;
	}
	return mSqlDB->GetSqlTool()->GetColumnType(mpVM, nCol);
}


bool CppSQLite3QueryU::eof()
{
	if(mpVM==NULL)
	{
		return true;
	}

	return mbEof;
}


void CppSQLite3QueryU::nextRow()
{
	if(mpVM==NULL)
	{
		mbEof = true;
		return;
	}

	int nRet = mSqlDB->GetSqlTool()->DBStep(mpVM);

	if (nRet == SQLITE_DONE)
	{
		mbEof = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		// more rows, nothing to do
	}
	else
	{
		nRet = mSqlDB->GetSqlTool()->Finalize(mpVM);
		mpVM = 0;
		LPCTSTR szError = (LPCTSTR)mSqlDB->GetSqlTool()->GetErrMsg16(mpDB);
		lastMsg = szError;
		mSqlDB->GetSqlTool()->SqliteFree(szError);

		mbEof = true;
	}
}


bool CppSQLite3QueryU::finalize()
{
	if (mpVM && mbOwnVM)
	{
		int nRet = mSqlDB->GetSqlTool()->Finalize(mpVM);
		mpVM = 0;
		if (nRet != SQLITE_OK)
		{
			LPCTSTR szError = (LPCTSTR)mSqlDB->GetSqlTool()->GetErrMsg16(mpDB);
			lastMsg = szError;
			mSqlDB->GetSqlTool()->SqliteFree(szError);

			return false;
		}
	}

	return true;
}

bool CppSQLite3QueryU::checkVM()
{
	if (mpVM == 0)
	{
		//记录日志
		
		return false;
	}

	return true;
}

CString DoubleQuotes(CString in)
{
	in.Replace(_T("\'"),_T("\'\'"));

	return in;
}
