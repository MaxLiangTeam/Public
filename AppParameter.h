#pragma once
#include <map>
#include <string>


#define  GETPARAM(group, param, defaultVal) CAppParameter::GetInstance()->GetParam(group, param, defaultVal)

//参数转换类
template<typename T>
class ConvertParam
{
public:
	ConvertParam(const std::string& paramValue):m_paramValue(paramValue)
	{
	}
	T operator()()
	{
		return m_paramValue.c_str();
	}
public:
	const std::string& m_paramValue; 
};

//参数转换（特化）类
template<>
class ConvertParam<int>
{
public:
	ConvertParam(const std::string& paramValue):m_paramValue(paramValue)
	{
	}
	int operator()()
	{
		return atoi(m_paramValue.c_str());
	}
public:
	const std::string& m_paramValue; 
};

//参数转换（特化）类
template<>
class ConvertParam<double>
{
public:
	ConvertParam(const std::string& paramValue):m_paramValue(paramValue)
	{
	}
	double operator()()
	{
		return atof(m_paramValue.c_str());
	}
public:
	const std::string& m_paramValue; 
};

class CADODatabase;
class CAppParameter
{
public:
	static CAppParameter* GetInstance();
	bool LoadParameter(const char* pszDbAddr, const char* pszDbName, const char* pszDbUser, const char* pszDbPassword, const char* pszTableName);
	
	template<typename T>
	T GetParam(const char* pszGroup , const char* pszParam, T value)
	{
		std::string unionKey = pszGroup;
		unionKey += pszParam;
		if (m_paramList.find(unionKey) == m_paramList.end())
		{
			return value;
		}
		return ConvertParam<T>(m_paramList[unionKey])();
	}

	~CAppParameter(void);
private:
	CAppParameter(void);
	bool LoadParamValue(CADODatabase* db, const char* pszTableName);

	std::map<std::string, std::string> m_paramList;
};

