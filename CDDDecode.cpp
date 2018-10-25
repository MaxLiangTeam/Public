/*---------------------------------------------------------------------------
文件    : 
描述    : 华为解码文件
作者    : wsl
版本    : V1.0
时间    : 2012-02-28
2012-06-30 更新Decode
---------------------------------------------------------------------------*/

#include "StdAfx.h"
#include "CDDDecode.h"
#include "Public.h"
#include <algorithm> 

CCDDDecode::CCDDDecode(void)
{
}

CCDDDecode::~CCDDDecode(void)
{
}

//-1数据可能不完整 但字段数和数据列数完整
//0成功
//1指令执行失败
//2找不到字符
//3数据行的列数与列头列数不符

int CCDDDecode::Decode(CString strCmd,CString& strData,map<CString,int>& mapHead,vector<vector<CString>>& vecValues,map<CString,CString>& mapPrm)
{
	//通用解码-------------------------------------------
	//modify by maxliang 20130726 北向登录[DSP LICUSAGE: TYPE=Current;] 分析失败
	CString strCmdFlag = strCmd + "%%";;
	int nTatol(0);	
	while (!strData.IsEmpty())
	{
		//获取指令包
		CString strPack = cutstr(strData,strCmdFlag,"---    END");
		if(strPack.IsEmpty())
		{
			break;
		}
		//判断指令是否执行成功
		if(!skipstr(strPack,"执行成功"))
		{
			return 1;//执行失败
		}

		int nStart(0),nEnd(0);
		if(!isLine(strPack,'-',nStart,nEnd))
		{
			return 2;//找不到关键字符
		}
		strPack.Delete(0,nEnd+1);
		strPack.TrimLeft('\r\n');
		strPack.TrimLeft();
		strPack.TrimLeft('\r\n');
		strPack.TrimLeft();

		//提取列头数据
		vector<int> vecClo;//当前列数
		int nPos = strPack.Find("\r\n\r\n");
		if (nPos == -1)
		{
			return 2;//如果没有空行则无法确定列头
		}
		CString strTemp = strPack.Left(nPos);
		strTemp.TrimRight();
		int nClo(0);

		while (!strTemp.IsEmpty())
		{
			CString strName = cutstr(strTemp,"  ");//列名
			if (strName.IsEmpty())
			{
				break;
			}
			//int nClo = (int)mapHead.size();
			map<CString,CString>::iterator it = mapPrm.find(strName);
			nClo++;
			if (it == mapPrm.end())
			{
				continue;
			}

			//插入新列
			mapHead.insert(make_pair(strName,nClo));
			vecClo.push_back(nClo);//列的位置
			strTemp.TrimLeft();// 去掉左边的空格

		}

		strPack.Delete(0,nPos + 4);
		nPos = strPack.Find("(结果个数");
		if(nPos == -1)
		{
			nPos = strPack.Find("仍有后续报告输出");
		}
		if (nPos != -1)
		{
			strTemp = strPack.Mid(nPos);
			strPack.Delete(nPos,strTemp.GetLength());
		}
		strPack.Delete(strPack.GetLength()-4,4);

		//根据目前最大列号进行值初始化
		//vector<CString> vecValue;

		//一行一行取出数据
		while (!strPack.IsEmpty())
		{
			CString strLine = cutstr(strPack,'\r\n');
			if (strLine.IsEmpty())
			{
				break;
			}
			strLine.TrimRight();
			vector<CString> vecValue;

			for(int i = 0; i < nClo;i++)
			{
				strLine.TrimLeft();
				CString strValue = cutstr(strLine,"  ");//每行所有列值 1 2  3 4
				if (vecClo.end() != find(vecClo.begin(),vecClo.end(),i))
				{
					vecValue.push_back(strValue);
				}
			}
			vecValues.push_back(vecValue);//所有行的值
		}

		//判断是否最后一包
		CString strRet = cutstr(strTemp,"所有结果总个数 =","\n");
		if (!strRet.IsEmpty())
		{
			strRet.Trim();
			if (atoi(strRet) == (int)vecValues.size())
			{
				return 0;//数据完整
			}
		}
	}
	return -1;//数据可能不完整 但字段数和数据列数完整
}

int CCDDDecode::DecodeColumnData(CString strCmd,CString& strData,vector<CString>& vecValues)
{
	//通用解码-------------------------------------------
	CString strCmdFlag = strCmd + "%%";;
	int nTatol(0);	
	while (!strData.IsEmpty())
	{
		//获取指令包
		CString strPack = cutstr(strData,strCmdFlag,"---    END");
		if(strPack.IsEmpty())
		{
			break;
		}
		//判断指令是否执行成功
		if(!skipstr(strPack,"执行成功") && !skipstr(strPack,"操作部分成功"))
		{
			return 1;//执行失败
		}

		while (!strPack.IsEmpty())
		{
			//获取指令包
			CString strColumnData = cutstr(strPack,"-----\n","(结果个数 =");
			if(strColumnData.IsEmpty())
			{
				break;;
			}
			vecValues.push_back(strColumnData);
		}

		if (vecValues.size() != 0)
			return 0;
	}

	return -1;//数据可能不完整 但字段数和数据列数完整
}

int CCDDDecode::GetMsgFromFile(CString strCmd,CString& strData,const char* strFile)
{
	//指针不能为空
	if (strFile == NULL)
	{
		return -1;
	}

	strCmd.TrimRight(';');
	if (strCmd.Find(':') == -1)
	{
		strCmd += ":";
	}
	int nCmdLen = strCmd.GetLength();

	strData  = strstr(strFile,strCmd);
	if(strData.IsEmpty())
	{
		return -1;
	}

	return 1;
}