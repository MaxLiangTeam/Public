#pragma once
#include <vector>
#include <map>
#include "Kernel.h"
#include "STelnet.h"
using namespace std;

struct TagDrain
{
	CString BscName;
	CString Cgi;
	CString RncCellId;
	CString Cell;
	CString UarfcnCpi;
	CString NeType;
	CString GsmCell;
	CString strCmd;
	int nPoint;
	bool bAdjust;
	bool bAdjRet;
	TagDrain()
	{
		bAdjRet = false;
		bAdjust = false;
		nPoint = 0;
	}
};

struct TagMultiSet
{
	CString Exchid;
	CString Cell;
	CString Cgi;
	CString RncCellId;
	CString MBCCHNO;
	CString NeType;
	CString Tag;
	CString strCmd;
	bool bAdjust;
	bool bAdjRet;
	TagMultiSet()
	{
		bAdjRet = false;
		bAdjust = false;
	}
};

struct TagCellConf
{
	CString BscName;
	CString CellName;
	CString CellCgi;
	CString CellId;
	CString NeType;
	bool bAdjust;
	CString Remark;
};

struct TagBscConf
{
	CString BscName;
	CString OssIp;
	int OssPort;
	CString OssUser;
	CString ossPwd;
	int nOperation;
	std::map<CString,TagCellConf> mapCellCfg;
};

struct TagCellGsmRelation
{
	CString BscName;	//网元名
	CString strDate;	//日期	
	CString str3GCell;	//3g定义的小区id
	CString strNeType;  //厂商		
	CString strMeid;			//RNCid号
	CString strTUtranCellTDDLcr;//
	CString strTExternalGsmCell;//
};

struct TagCellTUtranCellTDDLcr
{
	CString BscName;
	CString strDate;
	CString str3GCell;
	CString strNeType;
	CString strMeid;
	CString strTUtranCellTDDLcr;
	CString nCellId;
};

struct TagCellUarfcn
{
	CString BscName;
	CString strDate;
	CString str3GCell;
	CString strNeType;
	CString strMeid;
	CString strTUtranCellTDDLcr;
	int nUarfcn;//频点
};

struct TagCellTExternalGsmCell
{
	CString BscName;
	CString strDate;
	CString str3GCell;
	CString strNeType;
	CString strMeid;
	CString strTExternalGsmCell;
	CString Cgi;
};

struct TagCellRLDEP
{
	CString BscName;
	CString strDate;
	CString str3GCell;
	CString strNeType;
	CString Cgi;
	CString str2GCell;//2g定义的小区名
};

struct Tag3GCell
{
	CString BscName;
	CString strMeid;
	CString strDate;
	CString str3GCell;
	CString strNeType;
	CString Cgi;
	CString vec2GCell;
	int nUarfcn;
};


struct Tag2GCell
{
	CString BscName;
	CString strDate;
	CString str2GCell;
	CString strNeType;
	int mbcch;
	CString TMFI;
};

class CAdjustTask
{
public:
	CAdjustTask(void);
	~CAdjustTask(void);
	bool InitTable();
	bool Process();
	bool CheckCmd();
private:
	void LoadMultiSet();
	void LoadDrainSet();
	void LoadUarfcnSize();
	bool LoadBscConfig();
	bool loadCellConfig();
	bool loadTGsmRelationTable(map<CString,vector<TagCellGsmRelation>>& mapCellGsmRelation);
	bool loadTUtranCellTDDLcrTable(map<CString,vector<TagCellTUtranCellTDDLcr>>& mapCellTUtranCellTDDLcr);
	bool loadTExternalGsmCellTable(map<CString,vector<TagCellTExternalGsmCell>>& mapCellTExternalGsmCell);
	bool loadTUarfcnTable(map<CString,vector<TagCellUarfcn>>& mapCellUarfcn); 
	bool loadRLDEPTable(map<CString,TagCellRLDEP>& mapCellRLDEP);
	bool loadRLUMPTable();//获取2g定义3g表
	bool Meger3gCellInfo(map<CString,vector<TagCellGsmRelation>>& mapCellGsmRelation,
						map<CString,vector<TagCellTUtranCellTDDLcr>>& mapCellTUtranCellTDDLcr,
						map<CString,vector<TagCellTExternalGsmCell>>& mapCellTExternalGsmCell,
						map<CString,vector<TagCellUarfcn>>& mapCellUarfcn,
						map<CString,TagCellRLDEP>& mapCellRLDEP);//同步绑定3g定义2g表
	bool load3GInfo();
	bool UpdateCell();
	bool CreateEOESData();
	void GenerateDrainCmd(std::vector<TagDrain>& data , std::vector<TagMultiSet>& dataMulti);
	void GenerateMultiCmd(std::vector<TagMultiSet>& data);
	bool Meger7DayData();//汇总7天的2-3g数据

	void SendAdjustCmd(TagBscConf& bscConf);
	void DumpAdjustCmd(TagBscConf& bscConf, std::vector<CString>& vecCmd);
	void SaveAdjustCmd(CString& strDate, CString& strCmd, int nRes);
	void MakeLine(map<CString,int>& mapField,vector<CString>& vecData,CString& strSQL,CString strName,CString strType,CString strFormat,...);
	bool InportFile(CString strName,vector<vector<CString>>& vecDatas,CString& strSQL,bool bHandle);
	void SaveRETToDatabase(void);
	void Save2G3GDataTable(void);
	bool CreateRetTable(void);
	bool Create2G3GDataTable(void);

private:
	CSTelnet* m_pTelnet;//TELNET对象
	std::map<CString, std::vector<TagDrain> > m_mapDrain;
	std::map<CString, std::vector<TagMultiSet> > m_mapMultiSet;
	std::map<CString, std::vector<Tag3GCell> > m_map3GCell;
	std::map<CString, std::vector<Tag2GCell> > m_map2GCell;
	std::vector<TagBscConf> m_vecBscConf;
	CString m_strLogFile;
};

