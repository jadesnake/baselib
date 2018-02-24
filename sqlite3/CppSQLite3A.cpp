
#include "StdAfx.h"
#include "sqlite3.h"
#include "CppSQLite3A.h"
#include <cstdlib>
#include "SQLiteTool.h"

// Named constant for passing to CppSQLite3ExceptionA when passing it a string
// that cannot be deleted.
static const bool DONT_DELETE_MSG=false;

#define LOG_SQL_EXEC_TIME (500*1000) //微秒

#define  SQLITE_OTHER_FAIL -1

////////////////////////////////////////////////////////////////////////////////
// Prototypes for SQLite functions not included in SQLite DLL, but copied below
// from SQLite encode.c
////////////////////////////////////////////////////////////////////////////////
int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out);
int sqlite3_decode_binary(const unsigned char *in, unsigned char *out);

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

/*
** Convert microsoft unicode to UTF-8.  Space to hold the returned string is
** obtained from malloc().
*/
static char *unicodeToUtf8(const WCHAR *zWideFilename)
{
	int nByte;
	char *zFilename=NULL;

	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, 0, 0, 0, 0);
	zFilename = (char*)malloc( nByte*sizeof(char));
	if( zFilename==0 ){
		return 0;
	}
	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, zFilename, nByte,
		0, 0);
	if( nByte == 0 ){
		free(zFilename);
		zFilename = 0;
	}
	return zFilename;
}


/*
** Convert an ansi string to microsoft unicode, based on the
** current codepage settings for file apis.
** 
** Space to hold the returned string is obtained
** from malloc.
*/
static WCHAR *mbcsToUnicode(const char *zFilename)
{
	int nByte;
	WCHAR *zMbcsFilename;
	int codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;

	nByte = MultiByteToWideChar(codepage, 0, zFilename, -1, NULL,0);
	zMbcsFilename = (WCHAR*)malloc( nByte*sizeof(WCHAR) );
	if( zMbcsFilename==0 ){
		return 0;
	}
	nByte = MultiByteToWideChar(codepage, 0, zFilename, -1, zMbcsFilename, nByte);
	if( nByte==0 ){
		free(zMbcsFilename);
		zMbcsFilename = 0;
	}
	return zMbcsFilename;
}

/*
** Convert multibyte character string to UTF-8.  Space to hold the
** returned string is obtained from malloc().
*/
static char *mbcsToUtf8(const char *zFilename){
	char *zFilenameUtf8;
	WCHAR *zTmpWide;

	zTmpWide = mbcsToUnicode(zFilename);
	if( zTmpWide==0 ){
		return 0;
	}
	zFilenameUtf8 = unicodeToUtf8(zTmpWide);
	free(zTmpWide);
	return zFilenameUtf8;
}

CppSQLite3ExceptionA::CppSQLite3ExceptionA(const int nErrCode,
									char* szErrMess,
									bool bDeleteMsg/*=true*/) :
									mnErrCode(nErrCode)
{
	mpszErrMess = CSQLiteTool::Get()->MPrintf("%s[%d]: %s",
		errorCodeAsString(nErrCode),
		nErrCode,
		szErrMess ? szErrMess : "");

	if (bDeleteMsg && szErrMess)
	{
		CSQLiteTool::Get()->SqliteFree(szErrMess);
	}
}

									
CppSQLite3ExceptionA::CppSQLite3ExceptionA(const CppSQLite3ExceptionA&  e) :
									mnErrCode(e.mnErrCode)
{
	mpszErrMess = 0;
	if (e.mpszErrMess)
	{
		mpszErrMess = CSQLiteTool::Get()->MPrintf("%s", e.mpszErrMess);
	}
}


