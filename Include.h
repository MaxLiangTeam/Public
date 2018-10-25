#ifndef __INCLUDE_H_
#define __INCLUDE_H_

//#pragma once
#include <sys/stat.h>

//���ð����ļ�
#include "..\Public\ts_list.h"
#include "..\Public\ts_map.h"
#include "..\Public\ado.h"
#include "..\Public\Log.h"
#include "..\Public\Public.h"
#include "..\Public\XThreadPool.h"
#include "..\Public\Utils.h"
#include "..\Public\Service.h"

//���ýṹ����
#pragma pack(1)

enum enComeBackStatus
{
	STATUS_FAILED = -2,		//ʧ��
	STATUS_TIME_OUT = -1,    //��ʱ
	STATUS_INIT = 0,		//��ʼ��
	STATUS_GET_CDD = 1,		//ȡ��
	STATUS_FG_REVIEW = 2,	 //ǰ̨����
	STATUS_BG_REVIEW = 3,	 //��̨����
	STATUS_PLAN = 4,		//Ƶ�ʹ滮
	STATUS_CMD_REVIEW = 5,  //ָ������
	STATUS_CMD_SEND = 6,	//ָ���·�
	STATUS_SUCCESS = 7,		//�ɹ�
	STATUS_OTHERS= 8		//����
};

enum enAction
{
	ACTION_GET_CDD = 0,	//ȡ��
	ACTION_REVIEW = 1,	//����
	ACTION_CMD_SEND = 2,//ָ���·�
	ACTION_RET_OUTPUT = 3,//���ݵ���
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
	ADJ_ERROR = -1, //����
	ADJ_NORMAL = 0,  //����
	ADJ_BACK = 1  //�ص�

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
	JOB_ADJUST  = 0x7F,// �ɹ�
	JOB_TELNET	= 0x01,// TELNET�ɹ�
	JOB_GETSTS	= 0x02,// ��ȡͳ�Ƴɹ�
	JOB_END = 0x80//����
};

typedef struct _stField 
{
	CString strName;//������
	CString strByname;//�Զ�����
	CString strType;//����
	int nIndex;//�к�
} stField;

typedef struct _stOMMBCfg 
{
	CString strName;
	CString strIP;
	CString strUser;
	CString strPassword;

	int    nID;
	int    nPort;
	int    nOperation;//����

	int    nNotAdjustValid;   //�ǵ���ʱ����Ƿ�����,0-�����ã�Ĭ�ϣ���1-����
	double dNotAdjustBegin;//�ǵ���ʱ�ο�ʼʱ��,Ĭ��Ϊ0��24Сʱ��
	double dNotAdjustEnd;  //�ǵ���ʱ�ν���ʱ��,Ĭ��Ϊ0��24Сʱ��

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
	CString strKey;//С������[subnetwork + meid + cellid]
	CString strOMMBName;	//OMMB��
	CString strName;//С����
	int nType; //С������
	int nSubNetWork; //������
	int nMEID;		//enb ID
	int nCELLID;	//С������
	int nIsAdjust;	//�Ƿ����,0-��������1-������Ĭ�ϣ�
	int nRecord;    //�Ƿ��е�����¼
	int nBackRecord;//�Ƿ��лص���¼

	bool bStatus;	//С��״̬

	_stCELLCfg ()
	{
		nCELLID = -1;
		nIsAdjust = 0;//Ĭ�ϲ�����
		bStatus = false;
		nRecord = 0;
		nBackRecord = 0;
	}
} stCELLCfg;

//Ƶģ��Ϣ
struct TFreqModelInfo
{
	//CString strFreqModelName;  //Ƶģ����
	CString strGsm900TCH;      //GSM900Ƶ�ε�TCHƵ��
	CString strGsm1800TCH;     //GSM1800Ƶ�ε�TCHƵ��
	CString strRemark;         //��ע
};

//�߼����ʽ�ṹ��-��SYS_ERIC2G_Logic
typedef struct _stLogicCfg
{
	CString strName;	//�߼�����
	CString strLogic;	//�߼����ʽ(��̨��)
	int nStep;			//����
	int	nMinValue;		//��Сֵ
	int nMaxValue;		//���ֵ
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
	CString strName;//ָ������
	CString strFormula;//��ʽ
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
	CString strName;	//��������
	CString strbyName;	//��������
	CString strCmd;//ָ����ʽ(��̨��)
	int     nType;//0CELL 1ENB

	_stParamCfg()
	{
		nType = 0;
	}
} stParamCfg;

typedef struct _stCounterCfg
{
	CString strbyName;	//���������ݿ�������
	CString strName;	//Counter��
	CString strTime;//ʱ���
	CString strType;
	int nHisDays;//��ʷ����
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
	CString strLevel;//����
	CString strTable;//�����ؼ��ֶ�
	CString strTableName;//����
	CString strYMD;//����
	CString strPeridod;//ʱ��
	CString strCounterList;//Counter �б�
	CString strRemark;//��ע
	vector<CString> vecCounter;//Counter

	_stSTSTableInfo ()
	{
		nID = -1;
	}
} stSTSTableInfo;

typedef struct _stDataTimeInfo 
{
	CString strKey;//Key
	CString strYMD;//ʱ��

	int nDate;// ���ڣ�-1����ǰһ��
	int nIndex;// ʱ�Σ�-1����ǰһʱ��

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
