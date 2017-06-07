#pragma once
#include "afxcmn.h"


// CAntiRansomwareReportDlg ��ȭ �����Դϴ�.

class CAntiRansomwareReportDlg : public CDialog
{
	DECLARE_DYNAMIC(CAntiRansomwareReportDlg)

public:
	CAntiRansomwareReportDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CAntiRansomwareReportDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANTIRANSOMWAREREPORTDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// Anti-Ransomware Report
public:
	int m_numLog;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnLvnGetdispinfoListLog(NMHDR *pNMHDR, LRESULT *pResult);
	void UpdateLogList();
	CListCtrl ctr_listLog;
};
