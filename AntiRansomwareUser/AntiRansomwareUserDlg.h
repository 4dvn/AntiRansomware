
// AntiRansomwareUserDlg.h : ��� ����
//

#pragma once
#include "afxwin.h"
#include "AntiRansomwareReportDlg.h"
#include "AntiRansomwareRecord.h"
#include "scanuser.h"

#include "GdipButton.h"

const UINT WM_INITIALIZATION_COMPLETED = ::RegisterWindowMessage("WM_INITIALIZATION_COMPLETED");

typedef struct sScanLog {
	CString timeStamp;
	CString content;
}  SCAN_LOG;


// CAntiRansomwareUserDlg ��ȭ ����
class CAntiRansomwareUserDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CAntiRansomwareUserDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANTIRANSOMWAREUSER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

// MyScanner
private:
	CWinThread*	pThreadCommunication;
	list<CString> m_listFileExt; // ���� Ȯ����
	CRITICAL_SECTION m_csFileExt;

public:
	int m_isRunning;

	HANDLE m_hEventCheckRansomware;
	CRITICAL_SECTION m_csScanLog;
	CRITICAL_SECTION m_csFileQueue;

	vector<SCAN_LOG> m_listScanLog; // packet list
	list<ITEM_CHECK_FILE> m_listCheckFile;

	map<unsigned int, ArProcessBehavior*> m_mapProcessBehavior;

	int m_nCountMonitor;

	// ��� �� ���
	CString m_strBackingUpPath;

// �����Դϴ�.
protected:
	HICON m_hIcon;
	CBitmap m_background;

	CGdipButton	m_gbtnClose;
	CGdipButton	m_gbtnMinimum;
	CGdipButton	m_gbtnMenu2;
	CGdipButton	m_gbtnMenu3;
	CGdipButton	m_gbtnMenu4;
	CGdipButton	m_gbtnMenu5;
	CGdipButton	m_gbtnToggle1; // �������
	CGdipButton	m_gbtnToggle2; // �ñ״�ó

	CBitmap lamp_image;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CAntiRansomwareReportDlg m_pAntiRansomwareReportDlg;
	LRESULT OnInitializationCompleted(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonViewreport();
	afx_msg void OnBnClickedButtonRecovery();
	void AddLogList(CString msg, bool wTime = false);
	bool InitDialogUI();
	bool InitMyScanner();
	void SetFileExtList();
	bool AddCheckFileExtension(CString strExt);
	bool DoCheckFileExtension(CString strPath);
	static UINT CommunicationMyScanner(LPVOID lpParam);
	int RecordProcessBehavior(PSCANNER_NOTIFICATION notification);
	int GetPermissionDirectory(CString strPath, DWORD pid = 0);
	int DoCheckRansomware(CString strPath);
	bool DoKillRecoveryRansomware(DWORD pid);
	bool AddCheckRansomwareFile(DWORD pid, CString strPath);
	CString GetBackupFilePath(CString strPath);
	CEdit ctr_editTargetPid;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnBnClickedButtonClose();
	CStatic ctr_picMenu1;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	bool SetDetectionEngine();
	afx_msg void OnBnClickedButtonToggle1();
	afx_msg void OnBnClickedButtonToggle2();
};
