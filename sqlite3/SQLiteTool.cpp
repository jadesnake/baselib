#include "StdAfx.h"
#include "SQLiteTool.h"
#include "../dir/Dir.h"

//wxsqlite最新版本特性
typedef int (WINAPIV* pOpenDBV2)(const void *filename,sqlite3 **ppDb,int flags,const char *zVfs);
typedef int (WINAPIV* pExtendRetCodes)(sqlite3 *db,int onoff);
typedef int (WINAPIV* pUsrAdd)( sqlite3 *db, const void *pKey,const void* pPwd ,int pwdLen,bool isAdmin);
typedef int (WINAPIV* pUsrAuth)( sqlite3 *db, const void *Name,const void* pPwd ,int pwdLen);
typedef int (WINAPIV* pInitialize)(void);
typedef int (WINAPIV* pShutdown)(void);
//
typedef int (WINAPIV* pOpenDB16)( const void *filename, sqlite3 **ppDb );
typedef int (WINAPIV* pOpenDB)( const char *filename, sqlite3 **ppDb);
typedef const void * (WINAPIV* pGetErrMsg16)(sqlite3 *);
typedef const char * (WINAPIV* pGetErrMsg)(sqlite3 *);
typedef int (WINAPIV* pGetErrCode)(sqlite3 *);

typedef int (WINAPIV* pExecuteSQL)(sqlite3*, const char *sql, 
								   int (*callback)(void*,int,char**,char**), 
								   void *,                                   
								   char **errmsg      );

typedef int (WINAPIV* pCloseDB)(sqlite3 *);
typedef int (WINAPIV* pCloseDBV2)(sqlite3 *);
typedef int (WINAPIV* pFinalize)(sqlite3_stmt *pStmt);
typedef int (WINAPIV* pDBStep)(sqlite3_stmt *pStmt);
typedef sqlite3_int64 (WINAPIV* pGetLastInsertRowID)(sqlite3*);
typedef int (WINAPIV* pSetBusyTimeout)(sqlite3*, int ms);

typedef int (WINAPIV* pPrepare16_V2)(
									 sqlite3 *db,           
									 const void *zSql,      
									 int nByte,             
									 sqlite3_stmt **ppStmt, 
									 const void **pzTail     
									 );

typedef int (WINAPIV* pPrepare_V2)(
								  sqlite3 *db,           
								  const char *zSql,      
								  int nByte,             
								  sqlite3_stmt **ppStmt,  
								  const char **pzTail
								  );

typedef int (WINAPIV* pPrepare)(sqlite3 *db,const char *zSql,int nByte,sqlite3_stmt **ppStmt,const char **pzTail);

typedef int (WINAPIV* pSqliteChanges)(sqlite3*);
typedef int (WINAPIV* pReset)(sqlite3_stmt *pStmt);
typedef int (WINAPIV* pBindText)(sqlite3_stmt*, int, const char*, int n, void(*)(void*));
typedef int (WINAPIV* pBindText16)(sqlite3_stmt*, int, const void*, int, void(*)(void*));
typedef int (WINAPIV* pBindInt)(sqlite3_stmt*, int, int);

typedef int (WINAPIV* pBindDouble)(sqlite3_stmt*, int, double);

typedef int (WINAPIV* pBindBlob)(sqlite3_stmt*, int, const void*, int n, void(*)(void*));

typedef int (WINAPIV* pBindNull)(sqlite3_stmt*, int);

typedef int (WINAPIV* pGetColumnCount)(sqlite3_stmt *pStmt);

typedef const unsigned char * (WINAPIV* pGetColumnText)(sqlite3_stmt*, int iCol);
typedef const void * (WINAPIV* pGetColumnUnicodeText)(sqlite3_stmt*, int iCol);

typedef double (WINAPIV* pGetColumnDouble)(sqlite3_stmt*, int iCol);

typedef int (WINAPIV* pGetColumnInt)(sqlite3_stmt*, int iCol);
typedef __int64 (WINAPIV* pGetColumnInt64)(sqlite3_stmt*,int iCol);

