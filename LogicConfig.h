#pragma once
#include <map>
#include <vector>
#include <string>
#include "Include.h"

#include "ScriptEngine.h"

class LogicConfig
{
	friend class CDispose;
public:
	LogicConfig();
	~LogicConfig(void);


	//业务处理配置加载
	bool LoadConfig();
	bool LoadFormulaConfig();
	bool LoadTaskConfig();
	bool LoadActionConfig();
	bool LoadParamConfig();
	bool LoadWorkConfig();
private:
	int m_nKpiMun;
	std::map<CString, std::map<CString, CString> > m_formulaConf;
	std::map<CString, CString> m_workConf;
	std::map<CString, tagTaskConfig> m_taskConf;
	std::map<CString, std::vector<CString> > m_actionConf; 
	std::map<CString, tagCmdParamConfig > m_paramConf; 

};

