#ifndef __AWK_H__
#define __AWK_H__


/***********************************************************************************
* Copyright (c) 2010��2014, DingLi Communication Inc. All rights reserved.
* @brief   :�ַ������͡��ָ���
* @author  :zengtongqiao
* @date    :2013-07-17
* ʹ��ʾ����
	
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
	
	ifstream fin("��Ϊ��ϵͳ���ɲ�ѯLOG.txt");
	char szBuf[4096];
	fin.read(szBuf, sizeof(szBuf));

	CAwk awk(szBuf);
	const char* col[] = {"��λ��", "��ϵͳ��", "CPUռ����", "������ռ����"};
	awk.Patten(4, " ", col);
	//�Ի�ȡ�������ݼ����й��ˣ�CheckData��������false���н���������
	awk.FilterData(CheckData, NULL);
	double max = 0;
	//�����ݼ��л�ȡ����
	awk.FetchData(Max, &max);
************************************************************************************/
#include <string>
#include <vector>
#include <list>

//�ص�����   ��������ȡ������ʹ��
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
	//�ص������� ����true ��������   ����false  ֹͣ����
	void FetchData(AWKFUNC awkFunc, void *paramValue);
	//�ص���������false���򽫸��м�¼����
	int FilterData(AWKFUNC awkFunc, void *paramValue);
	//�޸����ݼ��е�����
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
	//��̬�������ܺ���
	//�ַ����ָ�
	static std::vector< std::string> SplitString(const char* pszData, const char* pattern = " ", bool ignoreMul = false);
	//�ж��ַ����Ƿ�double��������  ����nptr�ַ����ɰ��������š�С�����E(e)����ʾָ�����֡���123.456��123e-2
	static bool StrIsDouble(const char *nptr);
	//�ж��ַ����Ƿ�int��������    ����base������õĽ��Ʒ�ʽ
	static bool StrIsInt(const char *nptr, int base = 10);
private:
	int m_nPos;
	std::string m_strContext;
	DataSet m_vecData;
};

#endif //__AWK_H__