typedef int (WINAPIV* pGetColumnByteLen)(sqlite3_stmt*, int iCol);
typedef int (WINAPIV* pGetColumnByteLen16)(sqlite3_stmt*, int iCol);

typedef const void * (WINAPIV* pGetColumnBlob)(sqlite3_stmt*, int iCol);

typedef const char * (WINAPIV* pGetColumnName)(sqlite3_stmt*, int N);
typedef const void * (WINAPIV* pGetColumnUnicodeName)(sqlite3_stmt*, int N);

typedef const char * (WINAPIV* pGetColumnDeclType)(sqlite3_stmt*,int);
typedef const void * (WINAPIV* pGetColumnDeclType16)(sqlite3_stmt*,int);

typedef int (WINAPIV* pGetColumnType)(sqlite3_stmt*, int iCol);

typedef void * (WINAPIV* pSqliteMalloc)(int);
typedef void * (WINAPIV* pSqliteRealloc)(void*, int);
typedef void  (WINAPIV* pSqliteFree)(void*);

typedef int (WINAPIV* pReSetKey)(sqlite3 *db, const void *pKey, int nKey);

typedef int (WINAPIV* pSetKey)( sqlite3 *db, const void *pKey, int nKey);


typedef char *(WINAPIV* pMPrintf)(const char*,...);

typedef char *(WINAPIV* pVMPrintf)(const char*, va_list);

typedef int (WINAPIV* pGetTable)(sqlite3 * pDB, const char * pszSql,char *** pszResult, int * pnRow,int * pnColumn, char **pzErrmsg);

//释放表对象
typedef void (WINAPIV* pFreeTable)(char **result);

typedef void (WINAPIV* pInterrupt)(sqlite3* pDb);

#define SQLITE_FILE_NAME _T("sqlite3.dll")

CSQLiteTool::CSQLiteTool(void)
{
	CString strFilename=GetDllPath(SQLITE_FILE_NAME)+SQLITE_FILE_NAME;
	dll.Load(strFilename);
}

CSQLiteTool::~CSQLiteTool(void)
{
	
}

//释放资源
void CSQLiteTool::Release()
{
	
}
int CSQLiteTool::Initialize()
{
	pInitialize pFunction = dll.FindFunction<pInitialize>("sqlite3_initialize");
	if(pFunction)
	{
		return pFunction();
	}
	return -1;

}
int CSQLiteTool::Shutdown()
{
	pShutdown pFunction = dll.FindFunction<pShutdown>("sqlite3_shutdown");
	if(pFunction)
	{
		return pFunction();
	}
	return -1;
}

void CSQLiteTool::SqliteInterrupt(sqlite3* pDb)
{
	pInterrupt pFunction = dll.FindFunction<pInterrupt>("sqlite3_interrupt");
	if (pFunction != NULL)
	{
		pFunction(pDb);
	}
}

//释放表对象
void CSQLiteTool::FreeTable(char ** ppResult)
{
	pFreeTable pFunction = dll.FindFunction<pFreeTable>("sqlite3_free_table");
	if (pFunction != NULL)
	{
		pFunction(ppResult);
	}
}

//输出内容
char * CSQLiteTool::MPrintf(const char* pszFormat,const char* pszErrMsg,const int nErrCode,char* pszMess)
{
	pMPrintf pFunction = dll.FindFunction<pMPrintf>("sqlite3_mprintf");
	char * pRet=0;
	if (pFunction != NULL)
	{
		pRet=pFunction(pszFormat,pszErrMsg,nErrCode,pszMess);
	}
	return pRet;
}

//输出内容
char * CSQLiteTool::MPrintf(const char* pszFormat,char* pszMsg)
{
	pMPrintf pFunction = dll.FindFunction<pMPrintf>("sqlite3_mprintf");
	char * pRet=0;
	if (pFunction != NULL)
	{
		pRet=pFunction(pszFormat,pszMsg);
	}
	return pRet;
}

