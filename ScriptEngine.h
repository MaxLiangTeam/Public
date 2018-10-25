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
	//���б��ʽֵ
	bool ExecExpression(const char* pszExp);
	bool ExecScript(const char* pszExp);

	//��ȡ�߼���������ֵ
	bool GetBoolCalc();

	//��ȡ���ʽֵ
	double GetExprVal();

	//��ȡ������Ϣ
	CString GetErrorMsg();
	
	//��ȡָ�꼯
	std::map<CString, double> GetFormula(std::map<CString, CString>& mapFormula, CString& errorMsg);

	//ע�ắ��
	void RegisterFunction(const char* pszFuncName, lua_CFunction ptrFunc);
private:
	lua_State* m_luaHandle;
};

