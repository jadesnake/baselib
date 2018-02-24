#pragma once

#ifndef _CppSQLite3A_H_
#define _CppSQLite3A_H_

#include "sqlite3.h"
#include <cstdio>
#include <cstring>

#define CPPSQLITE_ERROR 1000

class CppSQLite3ExceptionA
{
public:

    CppSQLite3ExceptionA(const int nErrCode,
                    char* szErrMess,
                    bool bDeleteMsg=true);

    CppSQLite3ExceptionA(const CppSQLite3ExceptionA&  e);

    virtual ~CppSQLite3ExceptionA();

    const int errorCode() { return mnErrCode; }

    const char* errorMessage() { return mpszErrMess; }

    static const char* errorCodeAsString(int nErrCode);

private:

    int mnErrCode;
    char* mpszErrMess;
};


class CppSQLite3Buffer
{
public:

    CppSQLite3Buffer();

    virtual ~CppSQLite3Buffer();

    const char* format(const char* szFormat, ...);

    operator const char*() { return mpBuf; }

    void clear();

private:

    char* mpBuf;
};


class CppSQLite3Binary
{
public:

    CppSQLite3Binary();

    virtual ~CppSQLite3Binary();

    void setBinary(const unsigned char* pBuf, int nLen);
    void setEncoded(const unsigned char* pBuf);

    const unsigned char* getEncoded();
    const unsigned char* getBinary();

    int getBinaryLength();

    unsigned char* allocBuffer(int nLen);

    void clear();

private:

    unsigned char* mpBuf;
    int mnBinaryLen;
    int mnBufferLen;
    int mnEncodedLen;
    bool mbEncoded;
};


class CppSQLite3QueryA
{
public:

    CppSQLite3QueryA();

    CppSQLite3QueryA(const CppSQLite3QueryA& rQuery);

    CppSQLite3QueryA(sqlite3* pDB,
				sqlite3_stmt* pVM,
                bool bEof,
                bool bOwnVM=true);

    CppSQLite3QueryA& operator=(const CppSQLite3QueryA& rQuery);

    virtual ~CppSQLite3QueryA();

    int numFields();

    int fieldIndex(const char* szField);
    const char* fieldName(int nCol);

    const char* fieldDeclType(int nCol);
    int fieldDataType(int nCol);

    const char* fieldValue(int nField);
    const char* fieldValue(const char* szField);

    int getIntField(int nField, int nNullValue=0);
    int getIntField(const char* szField, int nNullValue=0);

    double getFloatField(int nField, double fNullValue=0.0);
    double getFloatField(const char* szField, double fNullValue=0.0);

    const char* getStringField(int nField, const char* szNullValue="");
    const char* getStringField(const char* szField, const char* szNullValue="");

    const unsigned char* getBlobField(int nField, int& nLen);
    const unsigned char* getBlobField(const char* szField, int& nLen);

    bool fieldIsNull(int nField);
    bool fieldIsNull(const char* szField);

    bool eof();

    void nextRow();

    void finalize();

private:

    void checkVM();

	sqlite3* mpDB;
    sqlite3_stmt* mpVM;
    bool mbEof;
    int mnCols;
    bool mbOwnVM;
};


class CppSQLite3Table
{
public:

    CppSQLite3Table();

    CppSQLite3Table(const CppSQLite3Table& rTable);

    CppSQLite3Table(char** paszResults, int nRows, int nCols);

    virtual ~CppSQLite3Table();

    CppSQLite3Table& operator=(const CppSQLite3Table& rTable);

    int numFields();

    int numRows();

    const char* fieldName(int nCol);

    const char* fieldValue(int nField);
    const char* fieldValue(const char* szField);

    int getIntField(int nField, int nNullValue=0);
    int getIntField(const char* szField, int nNullValue=0);

    double getFloatField(int nField, double fNullValue=0.0);
    double getFloatField(const char* szField, double fNullValue=0.0);

    const char* getStringField(int nField, const char* szNullValue="");
    const char* getStringField(const char* szField, const char* szNullValue="");

    bool fieldIsNull(int nField);
    bool fieldIsNull(const char* szField);

    void setRow(int nRow);

    void finalize();

private:

    void checkResults();

    int mnCols;
    int mnRows;
    int mnCurrentRow;
    char** mpaszResults;
};


class CppSQLite3StatementA
{
public:

    CppSQLite3StatementA();

    CppSQLite3StatementA(const CppSQLite3StatementA& rStatement);

    CppSQLite3StatementA(sqlite3* pDB, sqlite3_stmt* pVM);

    virtual ~CppSQLite3StatementA();

    CppSQLite3StatementA& operator=(const CppSQLite3StatementA& rStatement);

    int execDML();

    CppSQLite3QueryA execQuery();

    bool bind(int nParam, const char* szValue);
    bool bind(int nParam, const int nValue);
    bool bind(int nParam, const double dwValue);
    bool bind(int nParam, const unsigned char* blobValue, int nLen);
    bool bindNull(int nParam);

    void reset();

    void finalize();

private:

    bool checkDB();
    bool checkVM();

    sqlite3* mpDB;
    sqlite3_stmt* mpVM;
	CAtlStringA lastMsg;
};


class CppSQLite3DBA
{
public:

    CppSQLite3DBA();

    virtual ~CppSQLite3DBA();

    bool open(const char* szFile);

	int execSys(const char* szSQL);

	void EncryptDB(const char* szDBFile,const char* szKey);

    void close();

	bool tableExists(const char* szTable);

    int execDML(const char* szSQL);

    CppSQLite3QueryA execQuery(const char* szSQL);

    int execScalar(const char* szSQL);

    CppSQLite3Table getTable(const char* szSQL);

    CppSQLite3StatementA compileStatement(const char* szSQL);

    sqlite_int64 lastRowId();

    void interrupt() 
	{
		//sqlite3_interrupt(mpDB); 
	}

    void setBusyTimeout(int nMillisecs);

    static const char* SQLiteVersion() { return SQLITE_VERSION; }

	//释放所有资源
	static void Release();

	sqlite3* GetSqliteDB();
private:

    CppSQLite3DBA(const CppSQLite3DBA& db);
    CppSQLite3DBA& operator=(const CppSQLite3DBA& db);

    sqlite3_stmt* compile(const char* szSQL);

    bool checkDB();

    sqlite3* mpDB;
    int mnBusyTimeoutMs;
	CAtlStringA lastMsg;
};

#endif