//输出内容
char * CSQLiteTool::VMPrintf(const char* pszFormat,va_list& va)
{
	char * pRet=0;
	pVMPrintf pFunction = dll.FindFunction<pVMPrintf>("sqlite3_vmprintf");
	if (pFunction != NULL)
	{
		pRet=pFunction(pszFormat,va);
	}

	return pRet;
}
//重新设置密码
int CSQLiteTool::ResetKey(sqlite3 * pDB, const void *pKey, int nKey)
{
	int nRet=0;
	pReSetKey pFunction = dll.FindFunction<pReSetKey>("sqlite3_rekey");
	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,pKey,nKey);
	}	
	return nRet;
}
int CSQLiteTool::UserAdd(sqlite3 * pDB,const char* nm,const char* pwd,bool isAdmin)
{
	int nRet=0;
	pUsrAdd pFunction = dll.FindFunction<pUsrAdd>("sqlite3_user_add");
	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,nm,pwd,strlen(pwd),isAdmin);
	}
	return nRet;
}
//设置密码
int CSQLiteTool::SetKey( sqlite3 *pDB, const void *pKey, int nKey)
{
	int nRet=0;
	pSetKey pFunction = dll.FindFunction<pSetKey>("sqlite3_key");
	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,pKey,nKey);
	}

	return nRet;
}
int CSQLiteTool::UserLogin(sqlite3 * pDB,const char* nm,const char* pwd)
{
	int nRet=0;
	pUsrAuth pFunction = dll.FindFunction<pUsrAuth>("sqlite3_user_authenticate");
	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,nm,pwd,strlen(pwd));
	}
	return nRet;
}

//打开数据库
int  CSQLiteTool::OpenDB16( const void *filename, sqlite3 **ppDb )
{
	int nRet=0;
	pOpenDB16 pFunction = dll.FindFunction<pOpenDB16>("sqlite3_open16");
	if (pFunction != NULL)
	{
		nRet=pFunction(filename,ppDb);
	}
	return nRet;
}

//打开数据库
int  CSQLiteTool::OpenDB( const char *filename, sqlite3 **ppDb)
{
	int nRet=0;
	pOpenDB pFunction = dll.FindFunction<pOpenDB>("sqlite3_open");
	if (pFunction != NULL)
	{
		nRet=pFunction(filename,ppDb);
	}

	return nRet;
}

//获取错误信息
const void *  CSQLiteTool::GetErrMsg16(sqlite3 * pDB)
{
	const void* pMsg=NULL;
	pGetErrMsg16 pFunction = dll.FindFunction<pGetErrMsg16>("sqlite3_errmsg16");
	if (pFunction != NULL)
	{
		pMsg=pFunction(pDB);
	}

	return pMsg;
}

//获取错误信息
const char *  CSQLiteTool::GetErrMsg(sqlite3 *pDB)
{
	const char* pMsg=NULL;
	pGetErrMsg pFunction = dll.FindFunction<pGetErrMsg>("sqlite3_errmsg");
	if (pFunction != NULL)
	{
		pMsg=pFunction(pDB);
	}
	return pMsg;
}

//获取错误代码
int  CSQLiteTool::GetErrCode(sqlite3 * pDB)
{
	int nRet=0;
	pGetErrCode pFunction = dll.FindFunction<pGetErrCode>("sqlite3_errcode");
	if (pFunction != NULL)
	{
		nRet=pFunction(pDB);
	}
	return nRet;
}

//执行语句
int  CSQLiteTool::ExecuteSQL(sqlite3* pDB, const char *sql, 
				int (*callback)(void*,int,char**,char**), 
				void * arg,                                   
				char **errmsg)
{
	int nRet=0;
	pExecuteSQL pFunction = dll.FindFunction<pExecuteSQL>("sqlite3_exec");
	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,sql,callback,arg,errmsg);
	}

	return nRet;
}

//关闭数据库
int  CSQLiteTool::CloseDB(sqlite3 * pDB)
{
	int nRet=0;
	pCloseDB pFunction = dll.FindFunction<pCloseDB>("sqlite3_close");
	if (pFunction != NULL)
	{
		nRet=pFunction(pDB);
	}

	return nRet;
}

//关闭数据库
int  CSQLiteTool::CloseDBV2(sqlite3 *pDB)
{
	int nRet=0;

	pCloseDBV2 pFunction = dll.FindFunction<pCloseDBV2>("sqlite3_close_v2");

	if (pFunction != NULL)
	{
		nRet=pFunction(pDB);
	}

	return nRet;
}

