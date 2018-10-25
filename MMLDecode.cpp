/*---------------------------------------------------------------------------
文件    : 
描述    : 华为解码文件
作者    : wsl
版本    : V1.0
时间    : 2012-07-02
---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "MMLDecode.h"

CMMLDecode::CMMLDecode(void)
{
}

CMMLDecode::~CMMLDecode(void)
{
}

int CMMLDecode::Decode(CString strCmd,const char* strData,vector<map<CString,CString>>& vecValues)
{
	//指针不能为空
	if (strData == NULL)
	{
		return -1;
	}

	strCmd.TrimRight(';');
	if (strCmd.Find(':') == -1)
	{
		strCmd += ":";
	}
	int nCmdLen = strCmd.GetLength();

	//解析BAM版本
	if (strCmd.Find("For BAM version:") != -1)
	{
		CString strExtempData ="For BAM version: V900RXXXC00SPH552";
		map<CString,CString> mapValue;
		CString strVerSionData = strstr(strData,strCmd);
		strVerSionData.Delete(strExtempData.GetLength(),strVerSionData.GetLength()-strExtempData.GetLength());
		if(strVerSionData.IsEmpty())
		{
			return -1;
		}
		mapValue.insert(make_pair(strCmd,strVerSionData));
		vecValues.push_back(mapValue);
		return 0;
	}

	while(true)
	{ 
		strData  = strstr(strData,strCmd);
		if(strData == NULL)
		{
			break;
		}

		strData = strData + nCmdLen;

		int nIndex(0);
		CString strLine;
		vector<int> vecClo;
		vecClo.push_back(-1);
		while (strData[nIndex] != ';')
		{
			strLine += strData[nIndex];
			if (strData[nIndex] == ',' || strData[nIndex] == '=')
			{
				vecClo.push_back(nIndex);
			}
			nIndex++;
		}
		vecClo.push_back(nIndex);
		if (strLine.IsEmpty() || vecClo.size()%2 != 1)
		{
			break;
		}

		strData = strData + nIndex + 1;

		vecValues.push_back(map<CString,CString>());
		map<CString,CString>& mapValue = vecValues[vecValues.size()-1];
		for (int i = 0; i < (int)vecClo.size()/2; i++)
		{
			CString strName =  strLine.Mid(vecClo[2*i] + 1,vecClo[2*i+1] - vecClo[2*i] - 1);
			CString strValue = strLine.Mid(vecClo[2*i+1] + 1,vecClo[2*i+2] - vecClo[2*i+1] - 1);
			strName.Trim();
			strValue.Trim();
			strValue.Trim('\"');
			mapValue.insert(make_pair(strName,strValue));
		}
	}
	return 0;
}