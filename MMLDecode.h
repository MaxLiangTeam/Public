#pragma once

#include <map>
#include "vector"
using namespace std;

class CMMLDecode
{
public:
	CMMLDecode(void);
public:
	virtual ~CMMLDecode(void);

	static int Decode(CString strCmd,const char* strData,vector<map<CString,CString>>& vecValues);
};
