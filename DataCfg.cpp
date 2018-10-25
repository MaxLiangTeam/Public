/*---------------------------------------------------------------------------
文件    : 
描述    : 数据配置文件
作者    : wsl
版本    : V1.0
时间    : 2012-02-22
---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "DataCfg.h"
#include "Kernel.h"
#include "TinyEncode.h"

int CDataCfg::m_nVersion = 0;
int CDataCfg::m_nWorkMode = -1;
map<CString,stSTSTableInfo> CDataCfg::m_mapCellSTSTab;
map<CString,stSTSTableInfo> CDataCfg::m_mapEnbSTSTab;
map<CString,vector<CString>> CDataCfg::m_mapKpiTable;
map<CString,stDataTimeInfo> CDataCfg::m_mapDataTimeTable;
map<CString,int> CDataCfg::m_mapConfig;
vector<int> CDataCfg::m_vecDSCPoint;
vector<int> CDataCfg::m_vecEGMSPoint;
map<CString,TFreqModelInfo> CDataCfg::m_mapFreqModeTab;
vector<double> CDataCfg::m_vecIreland;
map<CString,st2GSTSInfo> CDataCfg::m_map2GSTSTab;

CDataCfg::CDataCfg(void)
{
	//---------------------------------------------------------------------------------------------------------------
	AddParamCfg("ADD ENB:","ENBID","KEY","P");
	AddParamCfg("ADD ENB:","ENBNAME","ENBNAME","P");
	AddParamCfg("ADD ENB:","MPMODE","MPMODE","P");
	AddParamCfg("ADD ENB:","ENBTYPE","ENBTYPE","P");

	//CGU信息
	AddParamCfg("ADD ENBCONNECT:","ENBID","KEY","P");  
	AddParamCfg("ADD ENBCONNECT:","INCN","INCN","P"); 
	AddParamCfg("ADD ENBCONNECT:","INPN","INPN","P"); 
	AddParamCfg("ADD ENBCONNECT:","DESTNODE","DESTNODE","P");

	AddParamCfg("SET ENBIDLETS:","ENBID","KEY","P");
	AddParamCfg("SET ENBIDLETS:","CGN","CGN","P");
	AddParamCfg("SET ENBIDLETS:","TSCOUNT","TSCOUNT","P");

	AddParamCfg("ADD ENBMONITORTS:","ENBID","KEY","P");
	AddParamCfg("ADD ENBMONITORTS:","STCN","STCN","P");
	AddParamCfg("ADD ENBMONITORTS:","TSRATE","TSRATE","P");

	//该基站激活的CELL 和 TRX
	AddParamCfg("ACT ENB:","ENBID","KEY","P");
	AddParamCfg("ACT ENB:","CELLIDLIST","CELLIDLIST","P");
	AddParamCfg("ACT ENB:","TRXIDLIST","TRXIDLIST","P");
	
	//---------------------------------------------------------------------------------------------------------------
	AddParamCfg("ADD GCELL:","CELLID","KEY","P");
	AddParamCfg("ADD GCELL:","CELLNAME","CELLNAME","P");
	AddParamCfg("ADD GCELL:","MCC","MCC","P");
	AddParamCfg("ADD GCELL:","MNC","MNC","P");
	AddParamCfg("ADD GCELL:","LAC","LAC","P");
	AddParamCfg("ADD GCELL:","CI","CI","P");
	AddParamCfg("ADD GCELL:","TYPE","TYPE","P");
	AddParamCfg("ADD GCELL:","NCC","NCC","P");
	AddParamCfg("ADD GCELL:","BCC","BCC","P");

	AddParamCfg("ADD CELLBIND2ENB:","CELLID","KEY","P");
	AddParamCfg("ADD CELLBIND2ENB:","ENBID","ENBID","P");

	//查询跳频指令
	AddParamCfg("SET GCELLHOPTP:","CELLID","KEY","P");
	AddParamCfg("SET GCELLHOPTP:","FHMODE","FHMODE","P");

	//查小区逻辑频点
	AddParamCfg("ADD GCELLFREQ:","CELLID","KEY","P");

	//查小区信道管理高级参数
	AddParamCfg("SET GCELLCHMGAD:","CELLID","KEY","P");
	AddParamCfg("SET GCELLCHMGAD:","QTRUPWRSHARE","QTRUPWRSHARE","P");
	
	//---------------------------------------------------------------------------------------------------------------
	AddParamCfg("ADD GTRX:","TRXID","KEY","P");
	AddParamCfg("ADD GTRX:","CELLID","CELLID","P");
	AddParamCfg("ADD GTRX:","FREQ","FREQ","P");//频点
	AddParamCfg("ADD GTRX:","ISMAINBCCH","ISMAINBCCH","P");
	AddParamCfg("ADD GTRX:","TRXNO","TRXNO","P");//载波号

	AddParamCfg("SET GTRXCHAN:","TRXID","KEY","P");
	AddParamCfg("SET GTRXCHAN:","CHTYPE","CHTYPE","P");//信道类型
	AddParamCfg("SET GTRXCHAN:","GPRSCHPRI","GPRSCHPRI","P");//PDCH信道优选类型

	AddParamCfg("SET GTRXDEV:","TRXID","KEY","P");
	AddParamCfg("SET GTRXDEV:","POWL","POWL","P");//功率等级
	AddParamCfg("SET GTRXDEV:","POWT","POWT","P");//功率类型
	AddParamCfg("SET GTRXDEV:","TCHAJFLAG","TCHAJFLAG","P");//TCH速率调整允许
	AddParamCfg("SET GTRXDEV:","RCVMD","RCVMD","P");//接收模式
	AddParamCfg("SET GTRXDEV:","SNDMD","SNDMD","P");//发送模式

	AddParamCfg("ADD TRXBIND2PHYBRD:","TRXID","KEY","P");
	AddParamCfg("ADD TRXBIND2PHYBRD:","TRXPN","TRXPN","P");//载频板通道号
	AddParamCfg("ADD TRXBIND2PHYBRD:","CN","CN","P");//机柜号
	AddParamCfg("ADD TRXBIND2PHYBRD:","SRN","SRN","P");
	AddParamCfg("ADD TRXBIND2PHYBRD:","SN","SN","P");
	AddParamCfg("ADD TRXBIND2PHYBRD:","TRXTP","TRXTP","P");//载频板类型
	AddParamCfg("ADD TRXBIND2PHYBRD:","ANTPASSNO","ANTPASSNO","P");//天馈通道号

	AddParamCfg("SET GTRXHOP:","TRXID","KEY","P");
	AddParamCfg("SET GTRXHOP:","HOPTYPE","HOPTYPE","P");//载频跳频

	//---------------------------------------------------------------------------------------------------------------
	AddParamCfg("ADD ENBRXUBRD:","ENBID-CN-SRN-SN","KEY","P");
	AddParamCfg("ADD ENBRXUBRD:","RXUNAME","RXUNAME","P");
	AddParamCfg("ADD ENBRXUBRD:","RXUPOS","RXUPOS","P");//RXU单板位置号
	AddParamCfg("ADD ENBRXUBRD:","RXUSPEC","RXUSPEC","P");//RXU规格

	AddParamCfg("SET ENBRXUBP:","ENBID-CN-SRN-SN","KEY","P");
	AddParamCfg("SET ENBRXUBP:","RXUTYPE","RXUTYPE","P");//RXU板类型
	AddParamCfg("SET ENBRXUBP:","SNDRCVMODE","SNDRCVMODE","P");//MRFU/GRFU单板的收发模式

	//---------------------------------------------------------------------------------------------------------------
	AddParamCfg("ADD GEXT2GCELL:","EXT2GCELLID","KEY","P");

	AddParamCfg("ADD G2GNCELL:","SRC2GNCELLID","KEY","P");
	AddParamCfg("ADD G2GNCELL:","NBR2GNCELLID","NBR2GNCELLID","P");

	//CDD 命令
	AddParamCfg("DSP LICUSAGE: TYPE=Current;","","","P");
	AddParamCfg("LST ENBRXUBRD:;","","","P");
	AddParamCfg("For BAM version:","","","P");
}

CDataCfg::~CDataCfg(void)
{
}

void CDataCfg::AddParamCfg(CString strCmd,CString strName,CString strByname,CString strType)
{
	stField field;
	field.strName = strName;
	field.strType = strType;
	map<CString,map<CString,stField>>::iterator it = m_mapParamCfg.find(strCmd);
	if (it == m_mapParamCfg.end())
	{
		map<CString,stField> mapParam;
		mapParam.insert(make_pair(strByname,field));
		m_mapParamCfg.insert(make_pair(strCmd,mapParam));
	}
	else
	{
		it->second.insert(make_pair(strByname,field));
	}
}


bool CDataCfg::Load()
{

	if (!LoadOMMBCfg())
	{
		return false;
	}

	if (!LoadSTSCfgTab())
	{
		return false;
	}

	if (!LoadKPICfg())
	{
		//todo
	}

	if (!LoadDataTimeCfgTab())
	{
		return false;
	}

	if (!Load2GERICSTSData() || !Load2GHWSTSData())
	{
	}

	return true;
}

bool CDataCfg::LoadOMMBCfg()
{
	CString strSql;
	strSql.Format("select * from PLUGIN_ZTE_LTE_OMMB where Operation <> 0");
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open((LPCTSTR)strSql);
		m_vecOMMBCfg.clear();
		while (!Rs.IsEOF())
		{		
			stOMMBCfg cfgOMMB;
			Rs.GetFieldValue("Id",cfgOMMB.nID);
			Rs.GetFieldValue("OMMB_NAME",cfgOMMB.strName);
			Rs.GetFieldValue("OMMB_IP",cfgOMMB.strIP);
			Rs.GetFieldValue("OMMB_Port",cfgOMMB.nPort);
			Rs.GetFieldValue("OMMB_UserName",cfgOMMB.strUser);
			Rs.GetFieldValue("OMMB_Password",cfgOMMB.strPassword);
			Rs.GetFieldValue("Operation",cfgOMMB.nOperation);
			Rs.GetFieldValue("Not_Adjust_Begin",cfgOMMB.dNotAdjustBegin);
			Rs.GetFieldValue("Not_Adjust_End",cfgOMMB.dNotAdjustEnd);
			Rs.GetFieldValue("Not_Adjust_Valid",cfgOMMB.nNotAdjustValid);

			CTinyEncode d;
			d.Decode(cfgOMMB.strUser,cfgOMMB.strUser);
			d.Decode(cfgOMMB.strPassword,cfgOMMB.strPassword);
			m_vecOMMBCfg.push_back(cfgOMMB);

			Rs.MoveNext();
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("[CDataCfg::LoadOMMBCfg] 数据库查询出错:%s\n%s",strSql,Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

//加载STS数据配置表
bool CDataCfg::LoadSTSCfgTab()
{
	CString strSql;
	strSql.Format("select * from PLUGIN_ZTE_LTE_STS_TABLE_CONFIG order by ID");
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open((LPCTSTR)strSql);
		CDataCfg::m_mapCellSTSTab.clear();
		CDataCfg::m_mapEnbSTSTab.clear();
		int nCi(0),nEi(0);
		while (!Rs.IsEOF())
		{
			CString strID,strLevel,strTable,strCounter,strRemark;
			Rs.GetFieldValue("Id",strID);
			Rs.GetFieldValue("Level",strLevel);
			Rs.GetFieldValue("TableName",strTable);
			Rs.GetFieldValue("Counter",strCounter);
			//strCounter.Replace(".","");
			Rs.GetFieldValue("Remark",strRemark);
			Rs.MoveNext();

			if (strLevel == "CELL")
			{
				map<CString,stSTSTableInfo>::iterator it = m_mapCellSTSTab.find(strTable);
				if (it == m_mapCellSTSTab.end())
				{
					nCi++;
					stSTSTableInfo tableInfo;
					tableInfo.strKey.Format("C%d",nCi);
					tableInfo.strLevel = strLevel;
					tableInfo.strTable = strTable;
					tableInfo.vecCounter.push_back(strCounter);
					tableInfo.strCounterList = strCounter;
					m_mapCellSTSTab[tableInfo.strTable] = tableInfo;
				} 
				else
				{
					stSTSTableInfo &tableInfo = it->second;
					tableInfo.vecCounter.push_back(strCounter);
					tableInfo.strCounterList += ","+ strCounter;
				}
			}
			else if (strLevel == "ENB")
			{
				map<CString,stSTSTableInfo>::iterator it = m_mapEnbSTSTab.find(strTable);
				if (it == m_mapEnbSTSTab.end())
				{
					nEi++;
					stSTSTableInfo tableInfo;
					tableInfo.strKey.Format("C%d",nEi);
					tableInfo.strLevel = strLevel;
					tableInfo.strTable = strTable;
					tableInfo.vecCounter.push_back(strCounter);
					tableInfo.strCounterList = strCounter;
					m_mapEnbSTSTab[tableInfo.strTable] = tableInfo;
				} 
				else
				{
					stSTSTableInfo &tableInfo = it->second;
					tableInfo.vecCounter.push_back(strCounter);
					tableInfo.strCounterList += ","+ strCounter;
				}
			}
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("[CDataCfg::LoadSTSCfgTab] 数据库查询出错:%s\n%s",strSql,Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool CDataCfg::LoadKPICfg()
{
	CString strSql;
	strSql.Format("select * from PLUGIN_ZTE_LTE_FORMULA order by TimeMark");
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open((LPCTSTR)strSql);
		CDataCfg::m_mapKpiTable.clear();
		while (!Rs.IsEOF())
		{
			CString strID,strKPI,strk;
			vector<CString> vecCounter;
			Rs.GetFieldValue("TimeMark",strID);
			Rs.GetFieldValue("expression",strKPI);

			strKPI.Replace(".","");
			strKPI.Replace("(","");
			strKPI.Replace(")","");
			strKPI.Replace("-","+");
			strKPI.Replace("/","+");
			strKPI.Replace("*","+");
			while (!strKPI.IsEmpty())
			{
				CString strCount = cutstr(strKPI,'+');
				if (!strCount.IsEmpty())
				{
					vecCounter.push_back(strCount);
					continue;
				}
			}
			Rs.MoveNext();

			map<CString,vector<CString>>::iterator it = CDataCfg::m_mapKpiTable.find(strID);
			if (it == CDataCfg::m_mapKpiTable.end())
			{
				vector<CString> vecCount;
				vecCount.insert(vecCount.end(),vecCounter.begin(),vecCounter.end());
				CDataCfg::m_mapKpiTable[strID] = vecCount;
			} 
			else
			{
				vector<CString> &vecCount = it->second;
				vecCount.insert(vecCount.end(),vecCounter.begin(),vecCounter.end());
			}
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("[CDataCfg::LoadKPICfg] 数据库查询出错:%s\n%s",strSql,Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool CDataCfg::LoadDataTimeCfgTab()
{
	CString strSql;
	strSql.Format("select * from PLUGIN_ZTE_LTE_DATA_TIME_CONFIG");
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open((LPCTSTR)strSql);
		CDataCfg::m_mapDataTimeTable.clear();
		while (!Rs.IsEOF())
		{		
			CString strMark,strDate,strIndex;
			Rs.GetFieldValue("TimeMark",strMark);
			Rs.GetFieldValue("Date",strDate);
			Rs.GetFieldValue("PeroidIndex",strIndex);
			Rs.MoveNext();

			map<CString,stDataTimeInfo>::iterator it = m_mapDataTimeTable.find(strMark);
			if (it == m_mapDataTimeTable.end())
			{
				stDataTimeInfo TimeInfo;
				TimeInfo.strKey = strMark;
				TimeInfo.nDate = atoi(strDate);
				TimeInfo.nIndex = atoi(strIndex);
				m_mapDataTimeTable[strMark] = TimeInfo;
			} 
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("[CDataCfg::LoadDataTimeCfgTab] 数据库查询出错:%s\n%s",strSql,Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool CDataCfg::Load2GERICSTSData()
{
	CString strSql;
	COleDateTime coCurTime(time(NULL));
	CString strTime = coCurTime.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString str2gTable = "obj_eric2g_BSC_CELL_60_" + strTime;
	int nRes = coCurTime.GetMinute() % 60;
	CString strPeriod = TimeFormat(coCurTime,"%H%M",0,0,0-nRes-60) + TimeFormat(coCurTime,"%H%M",0,0,0-nRes);

	if (g_bIsTest)
	{
		str2gTable = "obj_eric2g_BSC_CELL_15_151123";
		strPeriod = "10001100";
	}

	strSql.Format(" set ansi_warnings off set arithabort off select distinct [EXCHID] ,[OBJECT_ID],[CELL]"
		" ,(Ulthp3egdata+Ulthp2egdata+Ulthp1egdata+Ulbgegdata+Ulthp3gdata+Ulthp2gdata+Ulthp1gdata+Ulbggdata)/8192 as Ulflow"
		" ,(Dlthp3egdata+Dlthp2egdata+Dlthp1egdata+DlBGegdata+DlTHP3gdata+DlTHP2gdata+DlTHP1gdata+DlBGgdata)/8192 as Dlflow"
		" FROM %s where period = '%s' ",str2gTable,strPeriod);

	CADORecordset Rs(g_pDB2GDC);
	try
	{
		Rs.Open((LPCTSTR)strSql);
		CDataCfg::m_map2GSTSTab.clear();

		while (!Rs.IsEOF())
		{
			st2GSTSInfo st2GInfo;
			Rs.GetFieldValue("EXCHID",st2GInfo.strBSC);
			Rs.GetFieldValue("CELL",st2GInfo.strCell);
			Rs.GetFieldValue("Ulflow",st2GInfo.dUlFlow);
			Rs.GetFieldValue("Dlflow",st2GInfo.dDlFlow);
			Rs.MoveNext();
			st2GInfo.strKey = st2GInfo.strBSC + st2GInfo.strCell;
			m_map2GSTSTab[st2GInfo.strKey] = st2GInfo;

		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("[CDataCfg::Load2GSTSData] 数据库查询出错:%s\n%s",strSql,Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool CDataCfg::Load2GHWSTSData()
{
	CString strSql;
	COleDateTime coCurTime(time(NULL));
	CString strTime = coCurTime.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString str2gTable = "obj_hw2g_BSC_CELL_60_" + strTime;
	int nRes = coCurTime.GetMinute() % 60;
	CString strPeriod = TimeFormat(coCurTime,"%H%M",0,0,0-nRes-60) + TimeFormat(coCurTime,"%H%M",0,0,0-nRes);

	if (g_bIsTest)
	{
		str2gTable = "obj_hw2g_BSC_CELL_60_160615";
		strPeriod = "10001100";
	}

	strSql.Format(" set ansi_warnings off set arithabort off select distinct [EXCHID] ,[CELL]"
		" ,(C1279177439+C1279179453)/1024 as Ulflow"
		" ,(C1279178439+C1279180454)/1024 as Dlflow"
		" FROM %s where period = '%s' ",str2gTable,strPeriod);

	CADORecordset Rs(g_pDB2GHWDC);
	try
	{
		Rs.Open((LPCTSTR)strSql);
		//CDataCfg::m_map2GSTSTab.clear();

		while (!Rs.IsEOF())
		{
			st2GSTSInfo st2GInfo;
			Rs.GetFieldValue("EXCHID",st2GInfo.strBSC);
			Rs.GetFieldValue("CELL",st2GInfo.strCell);
			Rs.GetFieldValue("Ulflow",st2GInfo.dUlFlow);
			Rs.GetFieldValue("Dlflow",st2GInfo.dDlFlow);
			Rs.MoveNext();
			st2GInfo.strKey = st2GInfo.strBSC + st2GInfo.strCell;
			m_map2GSTSTab[st2GInfo.strKey] = st2GInfo;

		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("[CDataCfg::Load2GHWSTSData] 数据库查询出错:%s\n%s",strSql,Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return true;
	}
	return true;
}

bool CDataCfg::LoadParamCfg()
{
	CString strSql;
	strSql.Format("select * from PLUGIN_HW2G_TRXADJUST_CDD");
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open((LPCTSTR)strSql);
		m_vecParamCfg.clear();
		while (!Rs.IsEOF())
		{		
			stParamCfg ParamCfg;
			Rs.GetFieldValue("Name",ParamCfg.strName);
			Rs.GetFieldValue("Command",ParamCfg.strCmd);
			Rs.GetFieldValue("ParamName",ParamCfg.strbyName);

			m_vecParamCfg.push_back(ParamCfg);
			Rs.MoveNext();
		}
		Rs.Close();
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("[CDataCfg::LoadParamCfg] 数据库查询出错:%s\n%s",strSql,Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool CDataCfg::LoadParamCfg(CString strCmd,map<CString,stField>& vecParamCfg)
{
	map<CString,map<CString,stField>>::iterator it = m_mapParamCfg.find(strCmd);
	if (it == m_mapParamCfg.end())
	{
		return false;
	}
	vecParamCfg = it->second;
	return true;
}

bool CDataCfg::InsertParam(CString strName,int nValue,CString strRemark)
{
	CString strSQL;
	strSQL.Format("if not exists (select * from PLUGIN_HW2G_TRXADJUST_WORKMODE_CONFIG where [ConfigName]='%s' and Version=%d and WorkMode=%d) "
		"begin insert into PLUGIN_HW2G_TRXADJUST_WORKMODE_CONFIG (Version,WorkMode,ConfigName,ConfigValue,InsertTime,Remark) "
		"values(%d,%d,'%s',%d,getdate(),'%s') end;",
		strName,CDataCfg::m_nVersion,CDataCfg::m_nWorkMode,CDataCfg::m_nVersion,CDataCfg::m_nWorkMode,strName,nValue,strRemark);

	//如果没有配置 则使用默认配置
	std::map<CString,int>::iterator itMap = CDataCfg::m_mapConfig.find(strName);
	if (itMap == CDataCfg::m_mapConfig.end())
	{
		CDataCfg::m_mapConfig[strName] = nValue;
	}

	return g_pDBTRX->Execute(strSQL);
}

bool CDataCfg::CheckConfig()
{
	//全局
	CDataCfg::InsertParam("DataKeepTime",30,"历史数据保存时间(单位:天;默认:30)");

	//河源
	if (CDataCfg::m_nVersion == VER_HY)
	{
		CDataCfg::InsertParam("StartMin",20,"启动分钟(默认值:20)");
		CDataCfg::InsertParam("StartHour",0,"启动小时(默认值:0)");
		//应急版本
		if (CDataCfg::m_nWorkMode == MODE_URGENT)
		{
			CDataCfg::InsertParam("AutoRun",0,"是否自动执行(0:否;1:是)");
		}
	}
	else
	{
		if (CDataCfg::m_nWorkMode == MODE_URGENT)
		{
			CDataCfg::InsertParam("AutoRun",0,"是否自动执行(0:否;1:是)");
			CDataCfg::InsertParam("IsUsedFrequency",0,"应急模式是否使用频率集:0否;1是(默认0)");
		}
		else
		{
			CDataCfg::InsertParam("StartMin",48,"启动分钟(参考值:45-50)");
			CDataCfg::InsertParam("AdjustTime",55,"调整指令下发时间(范围:50-59)");
			CDataCfg::InsertParam("IsUsedFrequency",0,"日常模式是否使用频率集:0否;1是(默认0)");
		}
	}
	return true;
}

bool CDataCfg::CheckTable(CString strName,CString strType,vector<CString>& vecCol)
{
	CString strTable;
	CString strTemp;

	strTable.Format("CREATE TABLE [%s](DATE varchar(10) NULL,PERIOD varchar(10) NULL,\
					PERLEN int NULL,PERIOD_INDEX int NULL,UPDATE_TIME datetime NULL,\
					OMMB_NAME varchar(255) NULL,SubNetwork int NULL,Enb_NAME int NULL,\
					CELL_ID varchar(64) NULL,CELL_NAME varchar(255) NULL,CELL_TYPE varchar(64) NULL,\
					EUtranCellTDD int NULL,threshSvrLow float NULL,thresholdOfRSRP float NULL,\
					timeToTrigger float NULL,rsrpSrvTrd float NULL,trigTime float NULL,\
					REMARK varchar(255) NULL",strName);
	for (int i = 0; i < (int)vecCol.size(); i++)
	{
		strTemp.Format(",[%s] %s NULL",vecCol[i],strType);
		strTable += strTemp;
	}
	strTable += ")";

	CString strSql;
	strSql.Format("if not exists (select * from sysobjects where [name] = '%s' and xtype='U') "
		"begin "
		"%s"
		"end;",strName,strTable);
	if(!g_pDBTRX->Execute(strSql))//,g_pDBTRX->GetLastError()
	{
		CString strLog;
		strLog.Format("[CDataCfg::CheckTable] SQL语句执行出错: %s\n%s",strSql,g_pDBTRX->GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
	}

	CADORecordset Rs(g_pDBTRX);
	try
	{
		map<CString,int> mapField;
		strSql.Format("SELECT top 1 * from %s",strName);
		Rs.Open((LPCTSTR)strSql);
		CADOFieldInfo fldInfo;
		for (int i = 0; i < (int)Rs.GetFieldCount(); i++)
		{ 
			Rs.GetFieldInfo(i,&fldInfo);
			mapField[fldInfo.m_strName] = i;
		}
		Rs.Close();
		strSql = "";
		for (int i = 0; i < (int)vecCol.size(); i++)
		{
			map<CString,int>::iterator it = mapField.find(vecCol[i]);
			if (it == mapField.end())
			{
				strTemp.Format("alter table  %s add %s %s;",
					strName,vecCol[i],strType);
				strSql += strTemp;
			}
		}
		if (!strSql.IsEmpty())
		{
			if(!g_pDBTRX->Execute(strSql))//,g_pDBTRX->GetLastError()
			{
				CString strLog;
				strLog.Format("[CDataCfg::CheckTable] SQL语句执行出错: %s\n%s",strSql,g_pDBTRX->GetLastError());
				g_pLog->WriteLog(strLog,LOG_ERROR);
			}
		}
	}
	catch (...)
	{
		CString strLog;
		strLog.Format("[CDataCfg::CheckTable] SQL语句执行出错: %s\n%s",strSql,Rs.GetLastError());
		g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}

bool CDataCfg::GetTableHead(CString strName,map<CString,int>& mapHead)
{
	CString strSql;
	strSql.Format("SELECT top 1 * from %s",strName);
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open((LPCTSTR)strSql);
		CADOFieldInfo fldInfo;
		for (int i = 0; i < (int)Rs.GetFieldCount(); i++)
		{ 
			Rs.GetFieldInfo(i,&fldInfo);
			mapHead.insert(make_pair(fldInfo.m_strName,(int)mapHead.size()));
		}
		Rs.Close();
	}
	catch (...)
	{
		//CString strLog;
		//strLog.Format("[CDataCfg::GetTableHead] SQL语句执行出错: %s\n%s",strSql,Rs.GetLastError());
		//g_pLog->WriteLog(strLog,LOG_ERROR);
		return false;
	}
	return true;
}
