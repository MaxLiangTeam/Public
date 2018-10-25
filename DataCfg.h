#pragma once

#include "Include.h"
//#include "NeObject.h"

#include <map>
#include <vector>
using namespace std;

//Ƶģ��Ϣ
struct TPowerInfo
{
	//CString strName;			//���ʼ�������
	int nTRXSize;					//�ز���
	CString strStaticPower;     //��̬����
	CString strDynamicPower;    //��̬����

	TPowerInfo()
	{
		nTRXSize = 0;
	}

	struct CompareTrx
	{
		CompareTrx(int nSz):nSize(nSz){}
		bool operator()(const TPowerInfo& power)
		{
			return power.nTRXSize == nSize;
		}
		int nSize;
	};

	struct CompareDPower
	{
		CompareDPower(CString strP):strPower(strP){}
		bool operator()(const TPowerInfo& power)
		{
			return power.strDynamicPower == strPower;
		}
		CString strPower;
	};

	struct CompareSPower
	{
		CompareSPower(CString strP):strPower(strP){}
		bool operator()(const TPowerInfo& power)
		{
			return power.strStaticPower == strPower;
		}
		CString strPower;
	};

	static bool SortByTRXSize(const TPowerInfo& obj1,const TPowerInfo& obj2)
	{
		return obj1.nTRXSize - obj2.nTRXSize > 0 ? true : false;
	}
};

class CDataCfg
{
public:
	CDataCfg(void);
public:
	virtual ~CDataCfg(void);
public:
	static int m_nWorkMode;//����ģʽ 0 �� 1 ����
	static int m_nVersion;//�汾 0�ƹ� 1��Դ����
    static map<CString,stSTSTableInfo> m_mapCellSTSTab;//CELL��STS�����ñ�
	static map<CString,stSTSTableInfo> m_mapEnbSTSTab;  //Enb��STS�����ñ�
	static map<CString,vector<CString>> m_mapKpiTable;  //kpi���ñ�
	static map<CString,stDataTimeInfo> m_mapDataTimeTable;//����ʱ�����ñ�
	static map<CString,int> m_mapConfig;
	static vector<int> m_vecEGMSPoint;
	static vector<int> m_vecDSCPoint;
	static map<CString,TFreqModelInfo> m_mapFreqModeTab; //Ƶģ��Ϣ
	static vector<double> m_vecIreland;//���������ñ�
	static map<CString,st2GSTSInfo> m_map2GSTSTab;//2GSTS�����ñ�

	static bool InsertParam(CString strName,int nValue,CString strRemark);
	static bool CheckConfig();
	static bool LoadWorkMode();
	static bool LoadSTSCfgTab(); //����STS�������ñ�
	static bool LoadKPICfg();//����KPI���ñ�
	static bool LoadDataTimeCfgTab(); //��������ʱ�����ñ�
	static bool LoadCapacityTab();
	static bool LoadEGMSPoint();//����EGMSƵ��
	static bool LoadDSCPoint();//����DCS1800Ƶ��
	static bool LoadFreqModeTab();//��ȡƵģ���ñ�
	static bool LoadIreland();//���ذ��������ñ�
	static bool Load2GERICSTSData();//����2Gͳ������
	static bool Load2GHWSTSData();//����2Gͳ������

	bool LoadUpdateFlag();

	//����������������
	bool Load();

	//�������ж���
	vector<stOMMBCfg> m_vecOMMBCfg;
	bool LoadOMMBCfg();

	//�������ж�ָ��
	vector<stParamCfg> m_vecParamCfg;
	bool LoadParamCfg();

	//����ָ������ָ�����
	map<CString,map<CString,stField>> m_mapParamCfg;
	bool LoadParamCfg(CString strCmd,map<CString,stField>& vecParamCfg);
	void AddParamCfg(CString strCmd,CString strName,CString strByname,CString strType);

	//��������ܱ�
	bool CheckTable(CString strName,CString strType,vector<CString>& vecCol);
	bool GetTableHead(CString strName,map<CString,int>& mapHead);

};
