#include "StdAfx.h"
#include "LogicConfig.h"
#include "Include.h"
#include "Kernel.h"



LogicConfig::LogicConfig():m_nKpiMun(0)
{
}


LogicConfig::~LogicConfig(void)
{
}


bool LogicConfig::LoadConfig()
{
	return LoadFormulaConfig()
		&& LoadTaskConfig()
		&& LoadActionConfig()
		&& LoadParamConfig()
		&& LoadWorkConfig();
}

bool LogicConfig::LoadFormulaConfig()
{
	CString strSQL = "select exprId, expression,TimeMark  from PLUGIN_ZTE_LTE_FORMULA where status = 1";
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open(strSQL);
		CString strTg;
		while (!Rs.IsEOF())
		{	
			CString strExprId, strExpression, strTimeMark;
			Rs.GetFieldValue("exprId", strExprId);
			Rs.GetFieldValue("expression", strExpression);
			Rs.GetFieldValue("TimeMark", strTimeMark);
			strExpression.Replace(".","");
			m_formulaConf[strTimeMark][strExprId] = strExpression;
			m_nKpiMun++;
			Rs.MoveNext();
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("数据库查询出错:%s\n%s", strSQL, Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool LogicConfig::LoadTaskConfig()
{
	CString strSQL = "select taskId, Condition, trueTask, falseTask, doAction from PLUGIN_ZTE_LTE_TASK where status = 1";
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open(strSQL);
		CString strTg;
		while (!Rs.IsEOF())
		{	
			tagTaskConfig taskConf;
			Rs.GetFieldValue("taskId", taskConf.strName);
			Rs.GetFieldValue("Condition", taskConf.strCondition);
			Rs.GetFieldValue("trueTask", taskConf.strTrueTask);
			Rs.GetFieldValue("falseTask", taskConf.strFalseTask);
			Rs.GetFieldValue("doAction", taskConf.strDoAction);
			m_taskConf[taskConf.strName.GetBuffer()] = taskConf;
			Rs.MoveNext();
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("数据库查询出错:%s\n%s", strSQL, Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool LogicConfig::LoadActionConfig()
{
	CString strSQL = "select actionId, paramId from PLUGIN_ZTE_LTE_CMD_ACTION ";
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open(strSQL);
		CString strTg;
		while (!Rs.IsEOF())
		{	
			CString strActionId, strParamId;
			Rs.GetFieldValue("actionId", strActionId);
			Rs.GetFieldValue("paramId", strParamId);			
			m_actionConf[strActionId].push_back(strParamId);
			Rs.MoveNext();
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("数据库查询出错:%s\n%s", strSQL, Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool LogicConfig::LoadParamConfig()
{
	CString strSQL = "select paramId, paramName, paramType, valType, minVal, maxVal, step, Value from PLUGIN_ZTE_LTE_CMD_PARAM ";
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open(strSQL);
		CString strTg;
		while (!Rs.IsEOF())
		{	
			CString strParamId;
			tagCmdParamConfig paramConf;
			Rs.GetFieldValue("paramId", strParamId);
			Rs.GetFieldValue("paramName", paramConf.strParamName);	
			Rs.GetFieldValue("paramType", paramConf.strParamType);
			Rs.GetFieldValue("valType", paramConf.nValType);
			Rs.GetFieldValue("minVal", paramConf.nMinVal);
			Rs.GetFieldValue("maxVal", paramConf.nMaxVal);
			Rs.GetFieldValue("step", paramConf.nStep);
			Rs.GetFieldValue("Value", paramConf.nVal);
			m_paramConf[strParamId] = paramConf;
			Rs.MoveNext();
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("数据库查询出错:%s\n%s", strSQL, Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool LogicConfig::LoadWorkConfig()
{
	CString strSQL = "select workId, taskId from PLUGIN_ZTE_LTE_WORK where status = 1";
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open(strSQL);
		CString strTg;
		while (!Rs.IsEOF())
		{	
			CString strWorkId, strTaskId;
			Rs.GetFieldValue("workId", strWorkId);
			Rs.GetFieldValue("taskId", strTaskId);			
			m_workConf[strWorkId] = strTaskId;
			Rs.MoveNext();
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("数据库查询出错:%s\n%s", strSQL, Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}