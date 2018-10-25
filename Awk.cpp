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

//�ָ���
/****************************************************************************
* ƥ��ָ�������е����ݣ�chΪ�ָ�����pszColumnNameΪƥ��������� ����ͨ��GetDataVector()��ȡ��ɸѡ�������
* 1��columnΪ��ʱ����ʾ��ƥ���е������� �����ж����ᱣ��
* 2��column������ʱ��ֻ����������column���вŻᱣ��
* 3�������ķָ����ᱻ����һ���ָ���
* 4����ָ��pszColumnName�����б�ʱ��columnΪ�����Ԫ�ظ���
* 5����ָ��pszColumnNameʱ��Ĭ�ϻ�У��������Ҫ���������������е��е����������Ʊ���һ�£�ƥ�䵽�����󣬲ŻῪʼ����
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
	//��һ��ƥ�䵽�к󣬷��ű�������
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

//����   �������������������޳�
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

//��ȡ   �����ݼ�����ȡ��Ҫ������
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


//ignoreMul = true ��ʾ���ƥ���֮��û���κ����ݣ��������Ե���һ��ƥ���
std::vector< std::string> CAwk::SplitString(const char* pszData, const char* pattern, bool ignoreMul /*= false*/)
{

	std::string::size_type pos;
	std::vector<std::string> result;
	std::string str(pszData);
	str += pattern;//��չ�ַ����Է������
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

//�ж��ַ����Ƿ�double��������  ����nptr�ַ����ɰ��������š�С�����E(e)����ʾָ�����֡���123.456��123e-2
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
//�ж��ַ����Ƿ�int��������    ����base������õĽ��Ʒ�ʽ
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
