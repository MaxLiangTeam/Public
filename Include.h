#ifndef __INCLUDE_H_
#define __INCLUDE_H_

//#pragma once
#include <sys/stat.h>

//公用包涵文件
#include "..\Public\ts_list.h"
#include "..\Public\ts_map.h"
#include "..\Public\ado.h"
#include "..\Public\Log.h"
#include "..\Public\Public.h"
#include "..\Public\XThreadPool.h"
#include "..\Public\Utils.h"
#include "..\Public\Service.h"

//公用结构类型
#pragma pack(1)

enum enComeBackStatus
{
	STATUS_FAILED = -2,		//失败
	STATUS_TIME_OUT = -1,    //超时
	STATUS_INIT = 0,		//初始化
	STATUS_GET_CDD = 1,		//取数
	STATUS_FG_REVIEW = 2,	 //前台评审
	STATUS_BG_REVIEW = 3,	 //后台评审
	STATUS_PLAN = 4,		//频率规划
	STATUS_CMD_REVIEW = 5,  //指令评审
	STATUS_CMD_SEND = 6,	//指令下发
	STATUS_SUCCESS = 7,		//成功
	STATUS_OTHERS= 8		//其他
};

enum enAction
{
	ACTION_GET_CDD = 0,	//取数
	ACTION_REVIEW = 1,	//评审
	ACTION_CMD_SEND = 2,//指令下发
	ACTION_RET_OUTPUT = 3,//数据导出
	ACTION_OTHER 
};

enum enActionStatus
{
	ACTION_STATUS_START = 0,
	ACTION_STATUS_RUNNING = 1,
	ACTION_STATUS_FAILE = 2,
	ACTION_STATUS_SUCCESS = 3,
	ACTION_STATUS_OTHER 
};

enum enVersion
{
	VER_COMM = 0,
	VER_HY   = 1
};

enum enMode
{
	MODE_OTHER = -1,
	MODE_URGENT = 0,
	MODE_DAILY = 1,
	MODE_COMEBACK = 2
};

enum enCELLAdjType
{
	ADJ_ERROR = -1, //错误
	ADJ_NORMAL = 0,  //调整
	ADJ_BACK = 1  //回调

};

enum enMask
{ 
	MASK_PASS = 0,
	MASK_ADD  =1,
	MASK_CLOSE,
	MASK_DELETE
};
enum enStatus
{ 
	STATUS_READY = 0,
	STATUS_SUSPEND = 1,
	STATUS_EXEC,
	STATUS_END,
	STATUS_OVERTIME,
	STATUS_CANCEL,
	STATUS_OTHER
};

enum JobStatus
{
	JOB_ADJUST  = 0x7F,// 成功
	JOB_TELNET	= 0x01,// TELNET成功
	JOB_GETSTS	= 0x02,// 获取统计成功
	JOB_END = 0x80//结束
};

typedef struct _stField 
{
	CString strName;//网络名
	CString strByname;//自定义名
	CString strType;//类型
	int nIndex;//列号
} stField;

typedef struct _stOMMBCfg 
{
	CString strName;
	CString strIP;
	CString strUser;
	CString strPassword;

	int    nID;
	int    nPort;
	int    nOperation;//操作

	int    nNotAdjustValid;   //非调整时间段是否启用,0-不启用（默认），1-启用
	double dNotAdjustBegin;//非调整时段开始时间,默认为0，24小时制
	double dNotAdjustEnd;  //非调整时段结束时间,默认为0，24小时制

	_stOMMBCfg()
	{	
		nID = 0;
		nPort = 23;
		nOperation = 2;

		dNotAdjustBegin = 0;
		dNotAdjustEnd = 0;
		nNotAdjustValid = 0;
	}
} stOMMBCfg;

