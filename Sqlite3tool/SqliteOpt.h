/*
	sqlite3 数据库相关工具
	author： jiayh
*/
#pragma once
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <set>

#include "AsyncUtils/AsyncUtils.h"
#include "sqlite3/CppSQLite3U.h"
#include "List.h"

namespace SqliteOpt{
	struct Column
	{
		Column(CAtlString n,CAtlString t){
			name = n;
			type = t;
		}
		Column(char n,CAtlString t){
			name = n;
			type = t;
		}
		Column(){

		}
		CAtlString name;
		CAtlString type;
	};
	typedef std::map<CAtlString,Column> Columns;
	
	struct PairSql
	{
		CAtlString run1;
		CAtlString run2;
	};
	typedef std::vector<PairSql> PairSqls;
	
	/*
		对输入变量做sql转义
		@inParam
			输入变量
		@outParam
			转义结果
		@return
			转义结果
	*/
	CAtlString FormatSql(CAtlString& in);
	/*
		对输入变量做sql还原
		@inParam
			输入变量
		@outParam
			还原结果
		@return
			还原结果
	*/
	CAtlString TurnRawSql(CAtlString& in);

	/*
		执行DML sql（insert，delete，update）
		@inParam
			*db 数据库实例
			pszSQL sql语句
		@return
			true-成功，false-失败
	*/
	bool ExecuteDML(CppSQLite3DBU *db,LPCTSTR pszSQL);
	/*
		执行sqlite命令
		@inParam
			*db 数据库实例
			pszSQL sqlite命令
		@outParam
			&strRet 输出异常信息
		@return
			true-成功，false-失败
	*/
	bool ExecuteSys(CppSQLite3DBU *db,LPCTSTR pszSQL,CAtlString &strRet);
	/*
		执行数据查询
		@inParam
			*db 数据库实例
			pszSQL sql语句
		@outParam
			&result 返回记录集合
		@return
			true-成功，false-失败
	*/
	bool ExecuteQuery(CppSQLite3DBU *db,LPCTSTR pszSQL,CppSQLite3QueryU& result);
	/*
		开启事务
		@inParam
			*db 数据库实例
	*/
	void BeginTransaction(CppSQLite3DBU *db);
	/*
		结束事务
		@inParam
			*db 数据库实例
			bSubmit true-提交，false-回滚
	*/
	void EndTransaction(CppSQLite3DBU *db,bool bSubmit);
	/*
		获取表字段集合
		@inParam
			*db 数据库实例
			table 表名称
		@return
			表字段集合
	*/
	Columns GetColumns(CppSQLite3DBU* db,LPCTSTR table);
	/*
		bool 类型转数据存储类型，如：true-1,false-0
		@inParam
			b 转换的数据
		@return
			转换后数据
	*/
	CAtlString Bool2Save(bool b);
	/*
		存储数据转bool类型数据，支持
		true - 1,true,yes,suc,success,y
		false - 上述之外数据
		@inParam
			v 转换的数据
		@return
			转换后数据
	*/
	bool Save2Bool(CAtlString v);
	
