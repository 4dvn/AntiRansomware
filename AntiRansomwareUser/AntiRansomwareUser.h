
// AntiRansomwareUser.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CAntiRansomwareUserApp:
// �� Ŭ������ ������ ���ؼ��� AntiRansomwareUser.cpp�� �����Ͻʽÿ�.
//

class CAntiRansomwareUserApp : public CWinApp
{
public:
	CAntiRansomwareUserApp();
	ULONG_PTR m_gdiplusToken;

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CAntiRansomwareUserApp theApp;