//释放
int  CSQLiteTool::Finalize(sqlite3_stmt *pStmt)
{
	int nRet=0;

	pFinalize pFunction = dll.FindFunction<pFinalize>("sqlite3_finalize");
	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt);
	}

	return nRet;
}

//执行下一步
int  CSQLiteTool::DBStep(sqlite3_stmt *pStmt)
{
	int nRet=0;
	pDBStep pFunction = dll.FindFunction<pDBStep>("sqlite3_step");
	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt);
	}

	return nRet;
}

//获取最后一条插入的行标识
sqlite3_int64  CSQLiteTool::GetLastInsertRowID(sqlite3* pDB)
{
	sqlite3_int64 nRet=0;
	pGetLastInsertRowID pFunction = dll.FindFunction<pGetLastInsertRowID>("sqlite3_last_insert_rowid");
	if (pFunction != NULL)
	{
		nRet=pFunction(pDB);
	}
	return nRet;
}

//设置超时
int  CSQLiteTool::SetBusyTimeout(sqlite3* pDB, int ms)
{
	int nRet=0;

	pSetBusyTimeout pFunction = dll.FindFunction<pSetBusyTimeout>("sqlite3_busy_timeout");

	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,ms);
	}

	return nRet;
}

//获取表
int CSQLiteTool::GetTable(sqlite3 * pDB, const char * pszSql,char *** pszResult, int * pnRow,int * pnColumn, char **pzErrmsg )
{
	int nRet=0;

	pGetTable pFunction = dll.FindFunction<pGetTable>("sqlite3_get_table");

	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,pszSql,pszResult,pnRow,pnColumn,pzErrmsg);
	}

	return nRet;
}

//预编译
int CSQLiteTool::Prepare(sqlite3 *pDB,const char * pszSql,int nByte,sqlite3_stmt **ppStmt,const char **pzTail)
{
	int nRet=0;

	pPrepare pFunction = dll.FindFunction<pPrepare>("sqlite3_prepare");

	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,pszSql,nByte,ppStmt,pzTail);
	}

	return nRet;
}

//预编译
int  CSQLiteTool::Prepare16_V2(
				  sqlite3 *pDB,           
				  const void * pSql,      
				  int nByte,             
				  sqlite3_stmt **ppStmt, 
				  const void **pzTail     
				  )
{
	int nRet=0;

	pPrepare16_V2 pFunction = dll.FindFunction<pPrepare16_V2>("sqlite3_prepare16_v2");

	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,pSql,nByte,ppStmt,pzTail);
	}

	return nRet;
}

//预编译
int CSQLiteTool::Prepare_V2(
			   sqlite3 * pDB,           
			   const char * pSql,       
			   int nByte,             
			   sqlite3_stmt **ppStmt,  
			   const char **pzTail    
			   )
{
	int nRet=0;

	pPrepare_V2 pFunction = dll.FindFunction<pPrepare_V2>("sqlite3_prepare_v2");

	if (pFunction != NULL)
	{
		nRet=pFunction(pDB,pSql,nByte,ppStmt,pzTail);
	}

	return nRet;
}

//发生变化
int  CSQLiteTool::SqliteChanges(sqlite3* pDB)
{

	int nRet=0;

	pSqliteChanges pFunction = dll.FindFunction<pSqliteChanges>("sqlite3_changes");

	if (pFunction != NULL)
	{
		nRet=pFunction(pDB);
	}

	return nRet;
}

//重置
int  CSQLiteTool::Reset(sqlite3_stmt *pStmt)
{
	int nRet=0;

	pReset pFunction = dll.FindFunction<pReset>("sqlite3_reset");

	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt);
	}

	return nRet;
}

//绑定文本
int  CSQLiteTool::BindText(sqlite3_stmt* pStmt, int nBind , const char* pszBuff, int nParam, void(* pCallBack)(void*))
{
	int nRet=0;

	pBindText pFunction = dll.FindFunction<pBindText>("sqlite3_bind_text");

	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,nBind,pszBuff,nParam,pCallBack);
	}

	return nRet;
}

