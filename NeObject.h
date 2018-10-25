#pragma once

#include <map>
#include "include.h"
#include "vector"
using namespace std;

//�ڵ����
class CNode
{
public:
	CNode();
	virtual ~CNode();
public:
	CString m_strType;//�ڵ�����
	CString m_strName;//�ڵ�����
	int     m_nID;//�ڵ�ID
	int     m_nParentID;
	bool    m_bValid;

	CNode* m_pParent;//������ָ��
	vector<CNode*>  m_vecChild;//�Ӷ���ָ��
	map<int,CNode*> m_mapChild;//�Ӷ���ָ��

	map<CString,map<CString,CString>*> m_mapTables;//����: ����-��(�ֶ���-�ֶ�ֵ)
	map<CString,CString>* GetTable(CString strTable);//���ݱ�����ȡ��
	void DelTable(CString strTable);//ɾ����
	
	bool SetValue(CString strName,CString  strValue,CString strTable);//��ȡֵ
	bool GetValue(CString strName,CString& strValue,CString strTable);//����ֵ

	bool SetValue(CString strName,double  dValue,CString strTable);//��ȡֵ
	bool GetValue(CString strName,double& dValue,CString strTable);//����ֵ
};


struct C2GCell
{
	CString BscName;
	CString CellName;
	CString CellId;
	bool bAdjust;
	CString strCmd;
	CString Remark;
};

//2GBsc����
class C2GBsc : public CNode
{
public:
	C2GBsc(){};
	virtual ~C2GBsc(){};
public:

	CString BscName;
	CString OssIp;
	int OssPort;
	CString OssUser;
	CString ossPwd;
	int nOperation;
	std::map<CString,C2GCell> m_mapCell;

};

//CELL����
class CCELL : public CNode
{
public:
	CCELL();
	virtual ~CCELL();
public:
	void* m_pCELLCfg;//С������ָ��
	CString m_strRemark;
	CString m_strOMMB;
	CString m_strKey;//С������[subnetwork + meid + cellid]
	CString m_strMOI;//EUtranReselectionTDD
	CString m_strMOC;
	CString m_strGsmTDDMOI;//GsmReselectionTDD
	CString m_strGsmTDDMOC;
	CString m_strAlgorithmList;
	CString m_strDoActionList;

	vector<CString> m_vecResetCmd;//��λָ��
	map<CString,map<CString,double>>  m_mapCounterValue; //time-counterlist��ֵ
	map<CString,double> m_mapKpiValue;	//KPI��ֵ
	map<CString,C2GCell> m_map2GCell;

	//С��������
	bool m_bIsSend;//С���Ƿ����
	int m_nSubNetwork;//������
	long m_nMEID;//enbID
	int m_nConfigSet;
	long m_nENBFunctionTDD;
	int m_nEUtranCellTDD;
	int m_nEUtranReselectionTDD;

	int m_adjType; //С���������� 0-���� 1-�ص� -1Ĭ��ֵ
	int m_nPriority;      //С�����ȼ�


	double m_dthreshSvrLow;//������Ƶ������(dB)
	double m_dthreshSvrLow_Ret;//������Ƶ������(dB)
	double m_dintraQrxLevMin;     //Ƶ��С����ѡ�������С����ˮƽ
	double m_dselQrxLevMin;     //С��ѡ���������СRSRP����ˮƽ
	double m_dsnonintrasearch;    //��Ƶ��ѡ4G��������
	double m_dsIntraSearch ;      //ͬƵ��ѡ4G��������
	double m_dtReselectionGERAN ; //��ѡ��GERANС���о���ʱ������
	double m_dintraQrxLevMin_Ret;     //Ƶ��С����ѡ�������С����ˮƽ
	double m_dselQrxLevMin_Ret;     //С��ѡ���������СRSRP����ˮƽ
	double m_dsnonintrasearch_Ret;    //��Ƶ��ѡ4G��������
	double m_dsIntraSearch_Ret ;      //ͬƵ��ѡ4G��������
	double m_dtReselectionGERAN_Ret ; //��ѡ��GERANС���о���ʱ������
	//double m_dthresholdOfRSRP;//�¼��о���RSRP����
	//double m_dtimeToTrigger;//�¼��������ϱ���ʱ���
	//double m_drsrpSrvTrd;//RSRP����ʱE-UTRANϵͳ����С���о��ľ�������(dBm)
	//double m_dtrigTime;//�¼��������ϱ���ʱ���(����)
};