	//验证无密码数据库
	bool IsNoPwdDB(CAtlString file,CSQLiteTool* driver);
	/*
		异步执行sql逻辑任务
	*/
	class Job
	{
	public:
		Job(){ mSync = NULL;  }
		virtual ~Job(){ 
			if(mSync){
				::CloseHandle(mSync);
				mSync = NULL;
			}
		}
		// 同步等待
		inline void WantSync(){
			if(mSync==NULL)
			{
				mSync = ::CreateEvent(NULL,FALSE,FALSE,NULL);
				return ;
			}
			::WaitForSingleObject(mSync,INFINITE);
			Release();
		}
		// 判断是否具有UI特性
		virtual bool IsNeedUI() { 
			if(mSync==NULL)
				return true;
			return false;
		}
		// 执行sql业务
		virtual bool Execute(CppSQLite3DBU *sqlite){ 
			return true;
		}
		// 触发同步完成
		virtual bool FireSync(){
			if(mSync)
			{
				::SetEvent(mSync);
				return true;
			}
			return false;
		}
		// 数据释放
		virtual void Release(){	}
		CAtlString mMyNsrsh;	// 纳税人税号
	protected:
		HANDLE mSync; // 同步事件
	};
	typedef base::IListImpl<Job,CLock> Jobs;

	
	//简单得查询条件
	struct SimpleWhere
	{
		SimpleWhere(){
			level = 0;
			like = false;
			bIN = false;
		}
		SimpleWhere(size_t l,CAtlString k,CAtlString v,bool b){
			level = l;
			key = k;
			val = v;
			like =b;
			bIN = false;
		}
		SimpleWhere(size_t l,CAtlString k,char v,bool b){
			level = l;
			key = k;
			val = v;
			like =b;
			bIN = false;
		}
		size_t level;	//级别
		CAtlString key;	//搜索关键字
		CAtlString val;	//搜索值
		bool like;	//是否模糊
		bool bIN;	//范围搜索
	};
	//用于对where数据进行重构处理
	class ReWhereX
	{
	public:
		virtual ~ReWhereX(){ }
		virtual void Release(){	}
		/*
			@inParam 
				k 关键字
				v 值
				like 是否模糊匹配
			@return
				重构结果
		*/
		virtual CAtlString OnReDoX(const SimpleWhere& sw,bool &handled){ 
 			return L""; 
		}
	};
	typedef std::vector<SimpleWhere> SIMPLEWHERES;
	/*
		生成常用where语句
		@inParam
			sw where结构体
		@return
			where子句
	*/
	CAtlString BuildUsualWhere(const SIMPLEWHERES& sw);
	/*
		根据json生成where结构体
		@inParam
			json格式字符串，格式如下：[{"key": "字段名称", "value": "匹配值", "level": "级别", "like": "是否模糊"}]
		@outParam
			sw where结构体
		@return
			where子句
	*/
	size_t BuildWhereFromJson(const std::string& sJson,SIMPLEWHERES& sw);
	/*
		根据where结构以及修改器，生成where sql子语句,
		@inParam
			sw where结构体
			rw 修改器指针
		@return
			where 子句			
	*/
	CAtlString BuildReWhereX(const SIMPLEWHERES& sw,ReWhereX *rw);
	/*
		生成insert sql语句
		@inParam 
			tbname 表名称
			*col 字段列表
		@return
			生成sql语句
	*/
	CAtlString BuildInsertSql(CAtlString tbname,Columns *col);
	//分页参数对象
	struct Pager
	{
		Pager(){
			index = -1;
			size = -1;
		}
		inline bool IsAll(){
			if(index==-1 && size==-1)
				return true;
			return false;
		}
		inline CAtlString Sql(){
			CAtlString offset;
			if(index<0 || size<0)
				return offset;
			offset.Format(L"limit %d offset %d",size,index);
			return offset;
		}
		long index;	//索引
		long size;	//张数
	};
	/*
		将json[pager]字段反序列化陈pager对象
	@inParam 
		sJson ——分页json参数格式如下：{"index": "索引", "size": "页面大小"}			
	@return
		pager 对象
	*/
	SqliteOpt::Pager Json2Pager(const std::string& sJson);
	
	//sqlite3数据库访问对象
	class Access : public base::BackLogicBase
	{
	public:
		Access(HWND win);
		virtual ~Access();
		void SetNoAsync();
		CAtlString GetFileDB();
		void TestNoPwd(CAtlString file,CSQLiteTool* driver);
		void UsePwd(CAtlString pwd);
		void SetBackupDB(CAtlString backfile);
		bool Open(CAtlString file,CAtlString driver);
		bool Open(CAtlString file,CSQLiteTool* driver);
		bool OpenClear(CAtlString file,CSQLiteTool* driver);
		bool OpenCheck(CAtlString file,CAtlString driver,Job *check);
		bool OpenCheck(CAtlString file,CSQLiteTool* driver,Job *check);
		void Append(std::tr1::shared_ptr<Job> job);
		void SyncRun(std::tr1::shared_ptr<Job> job);
		void Quit();
		void HandleDone();
		void GlobalName(CAtlString nm);
		bool IsOpen();
		void StartWAL();
		void SetNsrsh(CAtlString nsrsh);
	protected:
		void* Query(LPCTSTR pClass)
		{
			CAtlString tmpnm;
			{
				CLockGuard guard(&m_lkDBFILE);
				tmpnm = m_gnm;
			}
			if(tmpnm == pClass)
				return this;
			return NULL;
		}
		bool Run();
		void Done();
		bool CheckValid();
		void DriverInit();
	private:
		Jobs m_jobsW;	//等待执行任务队列
		Jobs m_jobsD;	//执行完成队列
		CLock	m_lkDBFILE;	//访问数据成员保护锁
		CAtlString m_dbBack; //备份文件
		CAtlString m_dbfile;  //数据库文件
		CAtlString m_pwd;	//访问密码
		CAtlString m_gnm;	//全局名称
		CAtlString m_nsrsh; //纳税人名称
		CppSQLite3DBU *m_db; //sqlite3数据对象
		bool m_opened;	//数据库打开状态
		bool m_init;	//数据库初始状态
		bool m_noasync;	//同步异步标记
	};
};
