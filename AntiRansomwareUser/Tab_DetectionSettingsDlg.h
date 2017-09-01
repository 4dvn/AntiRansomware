#pragma once
#include "afxwin.h"


// CTab_DetectionSettingsDlg ��ȭ �����Դϴ�.

class CTab_DetectionSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CTab_DetectionSettingsDlg)

public:
	CTab_DetectionSettingsDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CTab_DetectionSettingsDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_DETECTIONSETTINGSDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CButton ctr_checkEnableBehaviorDetection;
	CEdit ctr_editProtectionExt;
	CComboBox ctr_comboRansomHandling;
	CButton ctr_checkEnableSignatureDetection;
	CButton ctr_checkEnableAutoUpdate;
	CEdit ctr_editUpdateTime;
	virtual BOOL OnInitDialog();
};