const char* CppSQLite3ExceptionA::errorCodeAsString(int nErrCode)
{
	switch (nErrCode)
	{
		case SQLITE_OK          : return "SQLITE_OK";
		case SQLITE_ERROR       : return "SQLITE_ERROR";
		case SQLITE_INTERNAL    : return "SQLITE_INTERNAL";
		case SQLITE_PERM        : return "SQLITE_PERM";
		case SQLITE_ABORT       : return "SQLITE_ABORT";
		case SQLITE_BUSY        : return "SQLITE_BUSY";
		case SQLITE_LOCKED      : return "SQLITE_LOCKED";
		case SQLITE_NOMEM       : return "SQLITE_NOMEM";
		case SQLITE_READONLY    : return "SQLITE_READONLY";
		case SQLITE_INTERRUPT   : return "SQLITE_INTERRUPT";
		case SQLITE_IOERR       : return "SQLITE_IOERR";
		case SQLITE_CORRUPT     : return "SQLITE_CORRUPT";
		case SQLITE_NOTFOUND    : return "SQLITE_NOTFOUND";
		case SQLITE_FULL        : return "SQLITE_FULL";
		case SQLITE_CANTOPEN    : return "SQLITE_CANTOPEN";
		case SQLITE_PROTOCOL    : return "SQLITE_PROTOCOL";
		case SQLITE_EMPTY       : return "SQLITE_EMPTY";
		case SQLITE_SCHEMA      : return "SQLITE_SCHEMA";
		case SQLITE_TOOBIG      : return "SQLITE_TOOBIG";
		case SQLITE_CONSTRAINT  : return "SQLITE_CONSTRAINT";
		case SQLITE_MISMATCH    : return "SQLITE_MISMATCH";
		case SQLITE_MISUSE      : return "SQLITE_MISUSE";
		case SQLITE_NOLFS       : return "SQLITE_NOLFS";
		case SQLITE_AUTH        : return "SQLITE_AUTH";
		case SQLITE_FORMAT      : return "SQLITE_FORMAT";
		case SQLITE_RANGE       : return "SQLITE_RANGE";
		case SQLITE_ROW         : return "SQLITE_ROW";
		case SQLITE_DONE        : return "SQLITE_DONE";
		case CPPSQLITE_ERROR    : return "CPPSQLITE_ERROR";
		default: return "UNKNOWN_ERROR";
	}
}


CppSQLite3ExceptionA::~CppSQLite3ExceptionA()
{
	if (mpszErrMess)
	{
		CSQLiteTool::Get()->SqliteFree(mpszErrMess);
		mpszErrMess = 0;
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3Buffer::CppSQLite3Buffer()
{
	mpBuf = 0;
}


CppSQLite3Buffer::~CppSQLite3Buffer()
{
	clear();
}


void CppSQLite3Buffer::clear()
{
	if (mpBuf)
	{
		CSQLiteTool::Get()->SqliteFree(mpBuf);
		mpBuf = 0;
	}

}


const char* CppSQLite3Buffer::format(const char* szFormat, ...)
{
	clear();
	va_list va;
	va_start(va, szFormat);
	mpBuf = CSQLiteTool::Get()->VMPrintf(szFormat, va);
	va_end(va);
	return mpBuf;
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3Binary::CppSQLite3Binary() :
						mpBuf(0),
						mnBinaryLen(0),
						mnBufferLen(0),
						mnEncodedLen(0),
						mbEncoded(false)
{
}


CppSQLite3Binary::~CppSQLite3Binary()
{
	clear();
}


void CppSQLite3Binary::setBinary(const unsigned char* pBuf, int nLen)
{
	mpBuf = allocBuffer(nLen);
	memcpy(mpBuf, pBuf, nLen);
}


void CppSQLite3Binary::setEncoded(const unsigned char* pBuf)
{
	clear();

	mnEncodedLen = strlen((const char*)pBuf);
	mnBufferLen = mnEncodedLen + 1; // Allow for NULL terminator

	mpBuf = (unsigned char*)malloc(mnBufferLen);

	if (!mpBuf)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Cannot allocate memory",
								DONT_DELETE_MSG);
	}

	memcpy(mpBuf, pBuf, mnBufferLen);
	mbEncoded = true;
}


const unsigned char* CppSQLite3Binary::getEncoded()
{
	if (!mbEncoded)
	{
		unsigned char* ptmp = (unsigned char*)malloc(mnBinaryLen);
		memcpy(ptmp, mpBuf, mnBinaryLen);
		mnEncodedLen = sqlite3_encode_binary(ptmp, mnBinaryLen, mpBuf);
		free(ptmp);
		mbEncoded = true;
	}

	return mpBuf;
}


const unsigned char* CppSQLite3Binary::getBinary()
{
	if (mbEncoded)
	{
		// in/out buffers can be the same
		mnBinaryLen = sqlite3_decode_binary(mpBuf, mpBuf);

		if (mnBinaryLen == -1)
		{
			throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
									"Cannot decode binary",
									DONT_DELETE_MSG);
		}

		mbEncoded = false;
	}

	return mpBuf;
}


