/*---------------------------------------------------------------------------
文件    : 
描述    : 华为网元对象类
作者    : wsl
版本    : V1.0
时间    : 2012-01-13
2012-01-13 编写
2012-03-02 增加TRX、RXU、ENB
2012-03-06 增加删除表功能
2012-07-27 如果RXU只有一个载波，且该载波是BCCH，则不能减容
---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "NeObject.h"
#include "Include.h"
#include "DataCfg.h"
#include <algorithm> 

CNode::CNode()
:m_nID(-1),m_nParentID(-1),m_pParent(NULL),m_bValid(true)
{
}

CNode::~CNode()
{
	for(map<CString,map<CString,CString>*>::iterator it = m_mapTables.begin(); it != m_mapTables.end();it++)
	{
		delete it->second;
	}
}

//设置字段值，字符串型
bool CNode::SetValue(CString strName,CString strValue,CString strTable)
{
	map<CString,map<CString,CString>*>::iterator its = m_mapTables.find(strTable);
	map<CString,CString>* pMapTable = NULL;
	if (its == m_mapTables.end())
	{
		pMapTable = new map<CString,CString>;
		m_mapTables.insert(make_pair(strTable,pMapTable));
	}
	else
	{
		pMapTable = its->second;
	}

	map<CString,CString>::iterator it = pMapTable->find(strName);
	if (it == pMapTable->end())
	{
		pMapTable->insert(make_pair(strName,strValue));
		return true;
	}
	it->second = strValue;
	return false;
}

//获取字段值，字符串型
bool CNode::GetValue(CString strName,CString& strValue,CString strTable)
{
	map<CString,map<CString,CString>*>::iterator its = m_mapTables.find(strTable);
	if (its == m_mapTables.end())
	{
		return false;
	}

	map<CString,CString>* pMapTable = its->second;
	map<CString,CString>::iterator it = pMapTable->find(strName);
	if (it == pMapTable->end())
	{
		return false;
	}
	strValue = it->second;
	return true;
}

bool CNode::SetValue(CString strName,double  dValue,CString strTable)
{
	CString strValue;
	strValue.Format("%0.2f",dValue);
	return this->SetValue(strName,strValue,strTable);
}
bool CNode::GetValue(CString strName,double& dValue,CString strTable)
{
	CString strValue;
	if (this->GetValue(strName,strValue,strTable))
	{
		dValue = atof(strValue);
		return true;
	}
	return false;
}

map<CString,CString>* CNode::GetTable(CString strTable)
{
	map<CString,map<CString,CString>*>::iterator its = m_mapTables.find(strTable);
	if (its != m_mapTables.end())
	{
		return its->second;
	}
	return NULL;
}

void CNode::DelTable(CString strTable)
{
	map<CString,map<CString,CString>*>::iterator its = m_mapTables.find(strTable);
	if (its != m_mapTables.end())
	{
		delete its->second;
		m_mapTables.erase(its);
	}
}

//CELL对象
CCELL::CCELL()
:m_pCELLCfg(NULL),m_bIsSend(false),m_adjType(-1),m_nPriority(-1)
{
}

CCELL::~CCELL()
{
}


//Enb对象(基站)
CENB::CENB()
	:m_bAdj(false),m_nAdjCell(0),m_nCallBackCell(0),m_nAdjEnb(0),m_nCallBackEnb(0)
{
}

CENB::~CENB()
{

}

int CENB::GetAdjCell()
{
	int nSize(0);
	for(int i = 0; i < (int)m_vecChild.size(); i++)
	{
		CCELL* pCell = (CCELL*)m_vecChild[i];

		if (pCell->m_bIsSend && pCell->m_bValid)
		{
			nSize++;
		}
	}

	return nSize;
}

//COMMB对象
COMMB::COMMB()
	:m_nAdjCell(0),m_nCallBackCell(0),m_nAdjCellRet(0),m_nCallBackCellRet(0),m_nAdjEnb(0),m_nAdjEnbRet(0)
	,m_nCallBackEnb(0),m_nCallBackEnbRet(0),m_dAdjFlowSum(0),m_dAdjFlowAddRate(0),m_dAdjCallRate(0)
	,m_dAdjDropRate(0),m_dAdjTAURate(0),m_dBackFlowSum(0),m_dBackFlowAddRate(0),m_dBackCallRate(0)
	,m_dBackDropRate(0),m_dBackTAURate(0)
{
}
COMMB::~COMMB()
{
}

