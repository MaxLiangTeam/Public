#include "stdafx.h"
#include "AdjustTask.h"
#include "TinyEncode.h"

using namespace std;

CAdjustTask::CAdjustTask(void)
{
	m_pTelnet = new CSTelnet;

	COleDateTime t(time(NULL));
	CString strTime = t.Format("%Y%m%d%H%M");
	m_strLogFile.Format("2-3G频点方案_%s", strTime);
}


CAdjustTask::~CAdjustTask(void)
{
	delete m_pTelnet;
}

bool CAdjustTask::InitTable()
{
	return LoadBscConfig()&& loadCellConfig()&& UpdateCell();
}

bool CAdjustTask::Process()
{
	if (!load3GInfo())
	{
		return false;
	}

	if (!loadRLUMPTable())
	{
		return false;
	}

	Save2G3GDataTable();

	if (!Meger7DayData())
	{
		return false;
	}

	LoadDrainSet();
	LoadMultiSet();
	LoadUarfcnSize();

	for (int i = 0; i < m_vecBscConf.size(); i++)
	{
		TagBscConf& bscConf = m_vecBscConf[i];
		CString strLog;
		strLog.Format("网元[%s]----------------------多定指令--------------------",bscConf.BscName);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
		GenerateMultiCmd(m_mapMultiSet[bscConf.BscName]);
		strLog.Format("网元[%s]----------------------漏定指令--------------------",bscConf.BscName);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
		GenerateDrainCmd(m_mapDrain[bscConf.BscName],m_mapMultiSet[bscConf.BscName]);

		if (bscConf.nOperation == 2)
		{
			strLog.Format("网元[%s]----------------------等待指令下发--------------------",bscConf.BscName);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
			//等待 下发
			time_t tCurTime = time(NULL);
			struct tm tmTime;
			localtime_s(&tmTime,&tCurTime);
			CString strTime;
			strTime.Format("%d:%d",tmTime.tm_hour,tmTime.tm_min);
			//if (g_bIsTest)strTime = g_pConfig->m_nStartTime;
			while(strTime != g_pConfig->m_nAdjustTime)
			{
				time_t tCurTime = time(NULL);
				struct tm tmTime;
				localtime_s(&tmTime,&tCurTime);
				strTime.Format("%d:%d",tmTime.tm_hour,tmTime.tm_min);
				Sleep(10*1000);
			}
			SendAdjustCmd(bscConf);
		}
	}

	SaveRETToDatabase();
	g_pLog->WriteLog("2G3G结果入库完成",m_strLogFile,LOG_INFO);
	return true;
}

void CAdjustTask::GenerateDrainCmd(std::vector<TagDrain>& data, std::vector<TagMultiSet>& dataMulti)
{
	CString strCmd,strLog;
	
	for (int i = 0; i < data.size(); i++)
	{
		TagDrain &drain = data[i];
		if (drain.UarfcnCpi.IsEmpty() || drain.GsmCell.IsEmpty())
		{
			strLog.Format("cell[%s]频点为空,不生成指令",drain.GsmCell);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
			continue;
		}

		int ncount(0);
		for(int i = 0 ; i < dataMulti.size(); i++)
		{
			if (dataMulti[i].Cell == drain.GsmCell)
			{
				ncount++;
			}
		}

		if (drain.nPoint - ncount >= 8)
		{
			strLog.Format("cell[%s]频点超8个,不生成指令",drain.GsmCell);
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
			continue;
		}
		drain.nPoint++;
		drain.strCmd.Format("RLUMC:CELL=%s,ADD,TMFI=%s-72-NODIV-NODIV;", drain.GsmCell, drain.UarfcnCpi);
		g_pLog->WriteLog(drain.strCmd,m_strLogFile,LOG_INFO);
		//vecCmd.push_back(strCmd);
	}
}

void CAdjustTask::GenerateMultiCmd(std::vector<TagMultiSet>& data)
{
	CString strCmd,strLog;
	
	for (int i = 0; i < data.size(); i++)
	{
		TagMultiSet &multi = data[i];
		if (multi.Tag.IsEmpty() || multi.Cell.IsEmpty())
		{
			strLog.Format("cell[%s]频点为空,不生成指令",multi.Cell);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
			continue;
		}
		multi.strCmd.Format("RLUMC:CELL=%s,REM,TMFI=%s;", multi.Cell, multi.Tag);
		g_pLog->WriteLog(multi.strCmd,m_strLogFile,LOG_INFO);
		//vecCmd.push_back(strCmd);
	}
}

//下发
void CAdjustTask::SendAdjustCmd(TagBscConf& bscConf)
{
	CString strLog;
	strLog.Format("生成网元[%s]调整指令----<下发>", bscConf.BscName);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	//CTelnet telnet;
	//telnet.m_strLogFile = m_strLogFile;
	//telnet.SetTimeout(g_pConfig->m_nRecvTimeout);
	m_pTelnet->m_strLogFile = m_strLogFile;
	//m_pTelnet->SetTimeout(g_pConfig->m_nRecvTimeout);


	int nLoginCount(3);
	bool bRet(true);

	if (bscConf.nOperation == 2)
	{
		while (nLoginCount-- > 0 )
		{
			bRet = m_pTelnet->Login(bscConf.OssIp,bscConf.OssPort,bscConf.OssUser,bscConf.ossPwd,0);

			if(bRet) break;//超时或登录成功，都跳出
			Sleep(1000*10);//10秒后再试
		}
	}

	if (!bRet) return;
	int nCmdCount(0);//成功的指令计数
	
	//登录到BSC eaw
	CString strTemp,strData;
	strTemp.Format("/opt/ericsson/bin/eaw %s", bscConf.BscName);
	strTemp.Replace("R12","");
	m_pTelnet->PushMsg(strTemp);
	m_pTelnet->Send();

	//if (m_pTelnet->Send(strTemp,strData,CHECK_STAT,true) <= 0)
	//{
	//	strLog.Format("网元[%s]登录[%s]失败",bscConf.BscName,strTemp);
	//	g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	//	return;
	//}

	//发送调整指令
	//先删后加
	strLog.Format("网元[%s]----------------------下发多定指令--------------------",bscConf.BscName);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	std::vector<TagMultiSet> & vecMultiSet = m_mapMultiSet[bscConf.BscName];
	for (int i = 0; i < vecMultiSet.size(); i++)
	{
		CString strcmd(vecMultiSet[i].strCmd);
		CString strData("");
		bool isSend(false);
		if (vecMultiSet[i].bAdjust && bscConf.nOperation == 2) isSend = true;
		if (strcmd.IsEmpty()) continue;
		if (isSend)
		{
			m_pTelnet->PushMsg(strcmd);
			m_pTelnet->PushMsg(";");
			m_pTelnet->Send();
			CString strReturn = m_pTelnet->GetTelnetMsgDate(strcmd);
			if (-1!=strReturn.Find("EXECUTED") || -1!=strReturn.Find("END"))
			{
				if(vecMultiSet[i].bAdjust)vecMultiSet[i].bAdjRet = true;
				nCmdCount++;
			}
		}


		//int nRet = m_pTelnet->Send(strcmd,strData,CHECK_STAT,isSend);
		//if (nRet > 0)
		//{
		//	if(vecMultiSet[i].bAdjust)vecMultiSet[i].bAdjRet = true;
		//	nCmdCount++;
		//}
	}

	strLog.Format("网元[%s]----------------------下发漏定指令--------------------",bscConf.BscName);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	std::vector<TagDrain> & vecDrainList = m_mapDrain[bscConf.BscName];
	for (int i = 0; i < vecDrainList.size(); i++)
	{
		CString strcmd(vecDrainList[i].strCmd);
		CString strData("");
		bool isSend(false);
		if (vecDrainList[i].bAdjust && bscConf.nOperation == 2) isSend = true;
		if (strcmd.IsEmpty()) continue;

		if (isSend)
		{
			m_pTelnet->PushMsg(strcmd);
			m_pTelnet->PushMsg(";");
			m_pTelnet->Send();
			CString strReturn = m_pTelnet->GetTelnetMsgDate(strcmd);
			if (-1!=strReturn.Find("EXECUTED") || -1!=strReturn.Find("END"))
			{
				if(vecDrainList[i].bAdjust)vecDrainList[i].bAdjRet = true;
				nCmdCount++;
			}
		}

		//int nRet = m_pTelnet->Send(strcmd,strData,CHECK_STAT,isSend);
		//if (nRet > 0)
		//{
		//	if(vecDrainList[i].bAdjust)vecDrainList[i].bAdjRet = true;
		//	nCmdCount++;
		//}
	}
	//strLog.Format("总计指令【%d】条，成功【%d】条，失败【%d】条指令", vecCmd.size(), nCmdCount, vecCmd.size() - nCmdCount);
	//g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	//退出登录
	m_pTelnet->LogOut();
}

