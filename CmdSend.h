#pragma once

#include "Include.h"
#include "Telnet.h"
#include "DataCfg.h"
#include "NeObject.h"
#include "LTETelnet.h"
//#include "XFtp.h"
#include <algorithm>

class CCmdSend : public CWork
{
public:
	CCmdSend(map<CString,CENB*>& mapData);
	CCmdSend(void);
public:
	virtual ~CCmdSend(void);	
	virtual void ProcessJob(CWorkDesc* pJob);//��������
	int GetActionStatus(){return m_nActionStatus;}//��ȡ����ǰ������¼���״̬
	void SetActionStatus(int nStatus){m_nActionStatus = nStatus;}//���ö���ǰ������¼���״̬

	virtual void SendAdjustCmd();//����ָ��

	bool UpdateDatabaseResult(CENB* pEnb);

public:
	int m_nActionStatus;

	CString m_strLogFile;//��־�ļ���
	CString m_strThread;//�߳���
	CTelnet* m_pTelnet;//TELNET����
	CDataCfg* m_pDataCfg;//��������
	stOMMBCfg* m_pOMMBCfg; //OMMB��������
	COMMB*     m_pOMMBObj; //����OMMB
	map<CString,CENB*> m_mapENBObj;//��վ����-��վ


protected:
	CRITICAL_SECTION	m_csLock;			//�ٽ���

};

