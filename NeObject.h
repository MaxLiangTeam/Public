#pragma once

#include <map>
#include "include.h"
#include "vector"
using namespace std;

//节点对象
class CNode
{
public:
	CNode();
	virtual ~CNode();
public:
	CString m_strType;//节点类型
	CString m_strName;//节点名称
	int     m_nID;//节点ID
	int     m_nParentID;
	bool    m_bValid;

	CNode* m_pParent;//父对象指针
	vector<CNode*>  m_vecChild;//子对象集指针
	map<int,CNode*> m_mapChild;//子对象集指针

	map<CString,map<CString,CString>*> m_mapTables;//表集合: 表名-表(字段名-字段值)
	map<CString,CString>* GetTable(CString strTable);//根据表名获取表
	void DelTable(CString strTable);//删除表
	
	bool SetValue(CString strName,CString  strValue,CString strTable);//获取值
	bool GetValue(CString strName,CString& strValue,CString strTable);//设置值

	bool SetValue(CString strName,double  dValue,CString strTable);//获取值
	bool GetValue(CString strName,double& dValue,CString strTable);//设置值
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

//2GBsc对象
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

//CELL对象
class CCELL : public CNode
{
public:
	CCELL();
	virtual ~CCELL();
public:
	void* m_pCELLCfg;//小区配置指针
	CString m_strRemark;
	CString m_strOMMB;
	CString m_strKey;//小区索引[subnetwork + meid + cellid]
	CString m_strMOI;//EUtranReselectionTDD
	CString m_strMOC;
	CString m_strGsmTDDMOI;//GsmReselectionTDD
	CString m_strGsmTDDMOC;
	CString m_strAlgorithmList;
	CString m_strDoActionList;

	vector<CString> m_vecResetCmd;//复位指令
	map<CString,map<CString,double>>  m_mapCounterValue; //time-counterlist键值
	map<CString,double> m_mapKpiValue;	//KPI键值
	map<CString,C2GCell> m_map2GCell;

	//小区参数集
	bool m_bIsSend;//小区是否调整
	int m_nSubNetwork;//网络编号
	long m_nMEID;//enbID
	int m_nConfigSet;
	long m_nENBFunctionTDD;
	int m_nEUtranCellTDD;
	int m_nEUtranReselectionTDD;

	int m_adjType; //小区调整类型 0-调整 1-回调 -1默认值
	int m_nPriority;      //小区优先级


	double m_dthreshSvrLow;//服务载频低门限(dB)
	double m_dthreshSvrLow_Ret;//服务载频低门限(dB)
	double m_dintraQrxLevMin;     //频内小区重选所需的最小接收水平
	double m_dselQrxLevMin;     //小区选择所需的最小RSRP接收水平
	double m_dsnonintrasearch;    //异频重选4G启测门限
	double m_dsIntraSearch ;      //同频重选4G启测门限
	double m_dtReselectionGERAN ; //重选到GERAN小区判决定时器长度
	double m_dintraQrxLevMin_Ret;     //频内小区重选所需的最小接收水平
	double m_dselQrxLevMin_Ret;     //小区选择所需的最小RSRP接收水平
	double m_dsnonintrasearch_Ret;    //异频重选4G启测门限
	double m_dsIntraSearch_Ret ;      //同频重选4G启测门限
	double m_dtReselectionGERAN_Ret ; //重选到GERAN小区判决定时器长度
	//double m_dthresholdOfRSRP;//事件判决的RSRP门限
	//double m_dtimeToTrigger;//事件发生到上报的时间差
	//double m_drsrpSrvTrd;//RSRP测量时E-UTRAN系统服务小区判决的绝对门限(dBm)
	//double m_dtrigTime;//事件发生到上报的时间差(毫秒)
};

//Enb对象(基站)
class CENB : public CNode
{
public:
	CENB();
	virtual ~CENB();
public:
	CString m_strADJTYPE;
	CString m_strEnbKey;//小区索引[subnetwork + meid ]
	CString m_strUeEUtranMOC;//指令参数MOC="UeEUtranMeasurementTDD"
	CString m_strUeEUtranMOI;//指令参数MOI="SubNetwork=99,MEID=194306,ENBFunctionTDD=194306,UeEUtranMeasurementTDD=7"
	CString m_strUeRATMOC;//指令参数MOC="UeRATMeasurementTDD"
	CString m_strUeRATMOI;//指令参数MOI="SubNetwork=99,MEID=194306,ENBFunctionTDD=194306,UeRATMeasurementTDD=2"
	int m_nSubNetwork;//网络编号
	long m_nMEID;//enbID
	bool m_bAdj;//enb是否调整
	long int m_nAdjCell;//调整小区
	long int m_nCallBackCell;//回调小区
	long int m_nAdjEnb;//调整enb
	long int m_nCallBackEnb;//回调Enb

