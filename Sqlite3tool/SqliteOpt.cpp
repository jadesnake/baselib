#include "StdAfx.h"
#include "SqliteOpt.h"
#include "jsoncpp/JsonUtils.h"
#include "StringUtils/NumberConvert.h"
#include "StringUtils/StringUtils.h"
#include "dir/Dir.h"

namespace SqliteOpt{
	CAtlString Bool2Save(bool b)
	{
		return (b?L"1":L"0");
	}

	bool Save2Bool(CAtlString v)
	{
		if(v.CompareNoCase(L"true")==0)
			return true;
		if(v.CompareNoCase(L"yes")==0)
			return true;
		if(v.CompareNoCase(L"1")==0)
			return true;
		if(v.CompareNoCase(L"suc")==0)
			return true;
		if(v.CompareNoCase(L"success")==0)
			return true;
		if(v.CompareNoCase(L"y")==0)
			return true;
		return false;
	}

	CAtlString FormatSql(CAtlString& in)
	{
		in.Replace(_T("/"),_T("//"));
		in.Replace(_T("'"),_T("''"));
		in.Replace(_T("["),_T("/["));
		in.Replace(_T("]"),_T("/]"));
		in.Replace(_T("%"),_T("/%"));
		in.Replace(_T("&"),_T("/&"));
		in.Replace(_T("("),_T("/("));
		in.Replace(_T(")"),_T("/)"));
		return in;
	}
	CAtlString TurnRawSql(CAtlString& in)
	{
		in.Replace(_T("//"),_T("/"));
		in.Replace(_T("''"),_T("'"));
		in.Replace(_T("/["),_T("["));
		in.Replace(_T("/]"),_T("]"));
		in.Replace(_T("/%"),_T("%"));
		in.Replace(_T("/&"),_T("&"));
		in.Replace(_T("/("),_T("("));
		in.Replace(_T("/)"),_T(")"));
		return in;
	};
	void BeginTransaction(CppSQLite3DBU *db)
	{
		CAtlString strRet;
		ExecuteSys(db,_T("BEGIN TRANSACTION;"),strRet);
	}
	void EndTransaction(CppSQLite3DBU *db,bool bSubmit)
	{
		CAtlString strRet;
		if (bSubmit)
			ExecuteSys(db,_T("COMMIT TRANSACTION;"),strRet);
		else
			ExecuteSys(db,_T("ROLLBACK TRANSACTION;"),strRet);
	}
	Columns GetColumns(CppSQLite3DBU* db,LPCTSTR table)
	{
		Columns ret;
		CppSQLite3QueryU result(NULL);
		CAtlString sql;
		sql.Format(L"PRAGMA table_info('%s');",table);
		ExecuteQuery(db,sql,result);
		while( !result.eof() )
		{
			Column column;
			column.name = result.getStringField(L"name",L"");
			column.type = result.getStringField(L"type",L"");
			ret[column.name] = column;
			result.nextRow();
		}
		result.finalize();
		return ret;
	}
	bool ExecuteQuery(CppSQLite3DBU *db,LPCTSTR pszSQL,CppSQLite3QueryU& result)
	{
		CAtlString strSQL=pszSQL;
		if(strSQL.IsEmpty())
		{
			return false;
		}
		bool bSuccess=true;
		try
		{
			result = db->execQuery(strSQL);
		}
		catch (...)
		{
			bSuccess=false;
		}
		return bSuccess;
	}
	bool ExecuteSys(CppSQLite3DBU *db,LPCTSTR pszSQL,CAtlString & strRet)
	{
		int nRet;
		CAtlString strSQL=pszSQL;
		if(strSQL.IsEmpty())
		{
			return false;
		}
		try
		{
			nRet = db->execSys(strSQL,strRet);
		}
		catch(CppSQLite3ExceptionU e)
		{
			nRet=SQLITE_FAIL;
		}
		catch (...)
		{
			nRet=SQLITE_FAIL;
		}
		if (nRet !=SQLITE_OK )
		{
			nRet=SQLITE_FAIL;
		}
		return nRet == SQLITE_OK;
	}
	bool ExecuteDML(CppSQLite3DBU *db,LPCTSTR pszSQL)
	{
		int nRet;
		CAtlString strSQL=pszSQL;
		if(strSQL.IsEmpty())
		{
			return false;
		}
		try
		{
			nRet = db->execDML(strSQL);
		}
		catch(CppSQLite3ExceptionU e)
		{
			nRet=SQLITE_FAIL;
		}
		catch(...)
		{
			nRet=SQLITE_FAIL;
		}
		if (nRet != SQLITE_OK )
		{
			nRet=SQLITE_FAIL;
		}
		return nRet == SQLITE_OK;
	}
	SqliteOpt::Pager Json2Pager(const std::string& sJson)
	{
		SqliteOpt::Pager p;
		Json::Value val;
		if(!JsonUtils::ParseJson(sJson,val))
			return p;
		std::string index = JsonUtils::SafeJsonValue(val,"index");
		if(index.empty())
			index = JsonUtils::SafeJsonValue(val,"page");
		std::string size = JsonUtils::SafeJsonValue(val,"size");
		if(size.empty())
			size = JsonUtils::SafeJsonValue(val,"count");
		if(!index.empty()&&!size.empty())
		{
			int nTmpA=0,nTmpB=0;
			if(base::StringToInt(index,&nTmpA) && base::StringToInt(size,&nTmpB))
			{
				p.index = nTmpA;
				p.size = nTmpB;
			}
		}
		return p;
	}
	/*---------------------------------------------------------------------------*/
	size_t BuildWhereFromJson(const std::string& sJson,SIMPLEWHERES& sw)
	{
		Json::Value jvWhere;
		JsonUtils::ParseJson(sJson,jvWhere);
		if(jvWhere.isArray())
		{
			for(size_t t=0;t<jvWhere.size();t++)
			{
				SqliteOpt::SimpleWhere one;
				one.key = (TCHAR*)CA2CT(JsonUtils::SafeJsonValue(jvWhere[t],"key").c_str(),CP_UTF8);
				one.val = (TCHAR*)CA2CT(JsonUtils::SafeJsonValue(jvWhere[t],"value").c_str(),CP_UTF8);
				one.level = JsonUtils::SafeJsonValueINT(jvWhere[t],"level");
				one.like = JsonUtils::SafeJsonValueBOOL(jvWhere[t],"like");
				one.bIN = JsonUtils::SafeJsonValueBOOL(jvWhere[t],"in");
				one.likeOrder = (TCHAR*)CA2CT(JsonUtils::SafeJsonValue(jvWhere[t],"likeOrder").c_str(),CP_UTF8);
				if(jvWhere[t].isMember("value"))
				{
					if(jvWhere[t]["value"].isNull())
						one.type = L"null";
					else if(jvWhere[t]["value"].isString())
						one.type = L"str";
					else if(jvWhere[t]["value"].isInt() || jvWhere[t]["value"].isUInt() || jvWhere[t]["value"].isIntegral())
						one.type = L"int";
					else if(jvWhere[t]["value"].isBool())
						one.type = L"bool";
				}
				sw.push_back(one);
			}
		}
		if(jvWhere.isObject())
		{
			Json::ValueIterator vit = jvWhere.begin();
			for(vit;vit!=jvWhere.end();vit++)
			{
				SqliteOpt::SimpleWhere one;
				one.key = vit.name().c_str();
				one.val = CA2CT(JsonUtils::SafeJsonValue(jvWhere,vit.name()).c_str(),CP_UTF8);
				if(jvWhere[vit.name()].isNull())
					one.type = L"null";
				else if(jvWhere[vit.name()].isString())
					one.type = L"str";
				else if(jvWhere[vit.name()].isInt() || jvWhere[vit.name()].isUInt() || jvWhere[vit.name()].isIntegral())
					one.type = L"int";
				else if(jvWhere[vit.name()].isBool())
					one.type = L"bool";
				sw.push_back(one);
			}
		}
		return sw.size();
	}
	CAtlString BuildReWhereX(const SIMPLEWHERES& sw,ReWhereX *rw)
	{
		CAtlString sqlwhere(L"where");
		if(sw.size()==0)	return L"";
		struct sortByKey
		{
			bool operator()(size_t k1,size_t k2) const
			{
				return (k1<k2);
			}
		};
		std::map<size_t,SqliteOpt::SimpleWhere,sortByKey> hashwhere;
		for(size_t t=0;t<sw.size();t++)
		{
			const SqliteOpt::SimpleWhere *swval = &sw[t];
			if(swval->key.IsEmpty())
				continue;
			if(hashwhere.find(swval->level)==hashwhere.end())
				hashwhere.insert(std::make_pair(swval->level,*swval));
			else
				hashwhere.insert(std::make_pair(hashwhere.size(),*swval));
		}
		if(hashwhere.size()==0)	return L"";
		std::map<size_t,SqliteOpt::SimpleWhere,sortByKey>::iterator it = hashwhere.begin();
		for(it;it!=hashwhere.end();it++)
		{
			std::wstringstream wss;
			if(sqlwhere!=L"where")
				wss << L" and";
			CAtlString dbval;
			bool handled = false;
			if(rw)
			{
				CAtlString hval = rw->OnReDoX(it->second,handled);
				if(handled)
				{
					if(!hval.IsEmpty())
					{
						wss<<L" "<<hval.GetString();
						sqlwhere += wss.str().c_str();
					}
  					continue;
				}
 			}
			std::vector<std::wstring> fKEYS;
			if(base::SplitByW(it->second.key.GetString(),',',fKEYS)>1)
			{
				wss << L" (";
				for(size_t ik=0;ik<fKEYS.size();ik++)
				{
					std::wstring kn = fKEYS[ik];
					if(it->second.like)
					{
						wss<<L" "<<kn.c_str();
						if(it->second.likeOrder.CompareNoCase(L"left")==0)
							wss<<L" like '%"<<it->second.val.GetString()<<L"'";
						else if(it->second.likeOrder.CompareNoCase(L"right")==0)
							wss<<L" like '"<<it->second.val.GetString()<<L"%'";
						else
							wss<<L" like '%"<<it->second.val.GetString()<<L"%'";
					}
					else
					{
						if(it->second.type==L"null")
							dbval = L"null";
						else
							dbval = it->second.val;
						if(it->second.type==L"null")
						{
							wss<<L" ("<<it->second.key.GetString()<<L" is "<<dbval.GetString()<<L" or "<<it->second.key.GetString()<<L"='')";
						}
						else if(it->second.type==L"str" || it->second.type.IsEmpty())
						{
							wss<<L" "<<it->second.key.GetString();
							wss<<L"='"<<dbval.GetString()<<L"'";
						}
						else
						{
							wss<<L" "<<it->second.key.GetString();
							wss<<L"="<<dbval.GetString()<<L"";
						}
					}
					if(ik==fKEYS.size()-1)
						continue;
					wss << L" or";
				}
				wss << L")";
			}
			else
			{
				if(it->second.bIN)
				{
					wss<<L" "<<it->second.key.GetString();
					wss<<L" in ("<<it->second.val.GetString()<<L")";
				}
				else if(it->second.like)
				{
					wss<<L" "<<it->second.key.GetString();
					if(it->second.likeOrder.CompareNoCase(L"left")==0)
						wss<<L" like '%"<<it->second.val.GetString()<<L"'";
					else if(it->second.likeOrder.CompareNoCase(L"right")==0)
						wss<<L" like '"<<it->second.val.GetString()<<L"%'";
					else
						wss<<L" like '%"<<it->second.val.GetString()<<L"%'";
				}
				else
				{
					if(it->second.type==L"null")
						dbval = L"null";
					else
						dbval = it->second.val;
					if(it->second.type==L"null")
					{
						wss<<L" ("<<it->second.key.GetString()<<L" is "<<dbval.GetString()<<L" or "<<it->second.key.GetString()<<L"='')";
					}
					else if(it->second.type==L"str" || it->second.type.IsEmpty())
					{
						wss<<L" "<<it->second.key.GetString();
						wss<<L"='"<<dbval.GetString()<<L"'";
					}
					else
					{
						wss<<L" "<<it->second.key.GetString();
						wss<<L"="<<dbval.GetString()<<L"";
					}
				}
			}
			sqlwhere += wss.str().c_str();
 		}
		if(sqlwhere == L"where")
			return L"";
		return sqlwhere;
	}
	CAtlString BuildUsualWhere(const SIMPLEWHERES& sw)
	{
		return BuildReWhereX(sw,NULL);
	}
	/*---------------------------------------------------------------------------*/
	CAtlString BuildInsertSql(CAtlString tbname,Columns *col)
	{
		CAtlString sql;
		std::wstringstream keys,vals;
		if(tbname.IsEmpty() || col==NULL || col->size()==0)
			return sql;
		Columns::iterator it = col->begin();
		for(it;it!=col->end();it++)
		{
			keys<<it->first.GetString()<<L",";
			if(it->second.type!=L"exp")
				FormatSql(it->second.name);
			if(it->second.type==L"str")
			{
				vals << L"'";
				if(it->second.name.IsEmpty())
					vals << L"";
				else
					vals << it->second.name.GetString();
				vals << L"'"<<L",";
			}
			else
				vals <<it->second.name.GetString()<<L",";
		}
		sql.Format(L"insert into %s(%s)values(%s);", tbname, keys.str().substr(0,keys.str().length()-1).c_str(), vals.str().substr(0,vals.str().length()-1).c_str());
		return sql;
	}
	CAtlString BuildUpdateSql(CAtlString tbname,Columns *col)
	{
		CAtlString sql;
		std::wstringstream exps;
		if(tbname.IsEmpty() || col==NULL || col->size()==0)
			return sql;
		Columns::iterator it = col->begin();
		for(it;it!=col->end();it++)
		{
			exps<<it->first.GetString()<<L"=";
			if(it->second.type!=L"exp")
				FormatSql(it->second.name);
			if(it->second.type==L"str")
				exps << L"'" << it->second.name.GetString() << L"'"<<L",";
			else
				exps <<it->second.name.GetString()<<L",";
		}
		sql.Format(L"update %s set %s", tbname, exps.str().substr(0,exps.str().length()-1).c_str());
		return sql;
	}
	/*---------------------------------------------------------------------------*/
	bool IsNoPwdDB(CAtlString file,CSQLiteTool* driver)
	{
		CppSQLite3DBU db(driver);
		if(!db.open(file))
			return false;
	 	CppSQLite3QueryU rs = db.execQuery(L"select count(*) from sqlite_master;");
		int nTab = rs.getIntField(0);
		rs.finalize();
		db.close();
		return (nTab>0);
	}
	/*---------------------------------------------------------------------------*/
	Access::Access(HWND win)
		:m_db(NULL),base::BackLogicBase(win)
 	{
		m_opened = false;
		m_init = false;
		m_gnm = L"SqliteOptAccess";
		m_db = NULL;
		m_noasync = false;
 	}
	void Access::SetNoAsync()
	{
		m_noasync = true;
	}
	Access::~Access()
	{
		if(m_db)
		{
			delete m_db;
			m_db = NULL;
		}
	}
	void Access::Append(std::tr1::shared_ptr<Job> job)
	{
		job->mMyNsrsh = m_nsrsh;
		if(m_noasync)
		{
			job->Execute(m_db);
			job->FireSync();
			job->Release();
		}
		else
		{
			m_jobsW.push_back(job);
			start();
		}
	}
	void Access::SyncRun(std::tr1::shared_ptr<Job> job)
	{
		job->mMyNsrsh = m_nsrsh;
		if(m_noasync)
		{
			job->Execute(m_db);
			job->FireSync();
			job->Release();
		}
		else
		{
			job->WantSync();
			m_jobsW.push_back(job);
			start();
			job->WantSync(); //等待完成
		}
	}
	void Access::Done()
	{

	}
	bool Access::Run()
	{
		//如果停止那么不要执行后续任务
		if(BackLogicBase::STOP==getStatus())
			return false;
		base::IListImpl<Job,CLock> tmpArray;
		std::tr1::shared_ptr<Job> oneJob;
		tmpArray.takeAll(m_jobsW);
		if(tmpArray.count()==0)
		{
			return false;
		}
 		CAtlString desc;
		oneJob = tmpArray.getBegin();
		while(oneJob = tmpArray.getNext())
		{
 			try{
				if(BackLogicBase::STOP==getStatus())
					return false;
 				oneJob->Execute(m_db);
				if(oneJob->IsNeedUI())
					m_jobsD.push_back(oneJob);
				else
					oneJob->FireSync();
 			}
			catch(Json::LogicError e) {
				TRACEA(e.what());
			}
			catch(...){

			}
		}
		return true;
	}
	void Access::HandleDone()
	{
		base::IListImpl<Job,CLock> doneArray;
		doneArray.takeAll(m_jobsD);
		std::tr1::shared_ptr<Job> oneJob = doneArray.getBegin();
		while(oneJob = doneArray.getNext())
		{
			try{
				oneJob->Release();
			}
			catch(Json::LogicError e) {
				TRACEA(e.what());
			}
		}
		doneArray.clear();
	}
	CAtlString Access::GetFileDB()
	{
		CLockGuard guard(&m_lkDBFILE);
		return CAtlString(m_dbfile);	
	}
	void Access::UsePwd(CAtlString pwd)
	{
		m_pwd = pwd;
	}
	void Access::SetBackupDB(CAtlString backfile)
	{
		m_dbBack = backfile;
	}
	bool Access::CheckCrypt()
	{
		if(m_init==false || m_db==NULL)
			return false;
		bool blankDB = false;
		CppSQLite3QueryU q = m_db->execQuery(L"select count(*) from sqlite_master;");
		bool br = false;
		if(q.eof())
			br = true;
		else
			br = false;
		q.finalize();
		return br;
	}
	bool Access::CheckValid()
	{
		if(m_init==false || m_db==NULL)
			return false;
		bool blankDB = false;
		CppSQLite3QueryU q = m_db->execQuery(_T("PRAGMA integrity_check;"));
		if(!q.eof())
		{
			if(_tccmp(q.getStringField(_T("integrity_check")),_T("ok"))==0)
				blankDB = false;
		}
		q.finalize();
		return (!blankDB);
	}
	bool Access::OpenCheck(CAtlString file,CSQLiteTool* driver,Job *check)
	{
		bool bRet = false;
		if(!Open(file,driver))
		{
			m_db->close();
			::DeleteFile(file);
			if(!m_dbBack.IsEmpty())
				::CopyFile(m_dbBack,file,FALSE);
			//创建新库
			if(!Open(file,driver))
			{
 				return false;
			}
		}
		if(!CheckValid())
		{	//数据库损坏，创建新库
			m_db->close();
			::DeleteFile(file);
			//执行备份库copy
			if(!m_dbBack.IsEmpty())
				::CopyFile(m_dbBack,file,FALSE);
			if(!Open(file,driver))
				return false;
		}
		CLockGuard guard(&m_lkDBFILE);
		if(check)
		{
			check->mMyNsrsh = m_nsrsh;
			check->Execute(m_db);
			check->Release();
		}
		bRet = true;
		return bRet;
	}
	bool Access::OpenCheck(CAtlString file,CAtlString driver,Job *check)
	{
		bool bRet = false;
 		if(!Open(file,driver))
		{
			m_db->close();
			::DeleteFile(file);
 			//创建新库
			if(!Open(file,driver))
				return false;
		}
		if(!CheckValid())
		{	//数据库损坏，创建新库
			m_db->close();
			::DeleteFile(file);
			//执行备份库copy
			if(!m_dbBack.IsEmpty())
				::CopyFile(m_dbBack,file,FALSE);
			if(!Open(file,driver))
				return false;
		}
		CLockGuard guard(&m_lkDBFILE);
 		if(check)
		{
			check->mMyNsrsh = m_nsrsh;
			check->Execute(m_db);
			check->Release();
		}
		bRet = true;
		return bRet;
	}
	bool Access::IsOpen()
	{
		return m_opened;
	}
	void Access::StartWAL()
	{
		if(m_init && m_opened && m_db)
			m_db->execDML(L"pragma journal_mode=wal;");
	}
	bool Access::OpenClear(CAtlString file,CSQLiteTool* driver)
	{
		if(Open(file,driver))
			return m_db->ResetKeyDB(L"");
		return false;
	}
	bool Access::Open(CAtlString file,CSQLiteTool* driver)
	{
		CLockGuard guard(&m_lkDBFILE);
		//初始化sqlite
		bool fExist = IsPathFind(file);
		if(m_init==false)
		{
			m_db = new CppSQLite3DBU(driver);
 			m_init = true;
		}
		bool bRet = false;
		if(m_dbfile.CompareNoCase(file)!=0 && m_db)
		{
			m_db->close();
			m_opened = false;
		}
		try
		{
			if(!m_pwd.IsEmpty())
				bRet = m_db->EncryptDB(file,m_pwd);
			else
				bRet = m_db->open(file);
			m_opened = bRet;
			if(m_opened && fExist && CheckCrypt())
				return false;
			if(m_opened &&!CheckValid())
				return false;
		}
		catch (...)
		{
			bRet=false;
		}
		if(bRet)
		{
			m_opened = true;
			m_dbfile = file;
		}
		return bRet;
	}
	void Access::DriverInit()
	{
		CLockGuard guard(&m_lkDBFILE);
		if(m_db && m_init)
			m_db->GetSqlTool()->Initialize(m_db->GetSqlTool());
	}
	bool Access::Open(CAtlString file,CAtlString driver)
	{
		CLockGuard guard(&m_lkDBFILE);
		//初始化sqlite
		bool fExist = IsPathFind(file);
		if(m_init==false)
		{
			m_db = new CppSQLite3DBU(NULL);
			m_db->GetSqlTool()->Initialize((char*)CT2CA(driver));
			m_init = true;
		}
		bool bRet = false;
		if(m_dbfile.CompareNoCase(file)!=0 && m_db)
		{
			m_db->close();
			m_opened = false;
		}
		try
		{
			if(!m_pwd.IsEmpty())
				bRet = m_db->EncryptDB(file,m_pwd);
			else
				bRet = m_db->open(file);
			m_opened = bRet;
			if(m_opened && fExist && CheckCrypt())
				return false;
			if(m_opened &&!CheckValid())
				return false;
		}
		catch (...)
		{
			bRet=false;
		}
		if(bRet)
		{
			m_opened = true;
			m_dbfile = file;
		}
		return bRet;
	}
	void Access::Quit()
	{
		__super::close(INFINITE);
		if(m_opened)
		{
			m_opened = false;
			m_db->close();
		}
	}
	void Access::GlobalName(CAtlString nm)
	{
		CLockGuard guard(&m_lkDBFILE);
		m_gnm = nm;
	}
	void Access::SetNsrsh(CAtlString nsrsh)
	{
		m_nsrsh = nsrsh;
	}
};