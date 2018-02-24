
#if !defined(AFX_CPPSQLITE3U_H__1B1BE273_2D1E_439C_946F_3CBD1C0EFD2F__INCLUDED_)
#define AFX_CPPSQLITE3U_H__1B1BE273_2D1E_439C_946F_3CBD1C0EFD2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CppSQLite3U.h : header file
//
#include "sqlite3.h"
/////////////////////////////////////////////////////////////////////////////
#define SQL_MAXSIZE    2048

#define CPPSQLITE_ERROR 1000
static const bool DONT_DELETE_MSG=false;


CString DoubleQuotes(CString in);

class CppSQLite3QueryU;
class CppSQLite3StatementU;

class CppSQLite3ExceptionU
{
public:

    CppSQLite3ExceptionU(const int nErrCode,
					    LPTSTR  szErrMess,
					    bool bDeleteMsg=true);

    CppSQLite3ExceptionU(const CppSQLite3ExceptionU&  e);

    virtual ~CppSQLite3ExceptionU();

    const int errorCode() { return mnErrCode; }

    LPCTSTR errorMessage() { return mpszErrMess; }

    static LPCTSTR  errorCodeAsString(int nErrCode);

private:

    int mnErrCode;
    LPTSTR  mpszErrMess;
};


class CppSQLite3DBU 
{
// Construction
public:
	CppSQLite3DBU();

// Operations
public:

    virtual ~CppSQLite3DBU();

    bool open(LPCTSTR szFile);
	bool openEx(LPCTSTR pszMemory);

	bool EncryptDB(LPCTSTR szDBFile,LPCTSTR szKey);
	bool ResetKeyDB(LPCTSTR szKey);
	bool SetKeyDB(LPCTSTR szKey);

    void close();
    bool tableExists(LPCTSTR szTable);
    int execDML(LPCTSTR szSQL);

	int execSys(LPCTSTR szSQL,CAtlString & stRet);

    CppSQLite3QueryU execQuery(LPCTSTR szSQL);

    int execScalar(LPCTSTR szSQL);
	CString execScalarStr(LPCTSTR szSQL);

    CppSQLite3StatementU compileStatement(LPCTSTR szSQL);

    sqlite_int64 lastRowId();
    
    void setBusyTimeout(int nMillisecs);

    static const char* SQLiteVersion() { return SQLITE_VERSION; }

	bool finalize();

	//释放所有资源
	static void Release();
protected:
	void GetLastMsg();
	void InitParam();
private:

    CppSQLite3DBU(const CppSQLite3DBU& db);
    CppSQLite3DBU& operator=(const CppSQLite3DBU& db);

    sqlite3_stmt* compile(LPCTSTR szSQL);

    bool checkDB();
public:
    sqlite3* mpDB;
    int mnBusyTimeoutMs;
	sqlite3_stmt* mpVM;
	CAtlString lastMsg;
};
/////////////////////////////////////////////////////////////////////////////

class CppSQLite3StatementU
{
public:

	CppSQLite3StatementU();

	CppSQLite3StatementU(const CppSQLite3StatementU& rStatement);

	CppSQLite3StatementU(sqlite3* pDB, sqlite3_stmt* pVM);

	virtual ~CppSQLite3StatementU();

	CppSQLite3StatementU& operator=(const CppSQLite3StatementU& rStatement);

	int execDML();

	CppSQLite3QueryU execQuery();

	bool bind(int nParam, LPCTSTR szValue);
	bool bind(int nParam, const int nValue);
	bool bind(int nParam, const double dwValue);
	bool bind(int nParam, const unsigned char* blobValue, int nLen);
	bool bindNull(int nParam);

	bool reset();

	bool finalize();

private:

	bool checkDB();
	bool checkVM();

	sqlite3* mpDB;
	sqlite3_stmt* mpVM;
	CAtlString lastMsg;
};
/////////////////////  CppSQLite3QueryU  //////////////////////////////////////////////////
class CppSQLite3QueryU
{
public:

	CppSQLite3QueryU();

	CppSQLite3QueryU(const CppSQLite3QueryU& rQuery);

	CppSQLite3QueryU(sqlite3* pDB,
		sqlite3_stmt* pVM,
		bool bEof,
		bool bOwnVM=true);

	CppSQLite3QueryU& operator=(const CppSQLite3QueryU& rQuery);

	virtual ~CppSQLite3QueryU();

	int numFields();

	int fieldIndex(LPCTSTR szField);
	LPCTSTR fieldName(int nCol);

	LPCTSTR fieldDeclType(int nCol);
	int fieldDataType(int nCol);

	LPCTSTR fieldValue(int nField);
	LPCTSTR fieldValue(LPCTSTR szField);

	int getIntField(int nField, int nNullValue=0);
	int getIntField(LPCTSTR szField, int nNullValue=0);

	__int64 getInt64Field(LPCTSTR szField,__int64 nNullValue=0);
	__int64 getInt64Field(int nField,__int64 nNullValue=0);

	double getFloatField(int nField, double fNullValue=0.0);
	double getFloatField(LPCTSTR szField, double fNullValue=0.0);

	LPCTSTR getStringField(int nField, LPCTSTR szNullValue=_T(""));
	LPCTSTR getStringField(LPCTSTR szField, LPCTSTR szNullValue=_T(""));

	const unsigned char* getBlobField(int nField, int& nLen);
	const unsigned char* getBlobField(LPCTSTR szField, int& nLen);

	bool fieldIsNull(int nField);
	bool fieldIsNull(LPCTSTR szField);

	bool eof();
	void nextRow();
	bool finalize();

private:

	bool checkVM();

	sqlite3* mpDB;
	sqlite3_stmt* mpVM;
	bool mbEof;
	int mnCols;
	bool mbOwnVM;
	CAtlString lastMsg;
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CPPSQLITE3U_H__1B1BE273_2D1E_439C_946F_3CBD1C0EFD2F__INCLUDED_)