int CppSQLite3Binary::getBinaryLength()
{
	getBinary();
	return mnBinaryLen;
}


unsigned char* CppSQLite3Binary::allocBuffer(int nLen)
{
	clear();

	// Allow extra space for encoded binary as per comments in
	// SQLite encode.c See bottom of this file for implementation
	// of SQLite functions use 3 instead of 2 just to be sure ;-)
	mnBinaryLen = nLen;
	mnBufferLen = 3 + (257*nLen)/254;

	mpBuf = (unsigned char*)malloc(mnBufferLen);

	if (!mpBuf)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Cannot allocate memory",
								DONT_DELETE_MSG);
	}

	mbEncoded = false;

	return mpBuf;
}


void CppSQLite3Binary::clear()
{
	if (mpBuf)
	{
		mnBinaryLen = 0;
		mnBufferLen = 0;
		free(mpBuf);
		mpBuf = 0;
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3QueryA::CppSQLite3QueryA()
{
	mpVM = 0;
	mbEof = true;
	mnCols = 0;
	mbOwnVM = false;
}


CppSQLite3QueryA::CppSQLite3QueryA(const CppSQLite3QueryA& rQuery)
{
	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<CppSQLite3QueryA&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
}


CppSQLite3QueryA::CppSQLite3QueryA(sqlite3* pDB,
							sqlite3_stmt* pVM,
							bool bEof,
							bool bOwnVM/*=true*/)
{
	mpDB = pDB;
	mpVM = pVM;
	mbEof = bEof;
	mnCols = CSQLiteTool::Get()->GetColumnCount(mpVM);
	mbOwnVM = bOwnVM;
}


CppSQLite3QueryA::~CppSQLite3QueryA()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CppSQLite3QueryA& CppSQLite3QueryA::operator=(const CppSQLite3QueryA& rQuery)
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
	mpVM = rQuery.mpVM;
	// Only one object can own the VM
	const_cast<CppSQLite3QueryA&>(rQuery).mpVM = 0;
	mbEof = rQuery.mbEof;
	mnCols = rQuery.mnCols;
	mbOwnVM = rQuery.mbOwnVM;
	return *this;
}


int CppSQLite3QueryA::numFields()
{
	checkVM();
	return mnCols;
}


const char* CppSQLite3QueryA::fieldValue(int nField)
{
	checkVM();

	if (nField < 0 || nField > mnCols-1)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return (const char*)CSQLiteTool::Get()->GetColumnText(mpVM, nField);
}


const char* CppSQLite3QueryA::fieldValue(const char* szField)
{
	int nField = fieldIndex(szField);
	return (const char*)CSQLiteTool::Get()->GetColumnText(mpVM, nField);
}


int CppSQLite3QueryA::getIntField(int nField, int nNullValue/*=0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return nNullValue;
	}
	else
	{
		return CSQLiteTool::Get()->GetColumnInt(mpVM, nField);
	}
}


int CppSQLite3QueryA::getIntField(const char* szField, int nNullValue/*=0*/)
{
	int nField = fieldIndex(szField);
	return getIntField(nField, nNullValue);
}


double CppSQLite3QueryA::getFloatField(int nField, double fNullValue/*=0.0*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return fNullValue;
	}
	else
	{
		return CSQLiteTool::Get()->GetColumnDouble(mpVM, nField);
	}
}


double CppSQLite3QueryA::getFloatField(const char* szField, double fNullValue/*=0.0*/)
{
	int nField = fieldIndex(szField);
	return getFloatField(nField, fNullValue);
}


const char* CppSQLite3QueryA::getStringField(int nField, const char* szNullValue/*=""*/)
{
	if (fieldDataType(nField) == SQLITE_NULL)
	{
		return szNullValue;
	}
	else
	{
		return (const char*)CSQLiteTool::Get()->GetColumnText(mpVM, nField);
	}
}


const char* CppSQLite3QueryA::getStringField(const char* szField, const char* szNullValue/*=""*/)
{
	int nField = fieldIndex(szField);
	return getStringField(nField, szNullValue);
}


const unsigned char* CppSQLite3QueryA::getBlobField(int nField, int& nLen)
{
	checkVM();

	if (nField < 0 || nField > mnCols-1)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	nLen = CSQLiteTool::Get()->GetColumnByteLen(mpVM, nField);
	return (const unsigned char*)CSQLiteTool::Get()->GetColumnBlob(mpVM, nField);
}


const unsigned char* CppSQLite3QueryA::getBlobField(const char* szField, int& nLen)
{
	int nField = fieldIndex(szField);
	return getBlobField(nField, nLen);
}


bool CppSQLite3QueryA::fieldIsNull(int nField)
{
	return (fieldDataType(nField) == SQLITE_NULL);
}


bool CppSQLite3QueryA::fieldIsNull(const char* szField)
{
	int nField = fieldIndex(szField);
	return (fieldDataType(nField) == SQLITE_NULL);
}


int CppSQLite3QueryA::fieldIndex(const char* szField)
{
	checkVM();

	if (szField)
	{
		for (int nField = 0; nField < mnCols; nField++)
		{
			const char* szTemp = CSQLiteTool::Get()->GetColumnName(mpVM, nField);

			if (strcmp(szField, szTemp) == 0)
			{
				return nField;
			}
		}
	}

	throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
							"Invalid field name requested",
							DONT_DELETE_MSG);
}


