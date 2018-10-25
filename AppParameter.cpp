#include "StdAfx.h"
#include "AppParameter.h"
#include "Include.h"

CAppParameter::CAppParameter(void)
{
}


CAppParameter::~CAppParameter(void)
{
}


CAppParameter* CAppParameter::GetInstance()
{
	static CAppParameter appParam;
	return &appParam;
}

bool CAppParameter::LoadParameter(const char* pszDbAddr, const char* pszDbName, const char* pszDbUser, const char* pszDbPassword, const char* pszTableName)
{
	//连接本地数据库
	CString strLogs;
	CADODatabase mainConfig;
	mainConfig.SetTimeout(10);
	mainConfig.SetMax(1);
	if(!mainConfig.Init(pszDbAddr, pszDbName, pszDbUser, pszDbPassword))
	{
		throw std::exception("Load Parameter, Init Local db faild");
	}

	return LoadParamValue(&mainConfig, pszTableName);
}

bool CAppParameter::LoadParamValue(CADODatabase* db, const char* pszTableName)
{
	CString strSql;
	strSql.Format("select * from %s", pszTableName);

	CADORecordset Rs(db);
	if(Rs.Open(strSql.GetBuffer()))
	{
		while (!Rs.IsEOF())
		{	
			CString strSection, strKey, strValue, strUnionKey;
			Rs.GetFieldValue("section",strSection);
			Rs.GetFieldValue("key",strKey);
			Rs.GetFieldValue("value",strValue);
			strUnionKey = strSection + strKey;
			m_paramList[strUnionKey.GetBuffer()] = strValue.GetBuffer();
			Rs.MoveNext();
		}
		Rs.Close();
	}
	return true;
}