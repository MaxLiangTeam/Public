#pragma once
#include <string>
#include <map>

#include <lua.hpp>

class CScriptEngine
{
public:
	CScriptEngine(void);
	~CScriptEngine(void);

	void SetParam(const char* pszKey, double val);
	void SetParam(const char* pszKey, const char* val);
	void SetParam(std::map<CString, double> mapParamVal);
	void SetParam(std::map<CString, CString> mapParamVal);
	//运行表达式值
	bool ExecExpression(const char* pszExp);
	bool ExecScript(const char* pszExp);

	//获取逻辑布尔运算值
	bool GetBoolCalc();

	//获取表达式值
	double GetExprVal();

	//获取错误信息
	CString GetErrorMsg();
	
	//获取指标集
	std::map<CString, double> GetFormula(std::map<CString, CString>& mapFormula, CString& errorMsg);

	//注册函数
	void RegisterFunction(const char* pszFuncName, lua_CFunction ptrFunc);
private:
	lua_State* m_luaHandle;
};

