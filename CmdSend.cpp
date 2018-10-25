#include "CmdSend.h"
#include "StdAfx.h"
#include "Kernel.h"

CCmdSend::CCmdSend(map<CString,CENB*>& mapData):m_nActionStatus(-1),m_mapENBObj(mapData)
{
	//m_tTime = time(NULL);
	//m_pXFtp = new CXFtp;
	m_pOMMBObj = new COMMB;
	m_pTelnet = new CTelnet;

	//��ʹ��������
	::InitializeCriticalSection(&m_csLock);
}

CCmdSend::CCmdSend(void):m_nActionStatus(-1)
{
	//m_tTime = time(NULL);
	//m_pXFtp = new CXFtp;
	m_pOMMBObj = new COMMB;
	m_pTelnet = new CTelnet;

	//��ʹ��������
	::InitializeCriticalSection(&m_csLock);
}

CCmdSend::~CCmdSend(void)
{
	delete m_pOMMBObj;
	delete m_pTelnet;

	//for (map<CString,CENB*>::iterator it = m_mapENBObj.begin(); it != m_mapENBObj.end(); it++)
	//{
	//	delete it->second;
	//}

	//�رջ�����
	::DeleteCriticalSection(&m_csLock);

	//g_pLog->WriteLog("[CCmdSend::~CCmdSend] �����ͷ�",m_strLogFile,LOG_INFO);
}

void CCmdSend::ProcessJob(CWorkDesc* pJob)
{
	//COleDateTime t(m_tTime);
	//CString strTime = t.Format("%Y%m%d%H%M");
	//m_strLogFile.Format("%s_%s",m_pOMMBCfg->strName, strTime);

	g_pLog->WriteLog("[CCmdSend::ProcessJob] ������ʼ\n",m_strLogFile,LOG_INFO);


	try
	{
		SendAdjustCmd();
		g_pLog->WriteLog("[CCmdSend::ProcessJob] ���͵���ָ�����\n",m_strLogFile,LOG_INFO);
		SetActionStatus(ACTION_STATUS_SUCCESS);

	}
	catch(const CADOException& e)
	{
		CString strLog;
		strLog.Format("[CCmdSend::ProcessJob] ���ݿ������׳����� %s \n",e.GetErrorMessage());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	}
	catch (const std::exception& e)
	{
		CString strLog;
		strLog.Format("[CCmdSend::ProcessJob] C++�׳����� %s \n",e.what());
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
	}
	catch (...)
	{
		g_pLog->WriteLog("[CCmdSend::ProcessJob] δ֪���� ...\n",m_strLogFile,LOG_ERROR);

	}

}


void CCmdSend::SendAdjustCmd()//����ָ��
{
	m_pTelnet->m_strLogFile = m_strLogFile;
	m_pTelnet->m_strThreadName = m_strThread;
	m_pTelnet->SetTimeout(g_pConfig->m_nRecvTimeout);

	//time_t tCur = time(NULL);//��ǰʱ��
	//time_t tRes = (g_pConfig->m_nAdjustCycle*60 - (tCur-m_tTime)) / 3;//������ʣ��ʱ���2��1

	if (m_pOMMBCfg->nOperation == 2)
	{
		//��¼OMMB���͵���ָ��
		int nLoginCount(3);
		bool bRet(false);

		//while (nLoginCount-- > 0)
		//{
		//	bRet = m_pTelnet->Login(m_pOMMBCfg->strIP,m_pOMMBCfg->nPort,m_pOMMBCfg->strUser,m_pOMMBCfg->strPassword);

		//	if(bRet) break;//��ʱ���¼�ɹ���������
		//	Sleep(1000*10);//10�������
		//}
		bRet = m_pTelnet->Login(m_pOMMBCfg->strIP,m_pOMMBCfg->nPort,m_pOMMBCfg->strUser,m_pOMMBCfg->strPassword);
		//bRet = m_pTelnet->Login(m_pOMMBCfg->strIP,m_pOMMBCfg->nPort,m_pOMMBCfg->strUser,m_pOMMBCfg->strPassword,0);

		if (!bRet) return;

	}

	CString strLog;
	bool  bIsSend(false);//ָ���Ƿ��·�

	if (m_pOMMBCfg->nOperation == 2)
	{
		bIsSend = true;//�·�ָ��
	}

	//���͵���ָ��
	CString strEnbList;
	for (map<CString,CENB*>::iterator itEnb = m_mapENBObj.begin(); itEnb != m_mapENBObj.end(); itEnb++)
	{
		CENB* pEnb = itEnb->second;
		if (NULL == pEnb || !pEnb->m_bValid || pEnb->m_vecAdjustCmd.size() == 0) continue;

		strLog.Format("%s-OMMB[%s]-����ENB[%s]-------------------------------------------------------------------------------------",m_strThread,m_pOMMBCfg->strName,pEnb->m_strEnbKey);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);

		int nSendCount(0);
		//��enb��������б�
		CString strEnbMark;
		strEnbMark.Format("&\"%d\"-\"%d\"",pEnb->m_nSubNetwork,pEnb->m_nMEID);
		strEnbList += strEnbMark;

		for (int icmd = 0; icmd < (int)pEnb->m_vecAdjustCmd.size(); icmd++)
		{
			CString strcmd(pEnb->m_vecAdjustCmd[icmd]);
			CString strData("");
			if (strcmd.IsEmpty())
			{
				nSendCount++;
				continue;
			}

			//int nRet = m_pTelnet->Send(strcmd,strData,bIsSend);
			int nRet = m_pTelnet->Send(strcmd,strData,CHECK_EXEC,bIsSend);

			if (nRet == false)
			{
				strLog.Format("%s-OMMB[%s]-����ENB[%s]ָ���·�ʧ�ܣ�������",m_strThread,m_pOMMBCfg->strName,pEnb->m_strEnbKey);
				g_pLog->WriteLog(strLog,m_strLogFile,LOG_ERROR);
				break;
			}

			//����ָ�����
			nSendCount++;
			//SaveCMDToDatabase(pCELL->m_nID,strcmd,1,0,"");
		}//for (int icmd = 0; icmd < (int)pCELL->m_vecAdjustCmd.size(); icmd++)

		//��������ɹ� ����ͳ������
		if (nSendCount > 0 && nSendCount == (int)pEnb->m_vecAdjustCmd.size() && bIsSend)
		{
			g_pLog->WriteLog("����ͳ����Ϣ",m_strLogFile,LOG_INFO);
			m_pOMMBObj->m_nAdjCellRet += pEnb->m_nAdjCell;
			m_pOMMBObj->m_nCallBackCellRet += pEnb->m_nCallBackCell;
			m_pOMMBObj->m_nAdjEnbRet += pEnb->m_nAdjEnb;
			m_pOMMBObj->m_nCallBackEnbRet += pEnb->m_nCallBackEnb;
			UpdateDatabaseResult(pEnb);
		}
		strLog.Format("%s-OMMB[%s]-����ENB[%s]���-------------------------------------------------------------------------------------",m_strThread,m_pOMMBCfg->strName,pEnb->m_strEnbKey);
		g_pLog->WriteLog(strLog,m_strLogFile,LOG_INFO);
	}

	//strEnbList.Delete(0,1);
	//CString strcmd;
	//CString strData("");
	//if(!strEnbList.IsEmpty())
	//{
	//	strcmd.Format("SYNC:POS=%s,TYPE=8,SYNCTYPE=1;",strEnbList);
	//	//m_pTelnet->Send(strcmd,strData,bIsSend);
	//	m_pTelnet->Send(strcmd);
	//}

	//�˳���¼
	m_pTelnet->Logout();
	//m_pTelnet->LogOut();

}

