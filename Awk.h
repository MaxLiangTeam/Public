#ifndef __AWK_H__
#define __AWK_H__


/***********************************************************************************
* Copyright (c) 2010～2014, DingLi Communication Inc. All rights reserved.
* @brief   :字符串解释、分割类
* @author  :zengtongqiao
* @date    :2013-07-17
* 使用示例：
	
	bool CheckData(std::vector<std::string>& line, void* para)
	{
		if (line.size() != 4)
		{
			return false;
		}
		if (line[2].find("%") == std::string::npos)
		{
			return false;
		}
		if (!CAwk::StrIsInt(line[0].c_str()))
		{
			return false;
		}
		return true;
	}
	
	ifstream fin("华为子系统负荷查询LOG.txt");
	char szBuf[4096];
	fin.read(szBuf, sizeof(szBuf));

	CAwk awk(szBuf);
	const char* col[] = {"槽位号", "子系统号", "CPU占用率", "吞吐量占用率"};
	awk.Patten(4, " ", col);
	//对获取到的数据集进行过滤，CheckData函数返回false的行将被丢弃掉
	awk.FilterData(CheckData, NULL);
	double max = 0;
	//从数据集中获取数据
	awk.FetchData(Max, &max);
************************************************************************************/
#include <string>
#include <vector>
#include <list>

//回调函数   供数据提取、过滤使用
typedef bool (*AWKFUNC)(std::vector<std::string>& , void*);

class CAwk
{
	struct _paramValue
	{
		int idx;
		double val;
		_paramValue(int i, double v):idx(i),val(v){};
	};
public:
	typedef std::list< std::vector<std::string> > DataSet;
	CAwk(const char* strContext);
	~CAwk(void);
	bool GetLine(std::string& strLine);
	int Patten(int column, const char* pattern = " ", const char* pszColumnName[] = NULL, bool matchColName  = true);
	std::list< std::vector<std::string> >& GetDataVector();
	//回调函数， 返回true 继续遍历   返回false  停止遍历
	void FetchData(AWKFUNC awkFunc, void *paramValue);
	//回调函数返回false，则将改行记录丢弃
	int FilterData(AWKFUNC awkFunc, void *paramValue);
	//修改数据集中的内容
	int UpdateData(AWKFUNC awkFunc, void *paramValue);

	
	int GetMaxInt(int idx);
	double GetMaxDouble(int idx);
	int GetSumInt(int idx);
	double GetSumDouble(int idx);
	int GetCountLine();
	int GetCountColumn();
private:
	static bool _Max(std::vector<std::string>& line, void* para);
	static bool _Sum(std::vector<std::string>& line, void* para);
	
public:
	//静态公开功能函数
	//字符串分割
	static std::vector< std::string> SplitString(const char* pszData, const char* pattern = " ", bool ignoreMul = false);
	//判断字符串是否double类型数据  参数nptr字符串可包含正负号、小数点或E(e)来表示指数部分。如123.456或123e-2
	static bool StrIsDouble(const char *nptr);
	//判断字符串是否int类型数据    参数base代表采用的进制方式
	static bool StrIsInt(const char *nptr, int base = 10);
private:
	int m_nPos;
	std::string m_strContext;
	DataSet m_vecData;
};

#endif //__AWK_H__