//绑定文本
int  CSQLiteTool::BindText16(sqlite3_stmt* pStmt, int nBind, const void* pszBuff, int nParam, void(* pCallBack)(void*))
{
	int nRet=0;
	pBindText16 pFunction = dll.FindFunction<pBindText16>("sqlite3_bind_text16");
	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,nBind,pszBuff,nParam,pCallBack);
	}

	return nRet;
}

//绑定整型
int  CSQLiteTool::BindInt(sqlite3_stmt* pStmt, int nParam1, int nParam2)
{
	int nRet=0;

	pBindInt pFunction = dll.FindFunction<pBindInt>("sqlite3_bind_int");

	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,nParam1,nParam2);
	}

	return nRet;
}

//绑定浮点型
int  CSQLiteTool::BindDouble(sqlite3_stmt* pStmt, int nParam1, double nParam2)
{
	int nRet=0;
	pBindDouble pFunction = dll.FindFunction<pBindDouble>("sqlite3_bind_double");
	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,nParam1,nParam2);
	}

	return nRet;
}

//绑定blob
int  CSQLiteTool::BindBlob(sqlite3_stmt* pStmt, int nParam1, const void* pBuff, int nParam2, void(* pCallBack)(void*))
{
	int nRet=0;

	pBindBlob pFunction = dll.FindFunction<pBindBlob>("sqlite3_bind_blob");

	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,nParam1,pBuff,nParam2,pCallBack);
	}

	return nRet;
}

//绑定null值
int  CSQLiteTool::BindNull(sqlite3_stmt* pStmt, int nIndex)
{
	int nRet=0;

	pBindNull pFunction = dll.FindFunction<pBindNull>("sqlite3_bind_null");

	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,nIndex);
	}

	return nRet;
}

//获取列数
int  CSQLiteTool::GetColumnCount(sqlite3_stmt *pStmt)
{
	int nRet=0;

	pGetColumnCount pFunction = dll.FindFunction<pGetColumnCount>("sqlite3_column_count");

	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt);
	}

	return nRet;
}

//获取列文本
const unsigned char *  CSQLiteTool::GetColumnText(sqlite3_stmt* pStmt, int iCol)
{
	const unsigned char* pBuff=NULL;

	pGetColumnText pFunction = dll.FindFunction<pGetColumnText>("sqlite3_column_text");
	if (pFunction != NULL)
	{
		pBuff=pFunction(pStmt,iCol);
	}

	return pBuff;
}

//获取列文本
const void *  CSQLiteTool::GetColumnUnicodeText(sqlite3_stmt* pStmt, int iCol)
{
	const void *  pBuff=NULL;

	pGetColumnUnicodeText pFunction = dll.FindFunction<pGetColumnUnicodeText>("sqlite3_column_text16");

	if (pFunction != NULL)
	{
		pBuff=pFunction(pStmt,iCol);
	}

	return pBuff;
}

//获取列double值
double  CSQLiteTool::GetColumnDouble(sqlite3_stmt* pStmt, int iCol)
{

	double dValue=0;
	pGetColumnDouble pFunction = dll.FindFunction<pGetColumnDouble>("sqlite3_column_double");
	if (pFunction != NULL)
	{
		dValue=pFunction(pStmt,iCol);
	}

	return dValue;
}

//获取列整型值
int  CSQLiteTool::GetColumnInt(sqlite3_stmt* pStmt, int iCol)
{
	int nRet=0;
	pGetColumnInt pFunction = dll.FindFunction<pGetColumnInt>("sqlite3_column_int");
	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,iCol);
	}

	return nRet;
}
__int64 CSQLiteTool::GetColumnInt64(sqlite3_stmt* pStmt,int iCol)
{
	__int64 nRet=0;
	pGetColumnInt64 pFunction = dll.FindFunction<pGetColumnInt64>("sqlite3_column_int64");
	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,iCol);
	}
	return nRet;
}

//获取长度
int  CSQLiteTool::GetColumnByteLen(sqlite3_stmt* pStmt, int iCol)
{
	int nRet=0;
	pGetColumnByteLen pFunction = dll.FindFunction<pGetColumnByteLen>("sqlite3_column_bytes");
	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,iCol);
	}

	return nRet;
}

