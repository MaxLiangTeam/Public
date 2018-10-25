#include "StdAfx.h"
#include "Awk.h"

CAwk::CAwk(const char* strContext):m_strContext(strContext),m_nPos(0)
{
}


CAwk::~CAwk(void)
{
}

bool CAwk::GetLine(std::string& strLine)
{
	int pos = m_strContext.find('\n', m_nPos);
	if (pos == std::string::npos)
	{
		return false;
	}
	strLine = m_strContext.substr(m_nPos, pos - m_nPos);
	m_nPos = pos + 1;
	return true;
}

//分割行
/****************************************************************************
* 匹配指定数量列的数据，ch为分隔符，pszColumnName为匹配的列名， 可以通过GetDataVector()获取到筛选后的数据
* 1、column为零时，表示不匹配列的数量， 所有行都将会保留
* 2、column大于零时，只有列数等于column的行才会保留
* 3、连续的分隔符会被当成一个分隔符
* 4、当指定pszColumnName列名列表时，column为数组的元素个数
* 5、当指定pszColumnName时，默认会校验列名，要求列名与数据行中的列的数量和名称必须一致，匹配到列名后，才会开始计数
*****************************************************************************/
int CAwk::Patten(int column, const char* pattern /* = " "*/, const char* pszColumnName[] /* = NULL*/, bool matchColName /* = true*/)
{
	int begPos = 0;
	int curPos = 0;
	m_nPos = 0;
	std::string strLine;
	std::vector<std::string> m_colName;
	if (pszColumnName != NULL)
	{
		for (int i = 0; i < column; i++)
		{
			m_colName.push_back(pszColumnName[i]);
		}
	}
	//第一次匹配到列后，方才保存数据
	bool isFirstMatch = false;
	if (!pszColumnName || !matchColName || 0 == column)
	{
		isFirstMatch = true;
	}

	while(GetLine(strLine))
	{
		std::vector<std::string> line = SplitString(strLine.c_str(), pattern, true);
		if (column > 0 && line.size() != column)
		{
			continue;
		}

		if (pszColumnName != NULL && matchColName)
		{
			if (line == m_colName)
			{
				isFirstMatch = true;
				continue;
			}
		}

		if (isFirstMatch)
		{
			m_vecData.push_back(line);
		}
	}
	return m_vecData.size();
}

//过滤   将不符合条件的数据剔除
int CAwk::FilterData(AWKFUNC awkFunc, void *paramValue)
{
	DataSet::iterator item = m_vecData.begin();
	int nCnt = 0;
	while(item != m_vecData.end())
	{
		if (!(*awkFunc)(*item, paramValue))
		{
			item = m_vecData.erase(item);
			nCnt++;
		}
		else
		{
			item++;
		}
		
	}
	return nCnt;
}

//提取   从数据集中提取想要的数据
void CAwk::FetchData(AWKFUNC awkFunc, void *paramValue)
{
	DataSet::iterator item = m_vecData.begin();
	for(; item != m_vecData.end(); item++)
	{
		(*awkFunc)(*item, paramValue);
	}
}

CAwk::DataSet& CAwk::GetDataVector()
{
	return m_vecData;
}


//ignoreMul = true 表示多个匹配符之间没有任何数据，将被忽略当成一个匹配符
std::vector< std::string> CAwk::SplitString(const char* pszData, const char* pattern, bool ignoreMul /*= false*/)
{

	std::string::size_type pos;
	std::vector<std::string> result;
	std::string str(pszData);
	str += pattern;//扩展字符串以方便操作
	int size = str.size();

	for(int i=0;i<size;i++)
	{
		pos=str.find(pattern,i);
		if (pos == std::string::npos)
		{
			break;
		}
		if(pos < size)
		{
			std::string s=str.substr(i,pos-i);
			if (ignoreMul)
			{
				if (s != pattern && !s.empty())
				{
					result.push_back(s);
				}
			}
			else
			{
				result.push_back(s);
			}	
			i=pos;
		}
	}
	return result;

}

//判断字符串是否double类型数据  参数nptr字符串可包含正负号、小数点或E(e)来表示指数部分。如123.456或123e-2
bool CAwk::StrIsDouble(const char *nptr)
{
	 char *endptr;
	 double val = strtod(nptr, &endptr);
	 if (endptr != NULL && strlen(endptr) > 0)
	 {
		 return false;
	 }
	 return true;
}
//判断字符串是否int类型数据    参数base代表采用的进制方式
bool CAwk::StrIsInt(const char *nptr, int base /*= 10*/)
{
	char *endptr = NULL;
	int val = strtol(nptr, &endptr, base);
	if (endptr != NULL && strlen(endptr) > 0)
	{
		return false;
	}
	return true;
}

int CAwk::GetMaxInt(int idx)
{
	_paramValue paramVal(idx, 0);
	FetchData(_Max, &paramVal);
	return (int)paramVal.val;
}

double CAwk::GetMaxDouble(int idx)
{
	_paramValue paramVal(idx, 0);
	FetchData(_Max, &paramVal);
	return paramVal.val;
}

int CAwk::GetSumInt(int idx)
{
	_paramValue paramVal(idx, 0);
	FetchData(_Sum, &paramVal);
	return (int)paramVal.val;
}

double CAwk::GetSumDouble(int idx)
{
	_paramValue paramVal(idx, 0);
	FetchData(_Sum, &paramVal);
	return paramVal.val;
}

int CAwk::GetCountLine()
{
	return m_vecData.size();
}

int CAwk::GetCountColumn()
{
	int colCount = 0;
	for(DataSet::iterator item = m_vecData.begin(); item != m_vecData.end(); item++)
	{
		std::vector<std::string> &line = *item;
		for (int i = 0; i < item->size(); i++)
		{
			colCount = item->size() > colCount ? item->size() : colCount;
		}
	}
	return colCount;
}

bool  CAwk::_Max(std::vector<std::string>& line, void* para)
{
	_paramValue *paramVal = (_paramValue*)para;
	double val = strtod( line[paramVal->idx].c_str(), NULL);
	if (val > paramVal->val)
	{
		paramVal->val = val;
	}
	return true;
}
bool  CAwk::_Sum(std::vector<std::string>& line, void* para)
{
	_paramValue *paramVal = (_paramValue*)para;
	double val = strtod( line[paramVal->idx].c_str(), NULL);
	paramVal->val += val;
	return true;
}