	double m_dthresholdOfRSRP;//事件判决的RSRP门限
	double m_dtimeToTrigger;//事件发生到上报的时间差
	double m_drsrpSrvTrd;//RSRP测量时E-UTRAN系统服务小区判决的绝对门限(dBm)
	double m_dtrigTime;//事件发生到上报的时间差(毫秒)

	double m_dthresholdOfRSRP_Ret;//事件判决的RSRP门限
	double m_dtimeToTrigger_Ret;//事件发生到上报的时间差
	double m_drsrpSrvTrd_Ret;//RSRP测量时E-UTRAN系统服务小区判决的绝对门限(dBm)
	double m_dtrigTime_Ret;//事件发生到上报的时间差(毫秒)

	map<CString,map<CString,double>>  m_mapCounterValue; //counter键值
	vector<CString> vecAdjustCmd, vecAdjBackCmd;//小区调整指令 和 小区回调指令
	vector<CString> m_vecAdjustCmd;//指令容器

	int GetAdjCell();//获取调整小区的数量
};

//OMMB对象
class COMMB : public CNode
{
public:
	COMMB();
	virtual ~COMMB();
public:
	long int m_nAdjCell;//调整小区
	long int m_nCallBackCell;//回调小区
	long int m_nAdjCellRet;//最后调整小区
	long int m_nCallBackCellRet;//最后回调小区
	long int m_nAdjEnb;//调整enb
	long int m_nCallBackEnb;//回调Enb
	long int m_nAdjEnbRet;//最后调整Enb
	long int m_nCallBackEnbRet;//最后回调Enb

	//调整小区
	double m_dAdjFlowSum; //4g总流量上行+下行(sts10+ sts11)
	double m_dAdjFlowAddRate; //流量增长比例(sts10+ sts11)-(his0-3 sts10+sts11 /3)/(his0-3 sts10+sts11 /3)
	double m_dAdjCallRate; //接通率
	double m_dAdjDropRate; //掉话率
	double m_dAdjTAURate; //切换成功率
	//回调小区
	double m_dBackFlowSum; //4g总流量上行+下行(sts10+ sts11)
	double m_dBackFlowAddRate; //流量增长比例(sts10+ sts11)-(his0-3 sts10+sts11 /3)/(his0-3 sts10+sts11 /3)
	double m_dBackCallRate; //接通率
	double m_dBackDropRate; //掉话率
	double m_dBackTAURate; //切换成功率

	map<CString,double> m_mapAdjCellKpiValue;	//KPI键值
	map<CString,double> m_mapBackCellKpiValue;	//KPI键值

	vector<int> m_vecSubNetWork;//网络编号列表
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
	//调整指令列表
	vector<CString> m_vecCmd;
	//逻辑结果
	tagTaskConfig m_TaskRet;

	map<CString,map<CString,double>> m_mapCounterValue;
	//附加参数
	map<CString, double> m_mapExtraValue;
	//附加字符参数
	map<CString, CString> m_mapExtraStr;
	//KPI键值
	map<CString,double> m_mapKpiValue;	
};