const char* CppSQLite3QueryA::fieldName(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return CSQLiteTool::Get()->GetColumnName(mpVM, nCol);
}


const char* CppSQLite3QueryA::fieldDeclType(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return CSQLiteTool::Get()->GetColumnDeclType(mpVM, nCol);
}


int CppSQLite3QueryA::fieldDataType(int nCol)
{
	checkVM();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return CSQLiteTool::Get()->GetColumnType(mpVM, nCol);
}


bool CppSQLite3QueryA::eof()
{
	checkVM();
	return mbEof;
}


void CppSQLite3QueryA::nextRow()
{
	checkVM();

	int nRet = CSQLiteTool::Get()->DBStep(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		mbEof = true;
	}
	else if (nRet == SQLITE_ROW)
	{
		// more rows, nothing to do
	}
	else
	{
		nRet = CSQLiteTool::Get()->Finalize(mpVM);
		mpVM = 0;
		const char* szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
		throw CppSQLite3ExceptionA(nRet,
								(char*)szError,
								DONT_DELETE_MSG);
	}
}


void CppSQLite3QueryA::finalize()
{
	if (mpVM && mbOwnVM)
	{
		int nRet = CSQLiteTool::Get()->Finalize(mpVM);
		mpVM = 0;
		if (nRet != SQLITE_OK)
		{
			const char* szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
			throw CppSQLite3ExceptionA(nRet, (char*)szError, DONT_DELETE_MSG);
		}
	}
}


void CppSQLite3QueryA::checkVM()
{
	if (mpVM == 0)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Null Virtual Machine pointer",
								DONT_DELETE_MSG);
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3Table::CppSQLite3Table()
{
	mpaszResults = 0;
	mnRows = 0;
	mnCols = 0;
	mnCurrentRow = 0;
}


CppSQLite3Table::CppSQLite3Table(const CppSQLite3Table& rTable)
{
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<CppSQLite3Table&>(rTable).mpaszResults = 0;
	mnRows = rTable.mnRows;
	mnCols = rTable.mnCols;
	mnCurrentRow = rTable.mnCurrentRow;
}


CppSQLite3Table::CppSQLite3Table(char** paszResults, int nRows, int nCols)
{
	mpaszResults = paszResults;
	mnRows = nRows;
	mnCols = nCols;
	mnCurrentRow = 0;
}


CppSQLite3Table::~CppSQLite3Table()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CppSQLite3Table& CppSQLite3Table::operator=(const CppSQLite3Table& rTable)
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
	mpaszResults = rTable.mpaszResults;
	// Only one object can own the results
	const_cast<CppSQLite3Table&>(rTable).mpaszResults = 0;
	mnRows = rTable.mnRows;
	mnCols = rTable.mnCols;
	mnCurrentRow = rTable.mnCurrentRow;
	return *this;
}


void CppSQLite3Table::finalize()
{
	if (mpaszResults)
	{
		CSQLiteTool::Get()->FreeTable(mpaszResults);
		mpaszResults = 0;
	}
}


int CppSQLite3Table::numFields()
{
	checkResults();
	return mnCols;
}


int CppSQLite3Table::numRows()
{
	checkResults();
	return mnRows;
}


