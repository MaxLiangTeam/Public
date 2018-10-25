#pragma once

#include "Include.h"
#include "Telnet.h"
#include "DataCfg.h"
#include "NeObject.h"
//#include "XFtp.h"
#include <algorithm>
#include "LogicConfig.h"
#include "CmdSend.h"

class CDispose : public CWork
{
public:
	CDispose(void);
public:
	virtual ~CDispose(void);	
	virtual void ProcessJob(CWorkDesc* pJob);//��������
public:
	BYTE Status() {return m_byStatus;}//��ȡ״̬
	void Resset() {m_byStatus &= 0x7F;}//����(�ѽ�����־ȥ��)
	time_t Time() {return m_tTime;}//��ȡ���󴴽�ʱ��
	time_t Time2() {return m_tTime2;}//���������ʱ��
	int GetAction(){return m_nAction;} //��ȡ����ǰ������¼�
	void SetAction(int nAction){m_nAction = nAction;}//���ö���ǰ������¼�
	int GetActionStatus(){return m_nActionStatus;}//��ȡ����ǰ������¼���״̬
	void SetActionStatus(int nStatus){m_nActionStatus = nStatus;}//���ö���ǰ������¼���״̬

	time_t  m_tTime;//���󴴽�ʱ��
	time_t  m_tTime2;
	BYTE    m_byStatus;//����״̬
	int m_nAction; //����������¼�
	int m_nActionStatus;//�����¼���Ӧ��״̬

public:
	static time_t  m_tCurTime;//��ǰʱ��
	static CString m_strCurTime;//����ʱ��
	static CString m_strLog;//ʧ����־
	static int     m_nPeriodIndex;//������
	static CString m_strYMD;//130226
	static CString m_strPeriod;//����ʱ��

	static int     m_nADFailedSize;//�·�����ָ��ʧ�ܹ���OMMB����
	static int     m_nCBFailedSize;//�·�����ָ��ʧ�ܹ���OMMB����
	static int     m_nADUnequalSize;//�������ز�������OMMB����

	static int m_nAddResultCell; //���շ�����Ҫ����С����
	static int m_nAddResultTRX;//���շ�����Ҫ�����ز���

	static void Init(time_t time);
	
	//������
	static bool SortByPriority(CCELL* obj1,CCELL* obj2);
	//TELNET��������
	bool Telnet();

	//������
	bool BindData();

	//�ж��Ƿ�Ϊ����С��
	bool IsNewCell(int cellId);
	//��ȡС������
	bool GetCELL(void);
	//����С��
	void UpdateCELL();
	//����python�ű�
	bool StartPython(void);

	virtual bool CreateProject();//�ƹ��ճ�����
	virtual bool SetCELLLimit(CCELL* pCELL);//��������:С������{Ӳ������,�������}

	virtual bool DecodeData(void);//����
	virtual bool EraseUNAdjCell(vector<CCELL*> & vecCELL);//���˲�����С��
	virtual bool GetParamCollAndMakeCMD();//�����߼��������������ɵ���ָ��
	virtual void SendAdjustCmd();//����ָ��
	void Send2GAdjustCmd();

	bool GetCellConfigFromDC(void);//��ȡ��������С������
	bool GetCELLCDD(void);//��ȡС��CDD
	bool GetEnbCDD(void);//��ȡenb CDD
	bool GetCELLSTS(void);//��ȡС��CDD
	bool GetEnbSTS(void);//��ȡenb CDD
	bool Get2GCellSTS(void);//��ȡ2GС��CDD
	bool Get2GBscConfig(void);//��ȡ2g��Ԫ����
	bool CheckAndGetTable(vector<CString> & vecCounter, map<CString,stSTSTableInfo> & maptable,
							map<CString,stSTSTableInfo> & mapSTStable ,stDataTimeInfo & mapTime);//���STS�������ñ�
	bool SetCellProInfo(CCELL& pCELL,ParamCollect& PCellCollect); //����С��������Ϣ
	bool CellParamCollect(CCELL& pCELL,ParamCollect& PCellCollect);//����С������

	//����ָ��ļ�
	void SaveCMDToFile(CString& strcmd);
	bool SaveAdjCmdToFile(CString& strFile);
	bool GetCDDDataStatus();
	bool GetSTSDataStatus();

	void SaveCMDToDatabase(int& nCELLID,CString& strCMD,int nType,int nRet,CString strRemark);

	//�������
	void SaveRETToDatabase(void);
	void SaveCDDToDatabase(void);
	void SaveCMDToDatabase(void);
	void MakeLine(map<CString,int>& mapField,vector<CString>& vecData,CString& strSQL,CString strName,CString strType,CString strFormat,...);
	bool InportFile(CString strName,vector<vector<CString>>& vecDatas,CString& strSQL,bool bHandle);
	bool InportFile(CString strName,CString& strData);
	bool UpdateDatabaseResult(CENB* pEnb);
	bool ReportProInfo();
	void DecodeCmdUpdateInfo(CString& strCmdData);

	void DoProcess(ParamCollect& paramColl, CScriptEngine& cmdEngine);
	void GetCmdSet(CScriptEngine& cmdEngine, const CString& taskId, std::vector<CString>& cmdIdSet, tagTaskConfig& taskConf);
	void CreateCmd(std::vector<CString>& cmdIdSet, ParamCollect& paramColl);
	CString MakeUpCmd(const CString& strParamId, ParamCollect& paramColl);
	bool CreateCMDThread();

public:
	int m_nStartPos;//�ز���������
	CString m_strLogFile;//��־�ļ���
	CTelnet* m_pTelnet;//TELNET����
	CSTelnet* m_p2GTelnet;//TELNET����
    //CXFtp *  m_pXFtp;  //FTP����

	CDataCfg* m_pDataCfg;//��������
	stOMMBCfg* m_pOMMBCfg; //OMMB��������
	COMMB*     m_pOMMBObj; //����OMMB

	map<CString,stCELLCfg>  m_mapCELLCfg; //���Ա���DB
	vector<CCELL*> m_vecAdjCell;  //�����б�
	map<CString,CCELL*> m_mapCELLObj;//С������-С��
	map<CString,CENB*> m_mapENBObj;//��վ����-��վ
	map<CString,CString> m_mapCommand;//���õ�ָ��
	map<CString,C2GBsc> m_map2GBsc;//2g��Ԫ

	LogicConfig m_logicConfig;

protected:
	CRITICAL_SECTION	m_csLock;			//�ٽ���
};