//获取长度
int  CSQLiteTool::GetColumnByteLen16(sqlite3_stmt* pStmt, int iCol)
{
	int nRet=0;
	pGetColumnByteLen16 pFunction = dll.FindFunction<pGetColumnByteLen16>("sqlite3_column_bytes16");
	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,iCol);
	}

	return nRet;
}

//获取列blob值
const void *  CSQLiteTool::GetColumnBlob(sqlite3_stmt* pStmt, int iCol)
{
	const void * pBuff=NULL;
	pGetColumnBlob pFunction = dll.FindFunction<pGetColumnBlob>("sqlite3_column_blob");
	if (pFunction != NULL)
	{
		pBuff=pFunction(pStmt,iCol);
	}

	return pBuff;
}

//获取列名
const char *  CSQLiteTool::GetColumnName(sqlite3_stmt* pStmt, int iCol)
{
	const char * pBuff=NULL;
	pGetColumnName pFunction = dll.FindFunction<pGetColumnName>("sqlite3_column_name");
	if (pFunction != NULL)
	{
		pBuff=pFunction(pStmt,iCol);
	}
	return pBuff;
}

//获取列名
const void *  CSQLiteTool::GetColumnUnicodeName(sqlite3_stmt* pStmt, int iCol)
{
	const void * pBuff=NULL;
	pGetColumnName pFunction = dll.FindFunction<pGetColumnName>("sqlite3_column_name16");
	if (pFunction != NULL)
	{
		pBuff=pFunction(pStmt,iCol);
	}

	return pBuff;
}

//获取decl类型
const char *  CSQLiteTool::GetColumnDeclType(sqlite3_stmt* pStmt,int iCol)
{
	const char * pBuff=NULL;

	pGetColumnDeclType pFunction = dll.FindFunction<pGetColumnDeclType>("sqlite3_column_decltype");

	if (pFunction != NULL)
	{
		pBuff=pFunction(pStmt,iCol);
	}

	return pBuff;
}

//获取decl类型
const void *  CSQLiteTool::GetColumnDeclType16(sqlite3_stmt* pStmt,int iCol)
{
	const void * pBuff=NULL;
	pGetColumnDeclType pFunction = dll.FindFunction<pGetColumnDeclType>("sqlite3_column_decltype16");
	if (pFunction != NULL)
	{
		pBuff=pFunction(pStmt,iCol);
	}

	return pBuff;
}

//获取列类型
int  CSQLiteTool::GetColumnType(sqlite3_stmt* pStmt, int iCol)
{
	int nRet=0;
	pGetColumnType pFunction = dll.FindFunction<pGetColumnType>("sqlite3_column_type");
	if (pFunction != NULL)
	{
		nRet=pFunction(pStmt,iCol);
	}

	return nRet;
}

//分配内存
void *  CSQLiteTool::SqliteMalloc(int nLen)
{
	void * pBuff=NULL;
	pSqliteMalloc pFunction = dll.FindFunction<pSqliteMalloc>("sqlite3_malloc");
	if (pFunction != NULL)
	{
		pBuff=pFunction(nLen);
	}
	return pBuff;
}

//重新分配内存
void *  CSQLiteTool::SqliteRealloc(void* pBuff, int nLen)
{
	void * pRetBuff=NULL;
	pSqliteRealloc pFunction = dll.FindFunction<pSqliteRealloc>("sqlite3_realloc");
	if (pFunction != NULL)
	{
		pRetBuff=pFunction(pBuff,nLen);
	}

	return pRetBuff;
}

//释放内存
void   CSQLiteTool::SqliteFree(void* pBuff)
{
	pSqliteFree pFunction = dll.FindFunction<pSqliteFree>("sqlite3_free");
	if (pFunction != NULL)
	{
		pFunction(pBuff);
	}
}
void  CSQLiteTool::SqliteFree(const char* pBuff)
{
	SqliteFree((void*)pBuff);
}
void  CSQLiteTool::SqliteFree(const wchar_t* pBuff)
{
	SqliteFree((void*)pBuff);
}