const char* CppSQLite3Table::fieldValue(int nField)
{
	checkResults();

	if (nField < 0 || nField > mnCols-1)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	int nIndex = (mnCurrentRow*mnCols) + mnCols + nField;
	return mpaszResults[nIndex];
}


const char* CppSQLite3Table::fieldValue(const char* szField)
{
	checkResults();

	if (szField)
	{
		for (int nField = 0; nField < mnCols; nField++)
		{
			if (strcmp(szField, mpaszResults[nField]) == 0)
			{
				int nIndex = (mnCurrentRow*mnCols) + mnCols + nField;
				return mpaszResults[nIndex];
			}
		}
	}

	throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
							"Invalid field name requested",
							DONT_DELETE_MSG);
}


int CppSQLite3Table::getIntField(int nField, int nNullValue/*=0*/)
{
	if (fieldIsNull(nField))
	{
		return nNullValue;
	}
	else
	{
		return atoi(fieldValue(nField));
	}
}


int CppSQLite3Table::getIntField(const char* szField, int nNullValue/*=0*/)
{
	if (fieldIsNull(szField))
	{
		return nNullValue;
	}
	else
	{
		return atoi(fieldValue(szField));
	}
}


double CppSQLite3Table::getFloatField(int nField, double fNullValue/*=0.0*/)
{
	if (fieldIsNull(nField))
	{
		return fNullValue;
	}
	else
	{
		return atof(fieldValue(nField));
	}
}


double CppSQLite3Table::getFloatField(const char* szField, double fNullValue/*=0.0*/)
{
	if (fieldIsNull(szField))
	{
		return fNullValue;
	}
	else
	{
		return atof(fieldValue(szField));
	}
}


const char* CppSQLite3Table::getStringField(int nField, const char* szNullValue/*=""*/)
{
	if (fieldIsNull(nField))
	{
		return szNullValue;
	}
	else
	{
		return fieldValue(nField);
	}
}


const char* CppSQLite3Table::getStringField(const char* szField, const char* szNullValue/*=""*/)
{
	if (fieldIsNull(szField))
	{
		return szNullValue;
	}
	else
	{
		return fieldValue(szField);
	}
}


bool CppSQLite3Table::fieldIsNull(int nField)
{
	checkResults();
	return (fieldValue(nField) == 0);
}


bool CppSQLite3Table::fieldIsNull(const char* szField)
{
	checkResults();
	return (fieldValue(szField) == 0);
}


const char* CppSQLite3Table::fieldName(int nCol)
{
	checkResults();

	if (nCol < 0 || nCol > mnCols-1)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Invalid field index requested",
								DONT_DELETE_MSG);
	}

	return mpaszResults[nCol];
}


void CppSQLite3Table::setRow(int nRow)
{
	checkResults();

	if (nRow < 0 || nRow > mnRows-1)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Invalid row index requested",
								DONT_DELETE_MSG);
	}

	mnCurrentRow = nRow;
}


void CppSQLite3Table::checkResults()
{
	if (mpaszResults == 0)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Null Results pointer",
								DONT_DELETE_MSG);
	}
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3StatementA::CppSQLite3StatementA()
{
	mpDB = 0;
	mpVM = 0;
}


CppSQLite3StatementA::CppSQLite3StatementA(const CppSQLite3StatementA& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<CppSQLite3StatementA&>(rStatement).mpVM = 0;
}


CppSQLite3StatementA::CppSQLite3StatementA(sqlite3* pDB, sqlite3_stmt* pVM)
{
	mpDB = pDB;
	mpVM = pVM;
}


CppSQLite3StatementA::~CppSQLite3StatementA()
{
	try
	{
		finalize();
	}
	catch (...)
	{
	}
}


CppSQLite3StatementA& CppSQLite3StatementA::operator=(const CppSQLite3StatementA& rStatement)
{
	mpDB = rStatement.mpDB;
	mpVM = rStatement.mpVM;
	// Only one object can own VM
	const_cast<CppSQLite3StatementA&>(rStatement).mpVM = 0;
	return *this;
}