bool CCmdSend::UpdateDatabaseResult(CENB* pEnb)
{
	CString strSQL, strLog, strSendCMD, strOperLogSQL, strAdjRet;

	if (NULL == pEnb)return true;
	if ((int)pEnb->m_vecChild.size() == 0) return true;

	//����ָ��С�� �Ѿ�С������ָ��
	for (int i = 0; i < (int)pEnb->m_vecChild.size(); i++)
	{
		CCELL* pCELL(NULL);
		pCELL = (CCELL*)pEnb->m_vecChild[i];
		if (NULL == pCELL || pCELL->m_adjType == ADJ_ERROR 
			|| pCELL->m_strRemark.Find("SvrLow��������Χ") != -1)continue;

		//����������������
		strSQL.Format("Update PLUGIN_ZTE_LTE_CELL_RET_%s set threshSvrLow_Ret='%0.2f',thresholdOfRSRP_Ret='%0.2f',timeToTrigger_Ret='%0.2f', \
					  intraQrxLevMin_Ret='%0.2f',snonintrasearch_Ret='%0.2f',sIntraSearch_Ret='%0.2f',tReselectionGERAN_Ret='%0.2f', \
					  rsrpSrvTrd_Ret='%0.2f',trigTime_Ret='%0.2f',ADJ_RET ='�����ɹ�',selQrxLevMin_Ret='%0.2f' \
					  where OMMB_NAME='%s' and PERIOD_INDEX='%d' and SubNetwork='%d' and Enb_NAME='%d' and CELL_ID='%d'",CDispose::m_strYMD,
					  pCELL->m_dthreshSvrLow_Ret,pEnb->m_dthresholdOfRSRP_Ret,pEnb->m_dtimeToTrigger_Ret,
					  pCELL->m_dintraQrxLevMin_Ret,pCELL->m_dsnonintrasearch_Ret,pCELL->m_dsIntraSearch_Ret,pCELL->m_dtReselectionGERAN_Ret,
					  pEnb->m_drsrpSrvTrd_Ret,pEnb->m_dtrigTime_Ret,pCELL->m_dselQrxLevMin_Ret,
					  pCELL->m_strOMMB,CDispose::m_nPeriodIndex,pCELL->m_nSubNetwork,pCELL->m_nMEID,pCELL->m_nID);
		g_lstSQL.push_back(strSQL);

		//���������¼
		//����������������
		strSQL.Format("Update PLUGIN_ZTE_LTE_CELL set AdjRecord = '1' where OMMB_NAME='%s'and SubNetwork='%d' and MEID='%d' and CELL_ID='%d'"
			,pCELL->m_strOMMB,pCELL->m_nSubNetwork,pCELL->m_nMEID,pCELL->m_nID);
		g_lstSQL.push_back(strSQL);

		if (pCELL->m_adjType == ADJ_BACK)
		{
			strSQL.Format("Update PLUGIN_ZTE_LTE_CELL set BackAdjRecord = '%s' where OMMB_NAME='%s'and SubNetwork='%d' and MEID='%d' and CELL_ID='%d'"
				,CDispose::m_strYMD,pCELL->m_strOMMB,pCELL->m_nSubNetwork,pCELL->m_nMEID,pCELL->m_nID);
			g_lstSQL.push_back(strSQL);
		}

	}
	return true;
}