typedef struct _stCELLCfg
{
	CString strKey;//小区索引[subnetwork + meid + cellid]
	CString strOMMBName;	//OMMB名
	CString strName;//小区名
	int nType; //小区类型
	int nSubNetWork; //网络编号
	int nMEID;		//enb ID
	int nCELLID;	//小区代码
	int nIsAdjust;	//是否调整,0-不调整，1-调整（默认）
	int nRecord;    //是否有调整记录
	int nBackRecord;//是否有回调记录

	bool bStatus;	//小区状态

	_stCELLCfg ()
	{
		nCELLID = -1;
		nIsAdjust = 0;//默认不调整
		bStatus = false;
		nRecord = 0;
		nBackRecord = 0;
	}
} stCELLCfg;

//频模信息
struct TFreqModelInfo
{
	//CString strFreqModelName;  //频模名称
	CString strGsm900TCH;      //GSM900频段的TCH频点
	CString strGsm1800TCH;     //GSM1800频段的TCH频点
	CString strRemark;         //备注
};

//逻辑表达式结构体-表SYS_ERIC2G_Logic
typedef struct _stLogicCfg
{
	CString strName;	//逻辑名称
	CString strLogic;	//逻辑表达式(后台用)
	int nStep;			//步长
	int	nMinValue;		//最小值
	int nMaxValue;		//最大值
	int nType;

	_stLogicCfg()
	{
		nStep = 0;
		nMinValue = 0;
		nMaxValue = 0;
		nType = 0;
	}
} stLogicCfg;

typedef struct _stFormauleCfg
{
	CString strName;//指标名称
	CString strFormula;//公式
	int     nType;//0 CELL 1OMMB 2ENB
	int     nFlag;

	_stFormauleCfg()
	{
		nType = 0;
		nFlag = 0;
	}
} stFormauleCfg;

typedef struct _stParamCfg
{
	CString strName;	//参数名称
	CString strbyName;	//参数别名
	CString strCmd;//指令表达式(后台用)
	int     nType;//0CELL 1ENB

	_stParamCfg()
	{
		nType = 0;
	}
} stParamCfg;

typedef struct _stCounterCfg
{
	CString strbyName;	//参数在数据库中名字
	CString strName;	//Counter名
	CString strTime;//时间段
	CString strType;
	int nHisDays;//历史天数
	_stCounterCfg()
	{
		nHisDays = 0;
	}
} stCounterCfg;

typedef struct _stTable 
{
	int nStatus;
	int nCycleCount;
	//vector<void*> vecChild;
	_stTable()
	{
		nStatus = -1;
		nCycleCount = 0;
	}
} stTable;

typedef struct _stSTSTableInfo 
{
	int nID; //ID
	CString strKey;//Key
	CString strLevel;//级别
	CString strTable;//表名关键字段
	CString strTableName;//表名
	CString strYMD;//日期
	CString strPeridod;//时段
	CString strCounterList;//Counter 列表
	CString strRemark;//备注
	vector<CString> vecCounter;//Counter

	_stSTSTableInfo ()
	{
		nID = -1;
	}
} stSTSTableInfo;

typedef struct _stDataTimeInfo 
{
	CString strKey;//Key
	CString strYMD;//时间

	int nDate;// 日期，-1代表前一天
	int nIndex;// 时段，-1代表前一时段

	_stDataTimeInfo  ()
	{
		nDate = 0;
		nIndex = 0;
	}
} stDataTimeInfo ;

typedef struct _st2GSTSInfo 
{
	CString strKey;//Key
	CString strBSC;//
	CString strCell;//

	double dUlFlow;
	double dDlFlow;

	_st2GSTSInfo ()
	{
		dDlFlow= dUlFlow =0.0;
	}
} st2GSTSInfo ;

struct tagTaskConfig
{
	CString strName;
	CString strCondition;
	CString strTrueTask;
	CString strFalseTask;
	CString strDoAction;

	vector<CString> vecAlgorithmList;
	vector<CString> vecDoActionList;
};

struct tagCmdParamConfig
{
	CString strParamName;
	CString strParamType;
	bool nValType;
	int nMinVal;
	int nMaxVal;
	int nStep;
	int nVal;
};


#pragma pack()

#endif