int CppSQLite3StatementA::execDML()
{
	if(!checkDB() || !checkVM())
	{
		return SQLITE_FAIL;
	}	

	const char* szError=0;

	int nRet = CSQLiteTool::Get()->DBStep(mpVM);

	while(nRet==SQLITE_ROW)
	{
		nRet = CSQLiteTool::Get()->DBStep(mpVM);	
	}

	CString strErrMsg;

	if (nRet == SQLITE_DONE)
	{
		int nRowsChanged = CSQLiteTool::Get()->SqliteChanges(mpDB);

		nRet = CSQLiteTool::Get()->Reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
			lastMsg = szError;
			CSQLiteTool::Get()->SqliteFree(szError);
			return SQLITE_ERROR;
		}

		return SQLITE_OK;
	}
	else
	{
		nRet = CSQLiteTool::Get()->Reset(mpVM);
		szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
		lastMsg = szError;
		CSQLiteTool::Get()->SqliteFree(szError);
		return SQLITE_ERROR;
	}

	return SQLITE_ERROR;	
}


CppSQLite3QueryA CppSQLite3StatementA::execQuery()
{
	if(!checkDB() || !checkVM())
	{
		return CppSQLite3QueryA();
	}	

	int nRet = CSQLiteTool::Get()->DBStep(mpVM);

	if (nRet == SQLITE_DONE)
	{
		// no rows
		return CppSQLite3QueryA(mpDB, mpVM, true/*eof*/, false);
	}
	else if (nRet == SQLITE_ROW)
	{
		// at least 1 row
		return CppSQLite3QueryA(mpDB, mpVM, false/*eof*/, false);
	}
	else
	{
		nRet = CSQLiteTool::Get()->Reset(mpVM);
		const char* szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
		lastMsg = szError;
		CSQLiteTool::Get()->SqliteFree(szError);
	}
	return CppSQLite3QueryA();
}


bool CppSQLite3StatementA::bind(int nParam, const char* szValue)
{
	checkVM();
	int nRes = CSQLiteTool::Get()->BindText(mpVM, nParam, szValue, -1,SQLITE_STATIC);

	if (nRes != SQLITE_OK)
	{
		return false;
	}

	return true;
}


bool CppSQLite3StatementA::bind(int nParam, const int nValue)
{
	checkVM();
	int nRes = CSQLiteTool::Get()->BindInt(mpVM, nParam, nValue);

	if (nRes != SQLITE_OK)
	{
		return false;
	}

	return true;
}


bool CppSQLite3StatementA::bind(int nParam, const double dValue)
{
	checkVM();

	int nRes = CSQLiteTool::Get()->BindDouble(mpVM, nParam, dValue);

	if (nRes != SQLITE_OK)
	{
		return false;
	}

	return true;
}


bool CppSQLite3StatementA::bind(int nParam, const unsigned char* blobValue, int nLen)
{
	checkVM();

	int nRes = CSQLiteTool::Get()->BindBlob(mpVM, nParam,
								(const void*)blobValue, nLen, SQLITE_STATIC);

	if (nRes != SQLITE_OK)
	{
		return false;
	}

	return true;
}

	
bool CppSQLite3StatementA::bindNull(int nParam)
{
	checkVM();
	int nRes = CSQLiteTool::Get()->BindNull(mpVM, nParam);

	if (nRes != SQLITE_OK)
	{
		return false;
	}

	return true;
}


void CppSQLite3StatementA::reset()
{
	if (mpVM)
	{
		int nRet = CSQLiteTool::Get()->Reset(mpVM);

		if (nRet != SQLITE_OK)
		{
			const char* szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
			lastMsg = szError;
			CSQLiteTool::Get()->SqliteFree(szError);
		}
	}
}


void CppSQLite3StatementA::finalize()
{
	if (mpVM)
	{
		int nRet = CSQLiteTool::Get()->Finalize(mpVM);
		mpVM = 0;

		if (nRet != SQLITE_OK)
		{
			const char* szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
			lastMsg = szError;
			CSQLiteTool::Get()->SqliteFree(szError);
		}
	}
}


bool CppSQLite3StatementA::checkDB()
{
	if (mpDB == 0)
	{
		return false;
	}

	return true;
}