//输出
void CAdjustTask::DumpAdjustCmd(TagBscConf& bscConf, std::vector<CString>& vecCmd)
{
	CString strLog;
	strLog.Format("生成网元【%s】邻区漏配调整指令----<不下发>", bscConf.BscName);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	for (int i = 0; i < vecCmd.size(); i++)
	{
		g_pLog->WriteLog(vecCmd[i] ,LOG_INFO);
	}
}

bool CAdjustTask::CreateEOESData()
{
	g_pLog->WriteLog("[CAdjustTask::CreateEOESData]获取2G数据",m_strLogFile,LOG_INFO);
	CString strSql, strLog;
	//if(g_bIsTest) return true;
	strSql = "if exists (select 1 from dbo.sysObjects where name='tmp_td2gsm')  Drop TABLE tmp_td2gsm";
	if (!g_pDBEOES->Execute(strSql))
	{
		return false;
	}
	g_pLog->WriteLog(strSql,m_strLogFile,LOG_INFO);
	strSql.Format("select a.rnc_cellid,a.cgi,UArfcn,CPI,CONVERT(varchar,Uarfcn)+'-'+CONVERT(varchar,CPI) AS UARFCN_CPI,c.NeType,c.BSC,c.CellID,b.Cell "
		" INTO tmp_td2gsm  "
		" from CDD_TD_RNC_GSMNCELL a "
		" INNER join %s.dbo.cell_td b ON a.rnc_cellid=b.rnc_cellid  "
		" INNER JOIN  %s.dbo.CELL_2G c ON a.cgi=c.CGI  ", g_pConfig->m_strDBNameM, g_pConfig->m_strDBNameM);
	g_pLog->WriteLog(strSql,m_strLogFile,LOG_INFO);
	if (!g_pDBEOES->Execute(strSql))
	{
		return false;
	}
	return true;
}