//Enb����(��վ)
class CENB : public CNode
{
public:
	CENB();
	virtual ~CENB();
public:
	CString m_strADJTYPE;
	CString m_strEnbKey;//С������[subnetwork + meid ]
	CString m_strUeEUtranMOC;//ָ�����MOC="UeEUtranMeasurementTDD"
	CString m_strUeEUtranMOI;//ָ�����MOI="SubNetwork=99,MEID=194306,ENBFunctionTDD=194306,UeEUtranMeasurementTDD=7"
	CString m_strUeRATMOC;//ָ�����MOC="UeRATMeasurementTDD"
	CString m_strUeRATMOI;//ָ�����MOI="SubNetwork=99,MEID=194306,ENBFunctionTDD=194306,UeRATMeasurementTDD=2"
	int m_nSubNetwork;//������
	long m_nMEID;//enbID
	bool m_bAdj;//enb�Ƿ����
	long int m_nAdjCell;//����С��
	long int m_nCallBackCell;//�ص�С��
	long int m_nAdjEnb;//����enb
	long int m_nCallBackEnb;//�ص�Enb

	double m_dthresholdOfRSRP;//�¼��о���RSRP����
	double m_dtimeToTrigger;//�¼��������ϱ���ʱ���
	double m_drsrpSrvTrd;//RSRP����ʱE-UTRANϵͳ����С���о��ľ�������(dBm)
	double m_dtrigTime;//�¼��������ϱ���ʱ���(����)

	double m_dthresholdOfRSRP_Ret;//�¼��о���RSRP����
	double m_dtimeToTrigger_Ret;//�¼��������ϱ���ʱ���
	double m_drsrpSrvTrd_Ret;//RSRP����ʱE-UTRANϵͳ����С���о��ľ�������(dBm)
	double m_dtrigTime_Ret;//�¼��������ϱ���ʱ���(����)

	map<CString,map<CString,double>>  m_mapCounterValue; //counter��ֵ
	vector<CString> vecAdjustCmd, vecAdjBackCmd;//С������ָ�� �� С���ص�ָ��
	vector<CString> m_vecAdjustCmd;//ָ������

	int GetAdjCell();//��ȡ����С��������
};

//OMMB����
class COMMB : public CNode
{
public:
	COMMB();
	virtual ~COMMB();
public:
	long int m_nAdjCell;//����С��
	long int m_nCallBackCell;//�ص�С��
	long int m_nAdjCellRet;//������С��
	long int m_nCallBackCellRet;//���ص�С��
	long int m_nAdjEnb;//����enb
	long int m_nCallBackEnb;//�ص�Enb
	long int m_nAdjEnbRet;//������Enb
	long int m_nCallBackEnbRet;//���ص�Enb

	//����С��
	double m_dAdjFlowSum; //4g����������+����(sts10+ sts11)
	double m_dAdjFlowAddRate; //������������(sts10+ sts11)-(his0-3 sts10+sts11 /3)/(his0-3 sts10+sts11 /3)
	double m_dAdjCallRate; //��ͨ��
	double m_dAdjDropRate; //������
	double m_dAdjTAURate; //�л��ɹ���
	//�ص�С��
	double m_dBackFlowSum; //4g����������+����(sts10+ sts11)
	double m_dBackFlowAddRate; //������������(sts10+ sts11)-(his0-3 sts10+sts11 /3)/(his0-3 sts10+sts11 /3)
	double m_dBackCallRate; //��ͨ��
	double m_dBackDropRate; //������
	double m_dBackTAURate; //�л��ɹ���

	map<CString,double> m_mapAdjCellKpiValue;	//KPI��ֵ
	map<CString,double> m_mapBackCellKpiValue;	//KPI��ֵ

	vector<int> m_vecSubNetWork;//�������б�
};

