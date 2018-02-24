
#ifndef _CppSQLite3_H_
#define _CppSQLite3_H_

/*
	CppSQLite3DBA 许久没有维护
	CppSQLite3DBU 是最新版
*/

#ifdef _UNICODE
#include "CppSQLite3U.h"
#define CppSQLite3DB CppSQLite3DBU
#define CppSQLite3Exception CppSQLite3ExceptionU
#define CppSQLite3Statement CppSQLite3StatementU
#define CppSQLite3Query CppSQLite3QueryU

#else
#include "CppSQLite3A.h"
#define CppSQLite3DB CppSQLite3DBA
#define CppSQLite3Exception CppSQLite3ExceptionA
#define CppSQLite3Statement CppSQLite3StatementA
#define CppSQLite3Query CppSQLite3QueryA

#endif
#endif