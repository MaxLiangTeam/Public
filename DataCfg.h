#pragma once

#include "Include.h"
//#include "NeObject.h"

#include <map>
#include <vector>
using namespace std;

//频模信息
struct TPowerInfo
{
	//CString strName;			//功率捡索名称
	int nTRXSize;					//载波数
	CString strStaticPower;     //静态功率
	CString strDynamicPower;    //动态功率

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
	static int m_nWorkMode;//工作模式 0 简化 1 正常
	static int m_nVersion;//版本 0推广 1河源定制
    static map<CString,stSTSTableInfo> m_mapCellSTSTab;//CELL级STS表配置表
	static map<CString,stSTSTableInfo> m_mapEnbSTSTab;  //Enb级STS表配置表
	static map<CString,vector<CString>> m_mapKpiTable;  //kpi配置表
	static map<CString,stDataTimeInfo> m_mapDataTimeTable;//数据时间配置表
	static map<CString,int> m_mapConfig;
	static vector<int> m_vecEGMSPoint;
	static vector<int> m_vecDSCPoint;
	static map<CString,TFreqModelInfo> m_mapFreqModeTab; //频模信息
	static vector<double> m_vecIreland;//爱尔兰配置表
	static map<CString,st2GSTSInfo> m_map2GSTSTab;//2GSTS表配置表

	static bool InsertParam(CString strName,int nValue,CString strRemark);
	static bool CheckConfig();
	static bool LoadWorkMode();
	static bool LoadSTSCfgTab(); //加载STS数据配置表
	static bool LoadKPICfg();//加载KPI配置表
	static bool LoadDataTimeCfgTab(); //加载数据时间配置表
	static bool LoadCapacityTab();
	static bool LoadEGMSPoint();//加载EGMS频点
	static bool LoadDSCPoint();//加载DCS1800频点
	static bool LoadFreqModeTab();//获取频模配置表
	static bool LoadIreland();//加载爱尔兰配置表
	static bool Load2GERICSTSData();//加载2G统计数据
	static bool Load2GHWSTSData();//加载2G统计数据

	bool LoadUpdateFlag();

	//加载所有配置数据
	bool Load();

	//加载所有对象
	vector<stOMMBCfg> m_vecOMMBCfg;
	bool LoadOMMBCfg();

	//加载所有读指令
	vector<stParamCfg> m_vecParamCfg;
	bool LoadParamCfg();

	//根据指令名找指令参数
	map<CString,map<CString,stField>> m_mapParamCfg;
	bool LoadParamCfg(CString strCmd,map<CString,stField>& vecParamCfg);
	void AddParamCfg(CString strCmd,CString strName,CString strByname,CString strType);

	//检测数据总表
	bool CheckTable(CString strName,CString strType,vector<CString>& vecCol);
	bool GetTableHead(CString strName,map<CString,int>& mapHead);

};
