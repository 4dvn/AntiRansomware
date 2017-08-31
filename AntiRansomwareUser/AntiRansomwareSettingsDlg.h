#pragma once
#include "afxwin.h"


// CAntiRansomwareSettingsDlg ��ȭ �����Դϴ�.

class CAntiRansomwareSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CAntiRansomwareSettingsDlg)

public:
	CAntiRansomwareSettingsDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CAntiRansomwareSettingsDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANTIRANSOMWARESETTINGSDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CButton ctr_checkEnableBehaviorDetection;
	afx_msg void OnClose();
	CEdit ctr_editProtectionExt;
	CComboBox ctr_comboBackupDays;
	CButton ctr_checkEnableAutoUpdate;
	CButton ctr_checkEnableSignatureDetection;
	CEdit ctr_editUpdateTime;
	CComboBox ctr_comboRansomHandling;
};
