// ZteLteAdjPro.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CZteLteAdjProApp:
// �йش����ʵ�֣������ ZteLteAdjPro.cpp
//

class CZteLteAdjProApp : public CWinApp
{
public:
	CZteLteAdjProApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CZteLteAdjProApp theApp;