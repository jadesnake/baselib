#pragma once

#include "sqlite3.h"
#include "../SingleCpp.h"
#include "../baseX.h"
class CSQLiteTool
{
public:
	GarboSingle(CSQLiteTool);
public:
	int Initialize();
	int Shutdown();

	//打开数据库
	int  OpenDB16( const void *filename, sqlite3 **ppDb );

	//打开数据库
	int  OpenDB( const char *filename, sqlite3 **ppDb);

	//获取错误信息
	const void *  GetErrMsg16(sqlite3 *);

	//获取错误信息
	const char *  GetErrMsg(sqlite3 *);

	//获取错误代码
	int  GetErrCode(sqlite3 *);

	//执行语句
	int  ExecuteSQL(sqlite3*, const char *sql, 
		int (*callback)(void*,int,char**,char**), 
		void *,                                   
		char **errmsg);

	//关闭数据库
	int  CloseDB(sqlite3 *);

	//关闭数据库
	int  CloseDBV2(sqlite3 *);

	//释放
	int  Finalize(sqlite3_stmt *pStmt);

	//执行下一步
	int  DBStep(sqlite3_stmt *pStmt);

	//获取最后一条插入的行标识
	sqlite3_int64  GetLastInsertRowID(sqlite3*);

	//设置超时
	int  SetBusyTimeout(sqlite3*, int ms);

	int Prepare_V2(
		sqlite3 *db,           
		const char *zSql,       
		int nByte,             
		sqlite3_stmt **ppStmt,  
		const char **pzTail    
		);

	//预编译
	int  Prepare16_V2(
		sqlite3 *db,           
		const void *zSql,      
		int nByte,             
		sqlite3_stmt **ppStmt, 
		const void **pzTail     
		);

	//预编译
	int Prepare(sqlite3 *db,const char *zSql,int nByte,sqlite3_stmt **ppStmt,const char **pzTail);

	//发生变化
	int  SqliteChanges(sqlite3*);

	//重置
	int  Reset(sqlite3_stmt *pStmt);

	//绑定文本
	int  BindText(sqlite3_stmt*, int, const char*, int n, void(*)(void*));

	//绑定文本
	int  BindText16(sqlite3_stmt*, int, const void*, int, void(*)(void*));

	//绑定整型
	int  BindInt(sqlite3_stmt*, int, int);

	//绑定浮点型
	int  BindDouble(sqlite3_stmt*, int, double);

	//绑定blob
	int  BindBlob(sqlite3_stmt*, int, const void*, int n, void(*)(void*));

	//绑定null值
	int  BindNull(sqlite3_stmt*, int);

	//获取列数
	int  GetColumnCount(sqlite3_stmt *pStmt);

	//获取列文本
	const unsigned char *  GetColumnText(sqlite3_stmt*, int iCol);

	//获取列文本
	const void *  GetColumnUnicodeText(sqlite3_stmt*, int iCol);

	//获取列double值
	double  GetColumnDouble(sqlite3_stmt*, int iCol);

	//获取列整型值
	int  GetColumnInt(sqlite3_stmt*, int iCol);

	__int64 GetColumnInt64(sqlite3_stmt*,int iCol);

	//获取长度
	int  GetColumnByteLen(sqlite3_stmt*, int iCol);

	//获取长度
	int  GetColumnByteLen16(sqlite3_stmt*, int iCol);

	//获取列blob值
	const void *  GetColumnBlob(sqlite3_stmt*, int iCol);

	//获取列名
	const char *  GetColumnName(sqlite3_stmt*, int N);

	//获取列名
	const void *  GetColumnUnicodeName(sqlite3_stmt*, int N);

	//获取decl类型
	const char *  GetColumnDeclType(sqlite3_stmt*,int);

	//获取decl类型
	const void *  GetColumnDeclType16(sqlite3_stmt*,int);

	//获取列类型
	int  GetColumnType(sqlite3_stmt*, int iCol);

	//分配内存
	void *  SqliteMalloc(int);

	//重新分配内存
	void *  SqliteRealloc(void*, int);

	//释放内存
	void   SqliteFree(void*);
	void   SqliteFree(const char*);
	void   SqliteFree(const wchar_t*);

	//重新设置密码
	int ResetKey(sqlite3 *db, const void *pKey, int nKey);

	//设置密码
	int SetKey( sqlite3 *db, const void *pKey, int nKey);

	//输出内容
	char * MPrintf(const char* pszFormat,const char* pszErrMsg,const int nErrCode,char* pszMess);

	//输出内容
	char * MPrintf(const char* pszFormat,char* pszMsg);

	//输出内容
	char * VMPrintf(const char* pszFormat,va_list& va);

	//获取表
	int GetTable(sqlite3 * pDB, const char * pszSql,char *** pszResult, int * pnRow,int * pnColumn, char **pzErrmsg );

	//释放表对象
	void FreeTable(char **result);

	void SqliteInterrupt(sqlite3*);

	//添加用户 utf-8
	int UserAdd(sqlite3 * pDB,const char* nm,const char* pwd,bool isAdmin);
	int UserLogin(sqlite3 * pDB,const char* nm,const char* pwd);
	//释放资源
	void Release();
private:
	base::DLL	dll;
};
