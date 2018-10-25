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
	virtual void ProcessJob(CWorkDesc* pJob);//任务处理函数
	int GetActionStatus(){return m_nActionStatus;}//获取对象当前处理的事件的状态
	void SetActionStatus(int nStatus){m_nActionStatus = nStatus;}//设置对象当前处理的事件的状态

	virtual void SendAdjustCmd();//发送指令

	bool UpdateDatabaseResult(CENB* pEnb);

public:
	int m_nActionStatus;

	CString m_strLogFile;//日志文件名
	CString m_strThread;//线程名
	CTelnet* m_pTelnet;//TELNET对象
	CDataCfg* m_pDataCfg;//数据配置
	stOMMBCfg* m_pOMMBCfg; //OMMB对象属性
	COMMB*     m_pOMMBObj; //现网OMMB
	map<CString,CENB*> m_mapENBObj;//基站索引-基站


protected:
	CRITICAL_SECTION	m_csLock;			//临界区

};