struct ParamCollect
{
	ParamCollect(CCELL& cell)
	{
		m_mapExtraStr["EUtranReselectionTDD_Moc"] = cell.m_strMOC;
		m_mapExtraStr["EUtranReselectionTDD_Moi"] = cell.m_strMOI;
		m_mapExtraStr["GsmReselectionTDD_Moc"] = cell.m_strGsmTDDMOC;
		m_mapExtraStr["GsmReselectionTDD_Moi"] = cell.m_strGsmTDDMOI;
		CENB *pEnb = dynamic_cast<CENB *>(cell.m_pParent );
		m_mapExtraStr["UeEUtranMeasurementTDD_Moc"] = pEnb->m_strUeEUtranMOC;
		m_mapExtraStr["UeEUtranMeasurementTDD_Moi"] = pEnb->m_strUeEUtranMOI;
		m_mapExtraStr["UeRATMeasurementTDD_Moc"] = pEnb->m_strUeRATMOC;
		m_mapExtraStr["UeRATMeasurementTDD_Moi"] = pEnb->m_strUeRATMOI;

		m_mapExtraValue["threshSvrLow"] = cell.m_dthreshSvrLow;
		m_mapExtraValue["intraQrxLevMin"] = cell.m_dintraQrxLevMin;
		m_mapExtraValue["selQrxLevMin"] = cell.m_dselQrxLevMin;
		m_mapExtraValue["snonintrasearch"] = cell.m_dsnonintrasearch;
		m_mapExtraValue["sIntraSearch"] = cell.m_dsIntraSearch;
		m_mapExtraValue["tReselectionGERAN"] = cell.m_dtReselectionGERAN;
		
		m_mapExtraValue["thresholdOfRSRP"] = pEnb->m_dthresholdOfRSRP;
		m_mapExtraValue["timeToTrigger"] = pEnb->m_dtimeToTrigger;
		m_mapExtraValue["rsrpSrvTrd"] = pEnb->m_drsrpSrvTrd;
		m_mapExtraValue["trigTime"] = pEnb->m_dtrigTime;
		m_mapExtraValue["ENB_TYPE"] = 0;

		//stCELLCfg* pCELLCfg =  dynamic_cast<stCELLCfg *>(cell.m_pCELLCfg );
		stCELLCfg* pCELLCfg = (stCELLCfg*)cell.m_pCELLCfg;
		m_mapExtraValue["CELL_TYPE"] = pCELLCfg->nType;
		m_mapExtraValue["AdjRecord"] = pCELLCfg->nRecord;
		m_mapExtraValue["BackAdjRecord"] = pCELLCfg->nBackRecord;
		

		m_mapCounterValue.insert(cell.m_mapCounterValue.begin(), cell.m_mapCounterValue.end());
		for (map<CString,map<CString,double>>::iterator item = pEnb->m_mapCounterValue.begin(); item != pEnb->m_mapCounterValue.end(); item++)
		{
			if (m_mapCounterValue.find(item->first) == m_mapCounterValue.end())
			{
				continue;
			}
			std::map<CString,double>& mapCounter = m_mapCounterValue[item->first];
			mapCounter.insert(item->second.begin(),item->second.end());
		}
		//m_mapCounterValue.insert(pEnb->m_mapCounterValue.begin(), pEnb->m_mapCounterValue.end());

	}
	ParamCollect(CENB& enb)
	{
		m_mapExtraStr["UeEUtranMeasurementTDD_Moc"] = enb.m_strUeEUtranMOC;
		m_mapExtraStr["UeEUtranMeasurementTDD_Moi"] = enb.m_strUeEUtranMOI;
		m_mapExtraStr["UeRATMeasurementTDD_Moc"] = enb.m_strUeRATMOC;
		m_mapExtraStr["UeRATMeasurementTDD_Moi"] = enb.m_strUeRATMOI;

		m_mapExtraValue["thresholdOfRSRP"] = enb.m_dthresholdOfRSRP;
		m_mapExtraValue["timeToTrigger"] = enb.m_dtimeToTrigger;
		m_mapExtraValue["rsrpSrvTrd"] = enb.m_drsrpSrvTrd;
		m_mapExtraValue["trigTime"] = enb.m_dtrigTime;
		m_mapExtraValue["ENB_TYPE"] = 1;
		m_mapExtraValue["CELL_SIZE"] = enb.GetAdjCell();

	}
	//����ָ���б�
	vector<CString> m_vecCmd;
	//�߼����
	tagTaskConfig m_TaskRet;

	map<CString,map<CString,double>> m_mapCounterValue;
	//���Ӳ���
	map<CString, double> m_mapExtraValue;
	//�����ַ�����
	map<CString, CString> m_mapExtraStr;
	//KPI��ֵ
	map<CString,double> m_mapKpiValue;	
};