bool CppSQLite3StatementA::checkVM()
{
	if (mpVM == 0)
	{
		return false;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////

CppSQLite3DBA::CppSQLite3DBA()
{
	mpDB = 0;
	mnBusyTimeoutMs = 60000; // 60 seconds
}


CppSQLite3DBA::CppSQLite3DBA(const CppSQLite3DBA& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
}


CppSQLite3DBA::~CppSQLite3DBA()
{
	close();
}


CppSQLite3DBA& CppSQLite3DBA::operator=(const CppSQLite3DBA& db)
{
	mpDB = db.mpDB;
	mnBusyTimeoutMs = 60000; // 60 seconds
	return *this;
}

//释放所有资源
void CppSQLite3DBA::Release()
{
	CSQLiteTool::Get()->Release();
	
	CSQLiteTool::Destroy();
}

bool CppSQLite3DBA::open(const char* szFile)
{
	char* szUtf8file = mbcsToUtf8(szFile);

	if(NULL==szUtf8file)
	{
		return false;
	}

	int nRet = CSQLiteTool::Get()->OpenDB(szUtf8file, &mpDB);

	free(szUtf8file);

	bool bSuccess=true;

	if (nRet != SQLITE_OK)
	{
		bSuccess=false;
		
		const char* szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
		lastMsg = szError;
		CSQLiteTool::Get()->SqliteFree(szError);
	}
	else
	{
		setBusyTimeout(mnBusyTimeoutMs);
	}	

	return bSuccess;
}

void CppSQLite3DBA::EncryptDB(const char* szDBFile,const char* szKey)
{
	char* file = mbcsToUtf8(szDBFile);

	int nRet = CSQLiteTool::Get()->OpenDB(file, &mpDB);
	free(file);

	if (nRet != SQLITE_OK)
	{
		const char* szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
		throw CppSQLite3ExceptionA(nRet, (char*)szError);
	}

	//nRet = sqlite3_rekey(mpDB,szKey,strlen(szKey));
	
	close();
}


void CppSQLite3DBA::close()
{
	if (mpDB)
	{
		CSQLiteTool::Get()->CloseDB(mpDB);
		mpDB = 0;
	}
}


CppSQLite3StatementA CppSQLite3DBA::compileStatement(const char* szSQL)
{
	checkDB();

	sqlite3_stmt* pVM = compile(szSQL);

	return CppSQLite3StatementA(mpDB, pVM);
}


bool CppSQLite3DBA::tableExists(const char* szTable)
{
	char szSQL[128];

	sprintf_s(szSQL,
			"select count(*) from sqlite_master where type='table' and name='%s'",szTable);

	int nRet = execScalar(szSQL);

	return (nRet > 0);
}


int CppSQLite3DBA::execDML(const char* szSQL)
{
	if(!checkDB())
	{
		return SQLITE_FAIL;
	}

	char* szUTF8SQL = mbcsToUtf8(szSQL);

	if(NULL==szUTF8SQL)
	{
		return SQLITE_OTHER_FAIL;
	}

	//记录日志
	char* szError=0;

	int nRet = CSQLiteTool::Get()->ExecuteSQL(mpDB, szUTF8SQL, 0, 0, &szError);

	free(szUTF8SQL);

	if (nRet != SQLITE_OK)
	{
		if( NULL!=szError )
		{
			CString strErrMsg=CA2T(szError);
			
			CSQLiteTool::Get()->SqliteFree(szError);
		}		
	}
	return nRet;
}


CppSQLite3QueryA CppSQLite3DBA::execQuery(const char* szSQL)
{
	if(!checkDB())
	{
		return CppSQLite3QueryA();
	}

	int nRet;
	sqlite3_stmt* pVM; 

	char* szUTF8SQL = mbcsToUtf8(szSQL);

	if(NULL==szUTF8SQL)
	{
		return CppSQLite3QueryA();
	}


	do{ 
		pVM = compile(szUTF8SQL);

		if(pVM==NULL)
		{
			nRet=SQLITE_ERROR;

			break;
		}

		nRet = CSQLiteTool::Get()->DBStep(pVM);

		if (nRet == SQLITE_DONE)
		{	
			free(szUTF8SQL);
			return CppSQLite3QueryA(mpDB, pVM, true);
		}
		else if (nRet == SQLITE_ROW)
		{	
			free(szUTF8SQL);
			return CppSQLite3QueryA(mpDB, pVM, false);
		}
		else if(nRet==SQLITE_ERROR){
			LPCTSTR szError = (LPCTSTR) CSQLiteTool::Get()->GetErrMsg16(mpDB);
			lastMsg = szError;
			CSQLiteTool::Get()->SqliteFree(szError);
		}
		nRet = CSQLiteTool::Get()->Finalize(pVM);

		Sleep(1);
	} 
	while( nRet == SQLITE_SCHEMA ); 
	free(szUTF8SQL);
	const char* szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
	CString strErrMsg=CA2T(szError);
	return CppSQLite3QueryA();	
}


int CppSQLite3DBA::execScalar(const char* szSQL)
{
	CppSQLite3QueryA q = execQuery(szSQL);

	if (q.eof() || q.numFields() < 1)
	{
		throw CppSQLite3ExceptionA(CPPSQLITE_ERROR,
								"Invalid scalar query",
								DONT_DELETE_MSG);
	}

	return atoi(q.fieldValue(0));
}


CppSQLite3Table CppSQLite3DBA::getTable(const char* szSQL)
{
	checkDB();

	char* szError=0;
	char** paszResults=0;
	int nRet;
	int nRows(0);
	int nCols(0);
	char* szUtf8SQL=NULL;

	szUtf8SQL=mbcsToUtf8(szSQL);

	nRet = CSQLiteTool::Get()->GetTable(mpDB, szUtf8SQL, &paszResults, &nRows, &nCols, &szError);

	free(szUtf8SQL);

	if (nRet == SQLITE_OK)
	{
		return CppSQLite3Table(paszResults, nRows, nCols);
	}
	else
	{
		throw CppSQLite3ExceptionA(nRet, szError);
	}
}


sqlite_int64 CppSQLite3DBA::lastRowId()
{
	return CSQLiteTool::Get()->GetLastInsertRowID(mpDB);
}


void CppSQLite3DBA::setBusyTimeout(int nMillisecs)
{
	mnBusyTimeoutMs = nMillisecs;
	CSQLiteTool::Get()->SetBusyTimeout(mpDB, mnBusyTimeoutMs);
}


bool CppSQLite3DBA::checkDB()
{
	if (!mpDB)
	{
		return false;
	}

	return true;
}


sqlite3_stmt* CppSQLite3DBA::compile(const char* szSQL)
{
	checkDB();

	const char* szError=0;
	const char* szTail=0;
	sqlite3_stmt* pVM;

	int nRet = CSQLiteTool::Get()->Prepare_V2(mpDB, szSQL, -1, &pVM, &szTail);

	if (nRet != SQLITE_OK)
	{		
		szError = CSQLiteTool::Get()->GetErrMsg(mpDB);
		lastMsg = szError;
		CSQLiteTool::Get()->SqliteFree(szError);
		if(NULL!=pVM)
		{
			CSQLiteTool::Get()->Finalize(pVM);
		}

		close();

		pVM=NULL;
	}	

	return pVM;
}

int CppSQLite3DBA::execSys( const char* szSQL )
{
    return CSQLiteTool::Get()->ExecuteSQL(mpDB,szSQL,NULL,0,0);
}

sqlite3* CppSQLite3DBA::GetSqliteDB()
{
	return mpDB;
}

int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out){
  int i, j, e, m;
  int cnt[256];
  if( n<=0 ){
    out[0] = 'x';
    out[1] = 0;
    return 1;
  }
  memset(cnt, 0, sizeof(cnt));
  for(i=n-1; i>=0; i--){ cnt[in[i]]++; }
  m = n;
  for(i=1; i<256; i++){
    int sum;
    if( i=='\'' ) continue;
    sum = cnt[i] + cnt[(i+1)&0xff] + cnt[(i+'\'')&0xff];
    if( sum<m ){
      m = sum;
      e = i;
      if( m==0 ) break;
    }
  }
  out[0] = e;
  j = 1;
  for(i=0; i<n; i++){
    int c = (in[i] - e)&0xff;
    if( c==0 ){
      out[j++] = 1;
      out[j++] = 1;
    }else if( c==1 ){
      out[j++] = 1;
      out[j++] = 2;
    }else if( c=='\'' ){
      out[j++] = 1;
      out[j++] = 3;
    }else{
      out[j++] = c;
    }
  }
  out[j] = 0;
  return j;
}

int sqlite3_decode_binary(const unsigned char *in, unsigned char *out){
  int i, c, e;
  e = *(in++);
  i = 0;
  while( (c = *(in++))!=0 ){
    if( c==1 ){
      c = *(in++);
      if( c==1 ){
        c = 0;
      }else if( c==2 ){
        c = 1;
      }else if( c==3 ){
        c = '\'';
      }else{
        return -1;
      }
    }
    out[i++] = (c + e)&0xff;
  }
  return i;
}
