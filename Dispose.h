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
	virtual void ProcessJob(CWorkDesc* pJob);//任务处理函数
public:
	BYTE Status() {return m_byStatus;}//获取状态
	void Resset() {m_byStatus &= 0x7F;}//重启(把结束标志去掉)
	time_t Time() {return m_tTime;}//获取对象创建时间
	time_t Time2() {return m_tTime2;}//正常跑完的时间
	int GetAction(){return m_nAction;} //获取对象当前处理的事件
	void SetAction(int nAction){m_nAction = nAction;}//设置对象当前处理的事件
	int GetActionStatus(){return m_nActionStatus;}//获取对象当前处理的事件的状态
	void SetActionStatus(int nStatus){m_nActionStatus = nStatus;}//设置对象当前处理的事件的状态

	time_t  m_tTime;//对象创建时间
	time_t  m_tTime2;
	BYTE    m_byStatus;//对象状态
	int m_nAction; //方案处理的事件
	int m_nActionStatus;//方案事件对应的状态

public:
	static time_t  m_tCurTime;//当前时间
	static CString m_strCurTime;//调整时间
	static CString m_strLog;//失败日志
	static int     m_nPeriodIndex;//周期数
	static CString m_strYMD;//130226
	static CString m_strPeriod;//周期时段

	static int     m_nADFailedSize;//下发调整指令失败过多OMMB数量
	static int     m_nCBFailedSize;//下发倒回指令失败过多OMMB数量
	static int     m_nADUnequalSize;//扩减容载波数不等OMMB数量

	static int m_nAddResultCell; //最终方案需要扩容小区数
	static int m_nAddResultTRX;//最终方案需要扩容载波数

	static void Init(time_t time);
	
	//排序函数
	static bool SortByPriority(CCELL* obj1,CCELL* obj2);
	//TELNET现网参数
	bool Telnet();

	//绑定数据
	bool BindData();

	//判断是否为新增小区
	bool IsNewCell(int cellId);
	//获取小区配置
	bool GetCELL(void);
	//更新小区
	void UpdateCELL();
	//启动python脚本
	bool StartPython(void);

	virtual bool CreateProject();//推广日常方案
	virtual bool SetCELLLimit(CCELL* pCELL);//修正方案:小区限制{硬件限制,软件限制}

	virtual bool DecodeData(void);//解码
	virtual bool EraseUNAdjCell(vector<CCELL*> & vecCELL);//过滤不调整小区
	virtual bool GetParamCollAndMakeCMD();//调用逻辑解析器并且生成调整指令
	virtual void SendAdjustCmd();//发送指令
	void Send2GAdjustCmd();

	bool GetCellConfigFromDC(void);//获取数据中心小区配置
	bool GetCELLCDD(void);//获取小区CDD
	bool GetEnbCDD(void);//获取enb CDD
	bool GetCELLSTS(void);//获取小区CDD
	bool GetEnbSTS(void);//获取enb CDD
	bool Get2GCellSTS(void);//获取2G小区CDD
	bool Get2GBscConfig(void);//获取2g网元配置
	bool CheckAndGetTable(vector<CString> & vecCounter, map<CString,stSTSTableInfo> & maptable,
							map<CString,stSTSTableInfo> & mapSTStable ,stDataTimeInfo & mapTime);//检查STS参数配置表
	bool SetCellProInfo(CCELL& pCELL,ParamCollect& PCellCollect); //设置小区方案信息
	bool CellParamCollect(CCELL& pCELL,ParamCollect& PCellCollect);//修正小区方案

	//保存指令到文件
	void SaveCMDToFile(CString& strcmd);
	bool SaveAdjCmdToFile(CString& strFile);
	bool GetCDDDataStatus();
	bool GetSTSDataStatus();

	void SaveCMDToDatabase(int& nCELLID,CString& strCMD,int nType,int nRet,CString strRemark);

	//数据入库
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
	int m_nStartPos;//载波索引进度
	CString m_strLogFile;//日志文件名
	CTelnet* m_pTelnet;//TELNET对象
	CSTelnet* m_p2GTelnet;//TELNET对象
    //CXFtp *  m_pXFtp;  //FTP对象

	CDataCfg* m_pDataCfg;//数据配置
	stOMMBCfg* m_pOMMBCfg; //OMMB对象属性
	COMMB*     m_pOMMBObj; //现网OMMB

	map<CString,stCELLCfg>  m_mapCELLCfg; //来自本地DB
	vector<CCELL*> m_vecAdjCell;  //扩容列表
	map<CString,CCELL*> m_mapCELLObj;//小区索引-小区
	map<CString,CENB*> m_mapENBObj;//基站索引-基站
	map<CString,CString> m_mapCommand;//配置的指令
	map<CString,C2GBsc> m_map2GBsc;//2g网元

	LogicConfig m_logicConfig;

protected:
	CRITICAL_SECTION	m_csLock;			//临界区
};