bool CAdjustTask::LoadBscConfig()
{
	g_pLog->WriteLog("[CAdjustTask::LoadBscConfig]获取网元配置",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;
	strSQL.Format("select bscName, ossIp, ossPort, ossUser, ossPwd ,Operation from PLUGIN_2G_BSC_OSS where Operation in(1,2) ");
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open((LPCTSTR)strSQL);
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		while (!Rs.IsEOF())
		{
			TagBscConf bscConf;
			Rs.GetFieldValue("bscName", bscConf.BscName);
			Rs.GetFieldValue("ossIp",bscConf.OssIp);
			Rs.GetFieldValue("ossPort",bscConf.OssPort);
			Rs.GetFieldValue("ossUser",bscConf.OssUser);
			Rs.GetFieldValue("ossPwd",bscConf.ossPwd);
			Rs.GetFieldValue("Operation",bscConf.nOperation);
			
			CTinyEncode d;
			d.Decode(bscConf.OssUser,bscConf.OssUser);
			d.Decode(bscConf.ossPwd,bscConf.ossPwd);

			m_vecBscConf.push_back(bscConf);
			Rs.MoveNext();
		}
	}
	catch (...)
	{
		strLog.Format("[CAdjustTask::LoadBscConfig] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	return true;
}

bool CAdjustTask::loadCellConfig()
{
	g_pLog->WriteLog("[CAdjustTask::loadCellConfig]获取小区配置",m_strLogFile,LOG_INFO);
	for (int i = 0; i < m_vecBscConf.size(); i++)
	{
		CString strLog;
		CString strSQL;
		TagBscConf& bscConf = m_vecBscConf[i];
		strSQL.Format("select * from PLUGIN_2G_BSC_CELL where BSC_NAME= '%s'",bscConf.BscName);
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		CADORecordset Rs(g_pDBTRX);
		try
		{
			Rs.Open((LPCTSTR)strSQL);
			while (!Rs.IsEOF())
			{
				TagCellConf CellConf;
				CString strIsadj;
				Rs.GetFieldValue("BSC_NAME", CellConf.BscName);
				Rs.GetFieldValue("CELL_ID",CellConf.CellId);
				Rs.GetFieldValue("CELL_NAME",CellConf.CellName);
				Rs.GetFieldValue("IsAdjust",strIsadj);
				Rs.GetFieldValue("NE_TYPE",CellConf.NeType);
				bscConf.mapCellCfg[CellConf.CellName] = CellConf;

				CellConf.bAdjust = atoi(strIsadj);
				Rs.MoveNext();
			}
		}
		catch (...)
		{
			strLog.Format("[CAdjustTask::loadCellConfig] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
			return false;
		}
	}

	return true;
}

bool CAdjustTask::UpdateCell()
{
	g_pLog->WriteLog("[CAdjustTask::UpdateCell]更新小区配置",m_strLogFile,LOG_INFO);
	map<CString,TagCellConf> mapCellDate;

	for (int i = 0; i < m_vecBscConf.size(); i++)
	{
		CString strLog;
		CString strSQL;
		TagBscConf& bscConf = m_vecBscConf[i];

		strSQL.Format("select BSC,CGI,NeType,CellID,Cell from CELL_2G where BSC = '%s'",bscConf.BscName);
		CADORecordset Rs(g_pDBEOESM);
		try
		{
			Rs.Open((LPCTSTR)strSQL);
			while (!Rs.IsEOF())
			{
				TagCellConf CellConf;
				Rs.GetFieldValue("BSC", CellConf.BscName);
				Rs.GetFieldValue("NeType",CellConf.NeType);
				Rs.GetFieldValue("CGI",CellConf.CellCgi);
				Rs.GetFieldValue("CellID",CellConf.CellId);
				Rs.GetFieldValue("Cell",CellConf.CellName);
				CellConf.bAdjust = false;

				if (bscConf.mapCellCfg.end() == bscConf.mapCellCfg.find(CellConf.CellName))
				{
					//mapCellCfg[CellConf.CellName] = CellConf;
					strSQL.Format("insert into PLUGIN_2G_BSC_CELL(BSC_NAME,NE_TYPE,CGI,CELL_ID,CELL_NAME,IsAdjust) \
								  values('%s','%s','%s','%s','%s','%d') ",
								  CellConf.BscName,CellConf.NeType,CellConf.CellCgi,CellConf.CellId,CellConf.CellName,CellConf.bAdjust);
					g_lstSQL.push_back(strSQL);
				}
				Rs.MoveNext();
			}
		}
		catch (...)
		{
			strLog.Format("[CAdjustTask::UpdateCell] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
			return false;
		}
	}

	return true;
}

bool CAdjustTask::loadTGsmRelationTable(map<CString,vector<TagCellGsmRelation>>& mapCellGsmRelation)
{
	g_pLog->WriteLog("[CAdjustTask::loadTGsmRelationTable]获取TGsmRelation配置",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;

	strSQL.Format("select CddDate, Exchid, parentldn, MEID, refTExternalGsmCell from CDD_ZTETD_RNC_TGsmRelation ");
	CADORecordset Rs(g_pDBEOES);
	try
	{
		Rs.Open((LPCTSTR)strSQL);
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		while (!Rs.IsEOF())
		{
			TagCellGsmRelation CellGsmRelation;
			Rs.GetFieldValue("CddDate", CellGsmRelation.strDate);
			Rs.GetFieldValue("Exchid",CellGsmRelation.BscName);
			Rs.GetFieldValue("parentldn",CellGsmRelation.strTUtranCellTDDLcr);
			Rs.GetFieldValue("MEID",CellGsmRelation.strMeid);
			Rs.GetFieldValue("refTExternalGsmCell",CellGsmRelation.strTExternalGsmCell);
			cutstr(CellGsmRelation.strTUtranCellTDDLcr, "TUtranCellTDDLcr=");
			cutstr(CellGsmRelation.strTExternalGsmCell , "TExternalGsmCell=");
			Rs.MoveNext();
			if (CellGsmRelation.strMeid.IsEmpty() || CellGsmRelation.strTUtranCellTDDLcr.IsEmpty()
				|| CellGsmRelation.strTExternalGsmCell.IsEmpty())
			{
				continue;
			}
			mapCellGsmRelation[CellGsmRelation.BscName].push_back(CellGsmRelation);
		}
	}
	catch (...)
	{
		strLog.Format("[CAdjustTask::loadTGsmRelationTable] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	return true;
}

bool CAdjustTask::loadTUtranCellTDDLcrTable(map<CString,vector<TagCellTUtranCellTDDLcr>>& mapCellTUtranCellTDDLcr)
{
	g_pLog->WriteLog("[CAdjustTask::loadTUtranCellTDDLcrTable]获取TUtranCellTDDLcr配置",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;

	strSQL.Format("select CddDate, Exchid, localCellId, MEID, TUtranCellTDDLcr from cdd_ztetd_rnc_TUtranCellTDDLcr ");
	CADORecordset Rs(g_pDBEOES);
	try
	{
		Rs.Open((LPCTSTR)strSQL);
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		while (!Rs.IsEOF())
		{
			TagCellTUtranCellTDDLcr CellTUtranCellTDDLcr;
			Rs.GetFieldValue("CddDate", CellTUtranCellTDDLcr.strDate);
			Rs.GetFieldValue("Exchid",CellTUtranCellTDDLcr.BscName);
			Rs.GetFieldValue("localCellId",CellTUtranCellTDDLcr.str3GCell);
			Rs.GetFieldValue("MEID",CellTUtranCellTDDLcr.strMeid);
			Rs.GetFieldValue("TUtranCellTDDLcr",CellTUtranCellTDDLcr.strTUtranCellTDDLcr);
			Rs.MoveNext();
			if (CellTUtranCellTDDLcr.str3GCell.IsEmpty() || CellTUtranCellTDDLcr.strMeid.IsEmpty()
				|| CellTUtranCellTDDLcr.strTUtranCellTDDLcr.IsEmpty())
			{
				continue;
			}
			mapCellTUtranCellTDDLcr[CellTUtranCellTDDLcr.BscName].push_back(CellTUtranCellTDDLcr);
		}
	}
	catch (...)
	{
		strLog.Format("[CAdjustTask::loadTUtranCellTDDLcrTable] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	return true;
}

bool CAdjustTask::loadTExternalGsmCellTable(map<CString,vector<TagCellTExternalGsmCell>>& mapCellTExternalGsmCell)
{
	g_pLog->WriteLog("[CAdjustTask::loadTExternalGsmCellTable]获取TExternalGsmCel配置",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;

	strSQL.Format("select CddDate, Exchid, TExternalGsmCell, MEID, description from CDD_ZTETD_RNC_TExternalGsmCell ");
	CADORecordset Rs(g_pDBEOES);
	try
	{
		Rs.Open((LPCTSTR)strSQL);
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		while (!Rs.IsEOF())
		{
			TagCellTExternalGsmCell CellTExternalGsmCell;
			Rs.GetFieldValue("CddDate", CellTExternalGsmCell.strDate);
			Rs.GetFieldValue("Exchid",CellTExternalGsmCell.BscName);
			Rs.GetFieldValue("TExternalGsmCell",CellTExternalGsmCell.strTExternalGsmCell);
			Rs.GetFieldValue("MEID",CellTExternalGsmCell.strMeid);
			Rs.GetFieldValue("description",CellTExternalGsmCell.Cgi);
			CellTExternalGsmCell.Cgi.Replace("MCC=","");
			CellTExternalGsmCell.Cgi.Replace(",MNC=","-0");
			CellTExternalGsmCell.Cgi.Replace(",LAC=","-");
			CellTExternalGsmCell.Cgi.Replace(",CI=","-");
			Rs.MoveNext();
			if (CellTExternalGsmCell.strTExternalGsmCell.IsEmpty() || CellTExternalGsmCell.strMeid.IsEmpty()
				|| CellTExternalGsmCell.Cgi.IsEmpty())
			{
				continue;
			}
			mapCellTExternalGsmCell[CellTExternalGsmCell.BscName].push_back(CellTExternalGsmCell);
		}
	}
	catch (...)
	{
		strLog.Format("[CAdjustTask::loadTExternalGsmCellTable] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	return true;
}

bool CAdjustTask::loadTUarfcnTable(map<CString,vector<TagCellUarfcn>>& mapCellUarfcn)
{
	g_pLog->WriteLog("[CAdjustTask::loadTUarfcnTable]获取TUarfcn配置",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;

	strSQL.Format("select CddDate, Exchid, Uarfcn, MEID, parentldn from CDD_ZTETD_RNC_TUarfcn where Masterind = 1");
	CADORecordset Rs(g_pDBEOES);
	try
	{
		Rs.Open((LPCTSTR)strSQL);
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		while (!Rs.IsEOF())
		{
			TagCellUarfcn CellUarfcn;
			CString strUarfcn;
			Rs.GetFieldValue("CddDate", CellUarfcn.strDate);
			Rs.GetFieldValue("Exchid",CellUarfcn.BscName);
			Rs.GetFieldValue("Uarfcn",strUarfcn);
			Rs.GetFieldValue("MEID",CellUarfcn.strMeid);
			Rs.GetFieldValue("parentldn",CellUarfcn.strTUtranCellTDDLcr);
			cutstr(CellUarfcn.strTUtranCellTDDLcr, "TUtranCellTDDLcr=");
			CellUarfcn.nUarfcn = atof(strUarfcn)*5;
			Rs.MoveNext();
			if (CellUarfcn.strMeid.IsEmpty()||strUarfcn.IsEmpty()||CellUarfcn.strTUtranCellTDDLcr.IsEmpty())
			{
				continue;
			}
			mapCellUarfcn[CellUarfcn.BscName].push_back(CellUarfcn);
			
		}
	}
	catch (...)
	{
		strLog.Format("[CAdjustTask::loadTUarfcnTable] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	return true;
}

bool CAdjustTask::loadRLDEPTable(map<CString,TagCellRLDEP>& mapCellRLDEP)
{
	g_pLog->WriteLog("[CAdjustTask::loadRLDEPTable]获取RLDEP配置",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;

	strSQL.Format("select CddDate, Exchid, CELL, cgi from CDD_ERIC2G_BSC_RLDEP  ");
	CADORecordset Rs(g_pDBEOES);
	try
	{
		Rs.Open((LPCTSTR)strSQL);
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		while (!Rs.IsEOF())
		{
			TagCellRLDEP CellRLDEP;
			Rs.GetFieldValue("CddDate", CellRLDEP.strDate);
			Rs.GetFieldValue("Exchid",CellRLDEP.BscName);
			Rs.GetFieldValue("CELL",CellRLDEP.str2GCell);
			Rs.GetFieldValue("cgi",CellRLDEP.Cgi);
			Rs.MoveNext();
			if (CellRLDEP.str2GCell.IsEmpty() || CellRLDEP.Cgi.IsEmpty())
			{
				continue;
			}

			mapCellRLDEP[CellRLDEP.Cgi] = CellRLDEP;
			
		}
	}
	catch (...)
	{
		strLog.Format("[CAdjustTask::loadRLDEPTable] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	return true;
}

bool CAdjustTask::loadRLUMPTable()
{
	g_pLog->WriteLog("[CAdjustTask::loadRLUMPTable]获取RLUMP配置",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;

	strSQL.Format("select CddDate, Exchid, CELL, TMFI from CDD_ERIC2G_BSC_RLUMP where LISTTYPE <> 'ACTIVE' ");
	CADORecordset Rs(g_pDBEOES);
	try
	{
		Rs.Open((LPCTSTR)strSQL);
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		while (!Rs.IsEOF())
		{
			Tag2GCell m2GCell;
			Rs.GetFieldValue("CddDate", m2GCell.strDate);
			Rs.GetFieldValue("Exchid",m2GCell.BscName);
			Rs.GetFieldValue("CELL",m2GCell.str2GCell);
			Rs.GetFieldValue("TMFI",m2GCell.TMFI);
			CString strTemp = m2GCell.TMFI;
			//strTemp.Replace("-72-NODIV-NODIV","");
			m2GCell.mbcch = atoi(cutstr(strTemp,'-'));
			Rs.MoveNext();
			if (m2GCell.str2GCell.IsEmpty() || m2GCell.TMFI.IsEmpty())
			{
				
				continue;
			}
			m_map2GCell[m2GCell.BscName].push_back(m2GCell);
		}
	}
	catch (...)
	{
		strLog.Format("[CAdjustTask::loadRLUMPTable] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	return true;
}

bool CAdjustTask::Meger3gCellInfo(map<CString,vector<TagCellGsmRelation>>& mapCellGsmRelation,
	map<CString,vector<TagCellTUtranCellTDDLcr>>& mapCellTUtranCellTDDLcr,
	map<CString,vector<TagCellTExternalGsmCell>>& mapCellTExternalGsmCell,
	map<CString,vector<TagCellUarfcn>>& mapCellUarfcn,
	map<CString,TagCellRLDEP>& mapCellRLDEP)
{
	g_pLog->WriteLog("[CAdjustTask::Meger3gCellInfo]获取汇总3G-Cell配置",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;

	
	for(map<CString,vector<TagCellGsmRelation>>::iterator item = mapCellGsmRelation.begin(); item != mapCellGsmRelation.end(); item++)
	{
		vector<TagCellGsmRelation> & vecCellGsmRelation = item->second;
		for(int i = 0 ; i < vecCellGsmRelation.size(); i++)
		{
			TagCellGsmRelation & CellGsmRelation = vecCellGsmRelation[i];
			Tag3GCell m3GCell;
			m3GCell.BscName = CellGsmRelation.BscName;

			if (mapCellTUtranCellTDDLcr.find(m3GCell.BscName) != mapCellTUtranCellTDDLcr.end())
			{
				vector<TagCellTUtranCellTDDLcr> & vecTemp = mapCellTUtranCellTDDLcr[m3GCell.BscName];
				for(int i = 0 ; i < vecTemp.size(); i++)
				{
					if (vecTemp[i].strTUtranCellTDDLcr == CellGsmRelation.strTUtranCellTDDLcr
						&&vecTemp[i].strMeid == CellGsmRelation.strMeid
						&& !vecTemp[i].strMeid.IsEmpty() && !vecTemp[i].strTUtranCellTDDLcr.IsEmpty())
					{
						m3GCell.str3GCell = vecTemp[i].str3GCell;
						m3GCell.strMeid = vecTemp[i].strMeid;
					}
				}
			}
			else
			{
				continue;
			}

			if (mapCellTExternalGsmCell.find(m3GCell.BscName) != mapCellTExternalGsmCell.end())
			{
				vector<TagCellTExternalGsmCell> & vecTemp = mapCellTExternalGsmCell[m3GCell.BscName];
				for(int i = 0 ; i < vecTemp.size(); i++)
				{
					if (vecTemp[i].strTExternalGsmCell == CellGsmRelation.strTExternalGsmCell
						&&vecTemp[i].strMeid == CellGsmRelation.strMeid
						&& !vecTemp[i].strMeid.IsEmpty() && !vecTemp[i].strTExternalGsmCell.IsEmpty())
					{
						m3GCell.Cgi = vecTemp[i].Cgi;
					}
				}
			}
			else
			{
				continue;
			}

			if (mapCellUarfcn.find(m3GCell.BscName) != mapCellUarfcn.end())
			{
				vector<TagCellUarfcn> & vecTemp = mapCellUarfcn[m3GCell.BscName];
				for(int i = 0 ; i < vecTemp.size(); i++)
				{
					if (vecTemp[i].strTUtranCellTDDLcr == CellGsmRelation.strTUtranCellTDDLcr
						&&vecTemp[i].strMeid == CellGsmRelation.strMeid)
					{
						m3GCell.nUarfcn = vecTemp[i].nUarfcn;
					}
				}
			}
			else
			{
				continue;
			}

			if (mapCellRLDEP.find(m3GCell.Cgi) != mapCellRLDEP.end())
			{
				m3GCell.vec2GCell = mapCellRLDEP[m3GCell.Cgi].str2GCell;
			}
			else
			{
				continue;
			}

			if (!m3GCell.Cgi.IsEmpty() && !m3GCell.vec2GCell.IsEmpty()
				&& !m3GCell.BscName.IsEmpty() && m3GCell.nUarfcn > 0)
			{
				m_map3GCell[m3GCell.BscName].push_back(m3GCell);
			}
		}
	}
	return true;
}

bool CAdjustTask::load3GInfo()
{
	map<CString,vector<TagCellGsmRelation>> mapCellGsmRelation;
	map<CString,vector<TagCellTUtranCellTDDLcr>> mapCellTUtranCellTDDLcr;
	map<CString,vector<TagCellTExternalGsmCell>> mapCellTExternalGsmCell;
	map<CString,vector<TagCellUarfcn>> mapCellUarfcn;
	map<CString,TagCellRLDEP> mapCellRLDEP;

	if (!loadTGsmRelationTable(mapCellGsmRelation)
		||!loadTUtranCellTDDLcrTable(mapCellTUtranCellTDDLcr)
		||!loadTExternalGsmCellTable(mapCellTExternalGsmCell)
		||!loadTUarfcnTable(mapCellUarfcn)
		||!loadRLDEPTable(mapCellRLDEP))
	{
		return false;
	}

	if (!Meger3gCellInfo(mapCellGsmRelation,mapCellTUtranCellTDDLcr,mapCellTExternalGsmCell,
		mapCellUarfcn,mapCellRLDEP))
	{
		return false;
	}

	return true;
}

bool CAdjustTask::Meger7DayData()
{
	CString strLog;
	CString strSQL;
	time_t tBeginTime = time(NULL);
	struct tm tmTime;
	localtime_s(&tmTime,&tBeginTime);
	COleDateTime coCurTime(tBeginTime-15*60);
	CString strSQLMBCCH,strSQLBusy;
	int nIndex(7);
	CDataCfg mDataCfg;

	vector<CString> vecTable;
	for (int i = 1; i <= nIndex; i++)  //获取天表
	{
		CString strYMD = TimeFormat(coCurTime,"%Y%m%d",0-i);
		strYMD.Delete(0,2);//保留6位;
		CString strTable = "PLUGIN_2G_CDD_2G_3G_MBCCH_" + strYMD;
		map<CString,int> mapData;
		mDataCfg.GetTableHead(strTable,mapData);
		if (mapData.size() != 0)
		{
			vecTable.push_back(strTable);
		}
	}

	//如果天表为空 退出
	if (vecTable.size() < 3)
	{
		g_pLog->WriteLog("[CAdjustTask::Meger7DayData]2G_3G_MBCCH少于3天数据 获取数据失败",m_strLogFile,LOG_ERROR);
		return false;
	}

	CString strRepleace;
	for (int i = 0; i < vecTable.size(); i++)
	{
		CString strSQLtemp;
		strSQLtemp.Format(" UNION SELECT BSC_NAME,NE_TYPE,CELL,MBCCH,TMFI,REMARK FROM %s",vecTable[i]);
		strRepleace += strSQLtemp;
	}
	strRepleace.Delete(0,6);
	CString strYMD = TimeFormat(coCurTime,"%Y%m%d",0);
	strYMD.Delete(0,2);
	strSQLMBCCH.Format("select * into PLUGIN_2G_CDD_2G_3G_MBCCH_MEGER_%s from( %s ) A",strYMD,strRepleace);
	g_pLog->WriteLog(strSQLMBCCH,m_strLogFile,LOG_INFO);
	if(!g_pDBTRX->Execute(strSQLMBCCH))
	{
		strLog.Format("[CAdjustTask::Meger7DayData] SQL 执行出错:%s\n%s",strSQLMBCCH,g_pDBTRX->GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	vecTable.clear();
	for (int i = 1; i <= nIndex; i++)  //获取天表
	{
		CString strYMD = TimeFormat(coCurTime,"%Y%m%d",0-i);
		strYMD.Delete(0,2);//保留6位;
		CString strTable = "PLUGIN_2G_CDD_3G_2G_NCELL_" + strYMD;
		map<CString,int> mapData;
		mDataCfg.GetTableHead(strTable,mapData);
		if (mapData.size() != 0)
		{
			vecTable.push_back(strTable);
		}
	}

	//如果天表为空 退出
	if (vecTable.size() < 3)
	{
		g_pLog->WriteLog("[CAdjustTask::Meger7DayData]3G_2G_NCELL少于3天数据 获取数据失败",m_strLogFile,LOG_ERROR);
		return false;
	}

	strRepleace.Empty();
	for (int i = 0; i < vecTable.size(); i++)
	{
		CString strSQLtemp;
		strSQLtemp.Format(" UNION SELECT BSC_NAME,NE_TYPE,MEID,CELLID,CELL,ARFCN,CGI,REMARK FROM %s",vecTable[i]);
		strRepleace += strSQLtemp;
	}
	strRepleace.Delete(0,6);
	strYMD = TimeFormat(coCurTime,"%Y%m%d",0);
	strYMD.Delete(0,2);
	strSQLMBCCH.Format("select * into PLUGIN_2G_CDD_3G_2G_NCELL_MEGER_%s from( %s ) A",strYMD,strRepleace);
	g_pLog->WriteLog(strSQLMBCCH,m_strLogFile,LOG_INFO);
	if(!g_pDBTRX->Execute(strSQLMBCCH))
	{
		strLog.Format("[CAdjustTask::Meger7DayData] SQL 执行出错:%s\n%s",strSQLMBCCH,g_pDBTRX->GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

		return true;
}

void CAdjustTask::LoadDrainSet()
{
	g_pLog->WriteLog("[CAdjustTask::LoadDrainSet]获取漏定数据",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;
	COleDateTime t(time(NULL));
	CString strTime = t.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString str3gTable = "PLUGIN_2G_CDD_3G_2G_NCELL_MEGER_" + strTime;
	CString str2gTable = "PLUGIN_2G_CDD_2G_3G_MBCCH_MEGER_" + strTime;

	strSQL.Format("select a.BSC_NAME,a.NE_TYPE,b.CELL,b.ARFCN,b.cgi FROM "
		"(select distinct BSC_NAME,CELL,NE_TYPE FROM %s) a,"
		"(SELECT distinct a.ARFCN,a.CELL,a.cgi "
		"FROM %s a  "
		"LEFT JOIN %s b ON a.cell=b.cell AND a.ARFCN=b.MBCCH "
		"WHERE  b.Cell IS null) b where a.CELL=b.CELL",str2gTable,str3gTable,str2gTable);
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open((LPCTSTR)strSQL);
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		while (!Rs.IsEOF())
		{
			TagDrain drain;
			Rs.GetFieldValue("BSC_NAME", drain.BscName);
			Rs.GetFieldValue("CGI",drain.Cgi);
			Rs.GetFieldValue("NE_TYPE",drain.NeType);
			//Rs.GetFieldValue("cellid",drain.RncCellId);
			//Rs.GetFieldValue("cellid",drain.Cell);
			Rs.GetFieldValue("ARFCN",drain.UarfcnCpi);
			Rs.GetFieldValue("CELL",drain.GsmCell);

			drain.bAdjust = false;
			for (int i = 0; i < m_vecBscConf.size(); i++)
			{
				TagBscConf& bscConf = m_vecBscConf[i];
				if (bscConf.BscName == drain.BscName)
				{			
					if (bscConf.nOperation == 2)
					{
						//std::map<CString,TagCellConf>& mapCell = bscConf.mapCellCfg;
						std::map<CString,TagCellConf>::iterator it = bscConf.mapCellCfg.find(drain.GsmCell);
						if (it != bscConf.mapCellCfg.end() && it->second.bAdjust)
						{
							drain.bAdjust = true;
						}
					} 
				}
			}

			if (m_mapDrain.find(drain.BscName) != m_mapDrain.end())
			{
				vector<TagDrain> & vecTemp = m_mapDrain[drain.BscName];
				for(int i = 0 ; i < vecTemp.size(); i++)
				{
					if (vecTemp[i].GsmCell == drain.GsmCell
						&&vecTemp[i].UarfcnCpi == drain.UarfcnCpi)
					{
						vecTemp[i].RncCellId +="," + drain.RncCellId;
						vecTemp[i].Cell +="," + drain.Cell;
						continue;
					}

					if (i == vecTemp.size()-1)
					{
						m_mapDrain[drain.BscName].push_back(drain);
					}
				}
			}
			else
			{
				m_mapDrain[drain.BscName].push_back(drain);
			}
			Rs.MoveNext();
		}
	}
	catch (...)
	{
		strLog.Format("[CKernel::LoadPYFile] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	}
}

void CAdjustTask::LoadMultiSet()
{
	g_pLog->WriteLog("[CAdjustTask::LoadMultiSet]获取多定数据",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;
	COleDateTime t(time(NULL));
	CString strTime = t.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString str3gTable = "PLUGIN_2G_CDD_3G_2G_NCELL_MEGER_" + strTime;
	CString str2gTable = "PLUGIN_2G_CDD_2G_3G_MBCCH_MEGER_" + strTime;

	strSQL.Format("select  b.BSC_NAME,b.NE_TYPE,b.CELL,b.MBCCH,b.TMFI,a.cgi FROM "
		"(select distinct CELL,cgi FROM %s) a, "
		"(SELECT distinct a.BSC_NAME,a.NE_TYPE,a.CELL,a.MBCCH,a.TMFI "
		"FROM %s a   "
		"LEFT JOIN %s b ON a.cell=b.cell AND b.ARFCN=a.MBCCH "
		"WHERE  b.Cell IS null) b where a.CELL=b.CELL",str3gTable,str2gTable,str3gTable);
	CADORecordset Rs(g_pDBTRX);
	try
	{
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		Rs.Open((LPCTSTR)strSQL);
		while (!Rs.IsEOF())
		{
			TagMultiSet multi;
			Rs.GetFieldValue("BSC_NAME", multi.Exchid);
			//Rs.GetFieldValue("cellid",multi.RncCellId);
			Rs.GetFieldValue("CGI",multi.Cgi);
			Rs.GetFieldValue("Cell",multi.Cell);
			Rs.GetFieldValue("MBCCH",multi.MBCCHNO);
			Rs.GetFieldValue("NE_TYPE",multi.NeType);
			Rs.GetFieldValue("TMFI",multi.Tag);
			multi.bAdjust = false;

			for (int i = 0; i < m_vecBscConf.size(); i++)
			{
				TagBscConf& bscConf = m_vecBscConf[i];
				if (bscConf.BscName == multi.Exchid)
				{			
					if (bscConf.nOperation == 2)
					{
						//std::map<CString,TagCellConf>& mapCell = bscConf.mapCellCfg;
						std::map<CString,TagCellConf>::iterator it = bscConf.mapCellCfg.find(multi.Cell);
						if (it != bscConf.mapCellCfg.end() && it->second.bAdjust)
						{
							multi.bAdjust = true;
						}
					} 
				}
			}

			if (m_mapMultiSet.find(multi.Exchid) != m_mapMultiSet.end())
			{
				vector<TagMultiSet> & vecTemp = m_mapMultiSet[multi.Exchid];
				for(int i = 0 ; i < vecTemp.size(); i++)
				{
					if (vecTemp[i].Cell == multi.Cell
						&&vecTemp[i].Tag == multi.Tag)
					{
						vecTemp[i].RncCellId +="," + multi.RncCellId;
						continue;
					}

					if (i == vecTemp.size()-1)
					{
						m_mapMultiSet[multi.Exchid].push_back(multi);
					}
				}
			}
			else
			{
				m_mapMultiSet[multi.Exchid].push_back(multi);
			}
			Rs.MoveNext();
		}
	}
	catch (...)
	{
		strLog.Format("[CKernel::LoadPYFile] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	}
}

void CAdjustTask::LoadUarfcnSize()
{
	g_pLog->WriteLog("[CAdjustTask::LoadMultiSet]获取频点数据",m_strLogFile,LOG_INFO);
	CString strLog;
	CString strSQL;
	COleDateTime t(time(NULL));
	CString strTime = t.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString str2gTable = "PLUGIN_2G_CDD_2G_3G_MBCCH_MEGER_" + strTime;
	strSQL.Format(" select a.bsc_name,a.cell ,count(*) as PointSize from (select distinct tmfi,bsc_name, cell from "
					" %s "
					"group by cell,tmfi,bsc_name) a group by a.cell,a.bsc_name  ",str2gTable);
	CADORecordset Rs(g_pDBTRX);
	try
	{
		g_pLog->WriteLog(strSQL,m_strLogFile,LOG_INFO);
		Rs.Open((LPCTSTR)strSQL);
		while (!Rs.IsEOF())
		{
			TagDrain Drain;
			Rs.GetFieldValue("BSC_NAME", Drain.BscName);
			Rs.GetFieldValue("Cell",Drain.Cell);
			Rs.GetFieldValue("PointSize",Drain.nPoint);

			if (m_mapDrain.find(Drain.BscName) != m_mapDrain.end())
			{
				vector<TagDrain> & vecTemp = m_mapDrain[Drain.BscName];
				for(int i = 0 ; i < vecTemp.size(); i++)
				{
					if (vecTemp[i].GsmCell == Drain.Cell)
					{
						vecTemp[i].nPoint = Drain.nPoint;
					}
				}
			}

			Rs.MoveNext();
		}
	}
	catch (...)
	{
		strLog.Format("[CKernel::LoadPYFile] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	}
}

void CAdjustTask::SaveAdjustCmd(CString& strDate, CString& strCmd, int nRes)
{
	CString strSql;
	strSql.Format("Insert into PLUGIN_ZTE_LTE_CHECK_CMD(adjustDate, adjustCmd, adjustResult) values('%s', '%s', %d);", strDate, strCmd, nRes);
	g_lstSQL.push_back(strSql);
}

bool CAdjustTask::CheckCmd()
{
	if (g_bIsTest) return false;

	CString strLog,strSQL,strYmd,strTime;
	COleDateTime t(time(NULL));
	strTime = t.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString strTable = "PLUGIN_2G_BSC_CELL_RET_" + strTime;
	strSQL.Format("select  top 1 * from %s ",strTable, t.Format("%Y%m%d"));
	CADORecordset Rs(g_pDBTRX);
	try
	{
		Rs.Open((LPCTSTR)strSQL);
		if (!Rs.IsEOF())
		{
			return true;
		}
	}
	catch (...)
	{
		//strLog.Format("[CKernel::CheckCmd] 数据库查询出错:%s\n%s",strSQL,Rs.GetLastError());
		//g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	return true;
}

void CAdjustTask::MakeLine(map<CString,int>& mapField,vector<CString>& vecData,CString& strSQL,CString strName,CString strType,CString strFormat,...)
{
	//格式化字符串
	CString strValue;
	va_list argList;
	va_start(argList,strFormat);
	strValue.FormatV(strFormat, argList);
	va_end(argList);
	//装入行容器
	while (!strName.IsEmpty())
	{
		CString sName = cutstr(strName,',');
		CString sType = cutstr(strType,',');
		//CString sValue = cutstr(strValue,',');
		CString sValue = strValue;
		map<CString,int>::iterator it = mapField.find(sName);
		if (it != mapField.end())
		{
			vecData[it->second] = sValue;
			continue;
		}
		strSQL += sName;
		strSQL += " ";
		strSQL += sType;
		strSQL += " NULL,";

		mapField.insert(make_pair(sName,mapField.size()));
		vecData.push_back(sValue);
	}
}

bool CAdjustTask::InportFile(CString strName,vector<vector<CString>>& vecDatas,CString& strSQL,bool bHandle)
{
	CString strLog;
	if (!strSQL.IsEmpty())
	{	
		strSQL.Delete(strSQL.GetLength()-1,1);
		CString strEXEC;
		if (bHandle)
		{
			strEXEC.Format("alter table %s add %s",strName,strSQL);
		}
		else
		{
			strEXEC.Format("create table %s (%s)",strName,strSQL);
		}
		if (!g_pDBTRX->Execute(strEXEC))
		{
			strLog.Format("[CDispose::InportFile] SQL 执行错误:%s\n%s",strSQL,g_pDBTRX->GetLastError());
			g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
			return false;
		}
	}

	CString strData;
	for (int i = 0; i < (int)vecDatas.size(); i++)
	{
		vector<CString>& vecData = vecDatas[i];
		for (int n = 0; n < (int)vecData.size(); n++)
		{
			strData += vecData[n];
			strData += "\t";
		}
		strData.Delete(strData.GetLength()-1,1);
		strData += "\r\n";
	}
	if (strData.IsEmpty())
	{
		g_pLog->WriteLog("[CDispose::InportFile] 导入内容为空",m_strLogFile,LOG_ERROR);
		return false;
	}

	//创建目录
	COleDateTime t(time(NULL));
	CString strYMD = t.Format("%Y%m%d");
	CString strDir = g_strFileDir + "\\" + strYMD + "\\";
	CreateDirectory(strDir, NULL);
	strDir = strDir + "2-3G频点方案" + "\\";
	CreateDirectory(strDir, NULL);

	CString strFilePath = strDir + strName + TimeFormat(time(NULL),"%H%M%S.txt");
	CMyFile mFile;
	if(0 != mFile.Write(strFilePath,CFile::modeCreate|CFile::modeWrite,strData,strData.GetLength()))
	{
		strLog.Format("[CDispose::InportFile] 创建文件出错: %s",strFilePath);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	int nRet = g_pDBTRX->Inport(strName,strFilePath,true);
	if (0 != nRet)
	{
		strLog.Format("[CDispose::InportFile] 导入文件失败,错误信息:%s",g_pDBTRX->GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	strLog.Format("[CDispose::InportFile] 导入文件成功: %s",strFilePath);
	g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

	//删除临时文件
	CMyFile::Remove(strFilePath);
	return true;
}

void CAdjustTask::Save2G3GDataTable(void)
{
	CString strSql,str3GCELLInfo,str2GCELLInfo;
	COleDateTime t(time(NULL));
	CString strTime = t.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString str3gTable = "PLUGIN_2G_CDD_3G_2G_NCELL_" + strTime;
	CString str2gTable = "PLUGIN_2G_CDD_2G_3G_MBCCH_" + strTime;
	map<CString,int> map3GCellInfo,map2GCellInfo;
	vector<vector<CString>> vec3GCELLInfos,vec2GCELLInfos;
	//if (!Create2G3GDataTable())
	//{
	//	g_pLog->WriteLog("创建2G3G表失败",m_strLogFile,LOG_ERROR);
	//	return;
	//}
	CDataCfg mDataCfg;
	mDataCfg.GetTableHead(str3gTable,map3GCellInfo);
	mDataCfg.GetTableHead(str2gTable,map2GCellInfo);
	bool b3gCELLRET(true),b2gCELLRET(true);
	if (map3GCellInfo.size() == 0)
	{
		b3gCELLRET = false;
	}
	if (map2GCellInfo.size() == 0)
	{
		b2gCELLRET = false;
	}

	for (std::map<CString, std::vector<Tag3GCell> >::iterator it = m_map3GCell.begin(); it != m_map3GCell.end(); it++)
	{
		std::vector<Tag3GCell> & temVec3GCell = it->second;

		for(int i=0; i < temVec3GCell.size(); i++)
		{
			Tag3GCell &tem3GCell = temVec3GCell[i];
			vector<CString> vec3GCellInfo;
			for (map<CString,int>::iterator it = map3GCellInfo.begin(); it != map3GCellInfo.end(); it++)
			{
				vec3GCellInfo.push_back("");
			}
			MakeLine(map3GCellInfo,vec3GCellInfo,str3GCELLInfo,"DATE","varchar(10)","%s",t.Format("%Y%m%d"));
			MakeLine(map3GCellInfo,vec3GCellInfo,str3GCELLInfo,"BSC_NAME","varchar(64)","%s",tem3GCell.BscName);
			MakeLine(map3GCellInfo,vec3GCellInfo,str3GCELLInfo,"NE_TYPE","varchar(64)","%s",tem3GCell.strNeType);
			MakeLine(map3GCellInfo,vec3GCellInfo,str3GCELLInfo,"MEID","varchar(255)","%s",tem3GCell.strMeid);
			MakeLine(map3GCellInfo,vec3GCellInfo,str3GCELLInfo,"CEllID","varchar(255)","%s",tem3GCell.str3GCell);
			MakeLine(map3GCellInfo,vec3GCellInfo,str3GCELLInfo,"ARFCN","varchar(64)","%d",tem3GCell.nUarfcn);
			MakeLine(map3GCellInfo,vec3GCellInfo,str3GCELLInfo,"CGI","varchar(255)","%s",tem3GCell.Cgi);
			MakeLine(map3GCellInfo,vec3GCellInfo,str3GCELLInfo,"CEll","varchar(64)","%s",tem3GCell.vec2GCell);
			MakeLine(map3GCellInfo,vec3GCellInfo,str3GCELLInfo,"REMARK","varchar(64)","%s","3G定义2G邻区关系");
			vec3GCELLInfos.push_back(vec3GCellInfo);
		}
	}
	InportFile(str3gTable,vec3GCELLInfos,str3GCELLInfo,b3gCELLRET);

	for (std::map<CString, std::vector<Tag2GCell> >::iterator it = m_map2GCell.begin(); it != m_map2GCell.end(); it++)
	{
		std::vector<Tag2GCell> & temVec2GCell = it->second;

		for(int i=0; i < temVec2GCell.size(); i++)
		{
			Tag2GCell &tem2GCell = temVec2GCell[i];
			if (tem2GCell.BscName.IsEmpty() || tem2GCell.str2GCell.IsEmpty()
				||tem2GCell.TMFI.IsEmpty())
			{
				continue;
			}

			vector<CString> vec2GCellInfo;
			for (map<CString,int>::iterator it = map2GCellInfo.begin(); it != map2GCellInfo.end(); it++)
			{
				vec2GCellInfo.push_back("");
			}
			MakeLine(map2GCellInfo,vec2GCellInfo,str2GCELLInfo,"DATE","varchar(10)","%s",t.Format("%Y%m%d"));
			MakeLine(map2GCellInfo,vec2GCellInfo,str2GCELLInfo,"BSC_NAME","varchar(64)","%s",tem2GCell.BscName);
			MakeLine(map2GCellInfo,vec2GCellInfo,str2GCELLInfo,"NE_TYPE","varchar(64)","%s",tem2GCell.strNeType);
			MakeLine(map2GCellInfo,vec2GCellInfo,str2GCELLInfo,"CELL","varchar(255)","%s",tem2GCell.str2GCell);
			MakeLine(map2GCellInfo,vec2GCellInfo,str2GCELLInfo,"MBCCH","varchar(10)","%d",tem2GCell.mbcch);
			MakeLine(map2GCellInfo,vec2GCellInfo,str2GCELLInfo,"TMFI","varchar(64)","%s",tem2GCell.TMFI);
			MakeLine(map2GCellInfo,vec2GCellInfo,str2GCELLInfo,"REMARK","varchar(64)","%s","2G定义3G测量频点");
			vec2GCELLInfos.push_back(vec2GCellInfo);
		}				
	}

	InportFile(str2gTable,vec2GCELLInfos,str2GCELLInfo,b2gCELLRET);
}

void CAdjustTask::SaveRETToDatabase(void)
{
	CString strSql;
	COleDateTime t(time(NULL));
	CString strTime = t.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString strTable = "PLUGIN_2G_BSC_CELL_RET_" + strTime;
	if (!CreateRetTable())
	{
		g_pLog->WriteLog("创建Ret表失败",m_strLogFile,LOG_ERROR);
	}
	//bool bCELLRET = true;
	//CString strCELLRET;
	//map<CString,int> mapCELLRET;
	//CDataCfg mDataCfg;
	//mDataCfg.GetTableHead(strTable,mapCELLRET);
	int nID(1);
	//vector<vector<CString>> vecCELLRETs;
	//if (mapCELLRET.size() == 0)
	//{
	//	bCELLRET = false;
	//}

	for (std::map<CString, std::vector<TagDrain> >::iterator it = m_mapDrain.begin(); it != m_mapDrain.end(); it++)
	{
		std::vector<TagDrain> & temVecDrain = it->second;

		for(int i=0; i < temVecDrain.size(); i++,nID++)
		{
			TagDrain &temDrain = temVecDrain[i];
			vector<CString> vecCELLRET;
			//for (map<CString,int>::iterator it = mapCELLRET.begin(); it != mapCELLRET.end(); it++)
			//{
			//	vecCELLRET.push_back("");
			//}
			////MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"ID","int","%d",nID);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"DATE","varchar(10)","%s",strTable,t.Format("%Y%m%d"));
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"BSC_NAME","varchar(64)","%s",temDrain.BscName);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"NE_TYPE","varchar(64)","%s",temDrain.NeType);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"Cgi","varchar(255)","%s",temDrain.Cgi);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"CEllID","varchar(255)","%s",temDrain.Cell);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"CEll","varchar(255)","%s",temDrain.Cell);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"RNC_CellID","varchar(MAX)","%s",temDrain.RncCellId);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"UARFCN","varchar(64)","%s",temDrain.UarfcnCpi);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"AdjCmd","varchar(1024)","%s",temDrain.strCmd);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"IS_Adjust","int","%d",temDrain.bAdjust);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"ADJ_RET","int","%d",temDrain.bAdjRet);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"REMARK","varchar(64)","%s","漏定");
			//vecCELLRETs.push_back(vecCELLRET);

			strSql.Format("Insert into %s(DATE,BSC_NAME,RNC_CellID,NE_TYPE,Cgi,CEllID,CEll,UARFCN,AdjCmd,IS_Adjust,ADJ_RET,REMARK) \
					values('%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,'%d','%s');"
				,strTable,t.Format("%Y%m%d"),temDrain.BscName,temDrain.RncCellId,temDrain.NeType,temDrain.Cgi,temDrain.Cell,
				temDrain.GsmCell,temDrain.UarfcnCpi,temDrain.strCmd,temDrain.bAdjust,temDrain.bAdjRet,"漏定");

			if(!g_pDBTRX->Execute(strSql))
			{
				CString strLog;
				strLog.Format("[CAdjustTask::SaveRETToDatabase] SQL 执行出错:%s\n%s",strSql,g_pDBTRX->GetLastError());
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
			}
		}	
	}

	for (std::map<CString, std::vector<TagMultiSet> >::iterator it = m_mapMultiSet.begin(); it != m_mapMultiSet.end(); it++)
	{
		std::vector<TagMultiSet> & temVecMultiSet = it->second;
		for(int i=0; i < temVecMultiSet.size(); i++,nID++)
		{
			TagMultiSet &temMultiSet = temVecMultiSet[i];

			//vector<CString> vecCELLRET;
			//for (map<CString,int>::iterator it = mapCELLRET.begin(); it != mapCELLRET.end(); it++)
			//{
			//	vecCELLRET.push_back("");
			//}
			////MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"ID","int","%d",nID);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"DATE","varchar(10)","%s",strTable,t.Format("%Y%m%d"));
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"BSC_NAME","varchar(64)","%s",temMultiSet.Exchid);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"NE_TYPE","varchar(64)","%s",temMultiSet.NeType);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"Cgi","varchar(255)","%s",temMultiSet.Cgi);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"CEllID","varchar(255)","%s",temMultiSet.Cell);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"CEll","varchar(255)","%s",temMultiSet.Cell);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"RNC_CellID","varchar(MAX)","%s",temMultiSet.RncCellId);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"UARFCN","varchar(64)","%s",temMultiSet.MBCCHNO);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"AdjCmd","varchar(1024)","%s",temMultiSet.strCmd);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"IS_Adjust","int","%d",temMultiSet.bAdjust);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"ADJ_RET","int","%d",temMultiSet.bAdjRet);
			//MakeLine(mapCELLRET,vecCELLRET,strCELLRET,"REMARK","varchar(64)","%s","多定");
			//vecCELLRETs.push_back(vecCELLRET);

			strSql.Format("Insert into %s(DATE,BSC_NAME,RNC_CellID,NE_TYPE,Cgi,CEllID,CEll,UARFCN,AdjCmd,IS_Adjust,ADJ_RET,REMARK) \
						  values('%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,'%d','%s');"
						  ,strTable,t.Format("%Y%m%d"),temMultiSet.Exchid,temMultiSet.RncCellId,temMultiSet.NeType,temMultiSet.Cgi,temMultiSet.Cell,
						  temMultiSet.Cell,temMultiSet.MBCCHNO,temMultiSet.strCmd,temMultiSet.bAdjust,temMultiSet.bAdjRet,"多定");

			if(!g_pDBTRX->Execute(strSql))
			{
				CString strLog;
				strLog.Format("[CAdjustTask::SaveRETToDatabase] SQL 执行出错:%s\n%s",strSql,g_pDBTRX->GetLastError());
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
			}
		}
	}

	////写入文件再导入数据库
	//InportFile(strTable,vecCELLRETs,strCELLRET,bCELLRET);
}

bool CAdjustTask::CreateRetTable(void)
{
	COleDateTime t(time(NULL));
	CString strTime = t.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString strTable = "PLUGIN_2G_BSC_CELL_RET_" + strTime;
	CString strSQL("");

	strSQL.Format("\
				  if not exists (select * from sysobjects where [name] = '%s' and xtype='U')\r\n\
				  begin\r\n\
				  SET ANSI_NULLS ON\r\n\
				  SET QUOTED_IDENTIFIER ON\r\n\
				  SET ANSI_PADDING ON\r\n\
				  CREATE TABLE [dbo].[%s](\r\n\
				  [ID] [int] IDENTITY(1,1) NOT NULL,\r\n\
				  [DATE] [varchar](10) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [BSC_NAME] [varchar](64) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [NE_TYPE] [varchar](64),\r\n\
				  [Cgi] [varchar](255) COLLATE Chinese_PRC_CI_AS ,\r\n\
				  [CEllID] [varchar](255) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [CEll] [varchar](255) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [RNC_CellID] [varchar](MAX) NOT NULL,\r\n\
				  [UARFCN] [varchar](64) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [AdjCmd] [varchar](1024) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [IS_Adjust] [int] ,\r\n\
				  [ADJ_RET] [varchar](64) COLLATE Chinese_PRC_CI_AS, \r\n\
				  [REMARK] [varchar](64) COLLATE Chinese_PRC_CI_AS )\r\n\
				  SET ANSI_PADDING OFF\r\n\
				  end;\r\n\
				  ",strTable,strTable,strTable);

	if(!g_pDBTRX->Execute(strSQL))
	{
		CString strLog;
		strLog.Format("[CAdjustTask::CreateRetTable] SQL 执行出错:%s\n%s",strSQL,g_pDBTRX->GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	return true;
}

bool CAdjustTask::Create2G3GDataTable(void)
{
	COleDateTime t(time(NULL));
	CString strTime = t.Format("%Y%m%d");
	strTime.Delete(0,2);
	CString strTable = "PLUGIN_2G_CDD_3G_2G_NCELL_" + strTime;
	CString strSQL("");

	strSQL.Format("\
				  if not exists (select * from sysobjects where [name] = '%s' and xtype='U')\r\n\
				  begin\r\n\
				  SET ANSI_NULLS ON\r\n\
				  SET QUOTED_IDENTIFIER ON\r\n\
				  SET ANSI_PADDING ON\r\n\
				  CREATE TABLE [dbo].[%s](\r\n\
				  [ID] [int] IDENTITY(1,1) NOT NULL,\r\n\
				  [DATE] [varchar](10) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [BSC_NAME] [varchar](64) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [NE_TYPE] [varchar](64),\r\n\
				  [MEID] [varchar](255) COLLATE Chinese_PRC_CI_AS ,\r\n\
				  [CEllID] [varchar](255) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [ARFCN] [varchar](10) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [CGI] [varchar](255) NOT NULL,\r\n\
				  [CELL] [varchar](64) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [REMARK] [varchar](64) COLLATE Chinese_PRC_CI_AS )\r\n\
				  SET ANSI_PADDING OFF\r\n\
				  end;\r\n\
				  ",strTable,strTable,strTable);

	if(!g_pDBTRX->Execute(strSQL))
	{
		CString strLog;
		strLog.Format("[CAdjustTask::Create3GDataTable] SQL 执行出错:%s\n%s",strSQL,g_pDBTRX->GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}

	strTable = "PLUGIN_2G_CDD_2G_3G_MBCCH_" + strTime;
	strSQL.Format("\
				  if not exists (select * from sysobjects where [name] = '%s' and xtype='U')\r\n\
				  begin\r\n\
				  SET ANSI_NULLS ON\r\n\
				  SET QUOTED_IDENTIFIER ON\r\n\
				  SET ANSI_PADDING ON\r\n\
				  CREATE TABLE [dbo].[%s](\r\n\
				  [ID] [int] IDENTITY(1,1) NOT NULL,\r\n\
				  [DATE] [varchar](10) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [BSC_NAME] [varchar](64) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [NE_TYPE] [varchar](64),\r\n\
				  [CEll] [varchar](255) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [MBCCH] [varchar](10) NOT NULL,\r\n\
				  [TMFI] [varchar](64) COLLATE Chinese_PRC_CI_AS NOT NULL,\r\n\
				  [REMARK] [varchar](64) COLLATE Chinese_PRC_CI_AS )\r\n\
				  SET ANSI_PADDING OFF\r\n\
				  end;\r\n\
				  ",strTable,strTable,strTable);

	if(!g_pDBTRX->Execute(strSQL))
	{
		CString strLog;
		strLog.Format("[CAdjustTask::Create2GDataTable] SQL 执行出错:%s\n%s",strSQL,g_pDBTRX->GetLastError());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
		return false;
	}
	return true;
}