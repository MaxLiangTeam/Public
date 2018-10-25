#include "StdAfx.h"
#include "ScriptEngine.h"

#ifdef _DEBUG
#pragma  comment(lib, "lua_d.lib")
#else
#pragma  comment(lib, "lua.lib")
#endif

CScriptEngine::CScriptEngine(void)
{
	m_luaHandle = luaL_newstate();
	luaopen_base(m_luaHandle);         /* opens the basic library */
	luaopen_table(m_luaHandle);        /* opens the table library */
	luaopen_io(m_luaHandle);           /* opens the I/O library */
	luaopen_string(m_luaHandle);       /* opens the string lib. */
	luaopen_math(m_luaHandle);         /* opens the math lib. */
}


CScriptEngine::~CScriptEngine(void)
{
	lua_close(m_luaHandle);
}

void CScriptEngine::SetParam(const char* pszKey, double val)
{
	lua_pushnumber(m_luaHandle, val);
	lua_setglobal(m_luaHandle, pszKey); 
}

void CScriptEngine::SetParam(const char* pszKey, const char* val)
{
	lua_pushstring(m_luaHandle, val);
	lua_setglobal(m_luaHandle, pszKey); 
}

void CScriptEngine::SetParam(std::map<CString, double> mapParamVal)
{
	for (auto item = mapParamVal.begin(); item != mapParamVal.end(); item++)
	{
		SetParam(item->first.GetString(), item->second);
	}
}


void CScriptEngine::SetParam(std::map<CString, CString> mapParamVal)
{
	for (auto item = mapParamVal.begin(); item != mapParamVal.end(); item++)
	{
		SetParam(item->first.GetString(), item->second.GetString());		
	}
}

//获取表达式值
bool CScriptEngine::ExecExpression(const char* pszExp)
{
	CString strExp("ExpResult = (");
	strExp += pszExp;
	strExp += ")";
	return !luaL_dostring(m_luaHandle, strExp.GetBuffer());
}

bool CScriptEngine::ExecScript(const char* pszExp)
{
	return !luaL_dostring(m_luaHandle, pszExp);
}

//获取逻辑布尔运算值
bool CScriptEngine::GetBoolCalc()
{
	lua_getglobal(m_luaHandle, "ExpResult");
	int bCond = lua_toboolean(m_luaHandle,-1);
	lua_pop(m_luaHandle, 1);
	return !!bCond;
}

//获取表达式值
double CScriptEngine::GetExprVal()
{
	lua_getglobal(m_luaHandle, "ExpResult");
	double fVal = lua_tonumber(m_luaHandle,-1);
	lua_pop(m_luaHandle, 1);
	return fVal;
}

CString CScriptEngine::GetErrorMsg()
{
	CString strErrMsg = lua_tostring(m_luaHandle, -1);
	lua_pop(m_luaHandle, 1);/* pop error message from the stack */
	return strErrMsg;
}

//获取指标集
std::map<CString, double> CScriptEngine::GetFormula(std::map<CString, CString>& mapFormula, CString& errorMsg)
{
	std::map<CString, double> formalVal;
	for (auto item = mapFormula.begin(); item != mapFormula.end(); item++)
	{
		if (ExecExpression(item->second.GetBuffer()))
		{
			formalVal[item->first] = GetExprVal();
		}
		else
		{
			errorMsg += GetErrorMsg();
			errorMsg += "\n";			
		}
	}
	return formalVal;
}

void CScriptEngine::RegisterFunction(const char* pszFuncName, lua_CFunction ptrFunc)
{
	lua_register(m_luaHandle, pszFuncName, ptrFunc);
}