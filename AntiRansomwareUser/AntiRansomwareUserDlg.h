
// AntiRansomwareUserDlg.h : ��� ����
//

#pragma once
#include "AntiRansomwareReportDlg.h"
#include "AntiRansomwareTable.h"
#include "scanuser.h"
#include "afxwin.h"

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
public:
	int m_isRunning;

	CWinThread*	pThreadCommunication;

	vector<SCAN_LOG> m_listScanLog; // packet list

	CRITICAL_SECTION m_csScanLog;

	map<unsigned int, PROCESS_BEHAVIOR> m_mapProcessBehavior;
	list<ITEM_NEW_FILE> m_listNewFile;
	list<ITEM_WRITE_FILE> m_listWriteFile;
	list<ITEM_RENAME_FILE> m_listRenameFile;
	list<ITEM_DELETE_FILE> m_listDeleteFile;
	list<ITEM_BACKUP_FILE> m_listBackupFile;

	int m_numNewFile;
	int m_numWriteFile;
	int m_numRenameFile;
	int m_numDeleteFile;
	int m_numBackupFile;

// �����Դϴ�.
protected:
	HICON m_hIcon;

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
	bool InitMyScanner();
	static UINT CommunicationMyScanner(LPVOID lpParam);
	int RecordProcessBehavior(PSCANNER_NOTIFICATION notification);
	bool RecoveryProcessBehavior(DWORD pid);
	bool AddEventNewFile(DWORD pid, bool isDirectory, CString strPath);
	bool AddEventRenameFile(DWORD pid, CString strSrc, CString strDst);
	bool AddEventWriteFile(DWORD pid, CString strPath);
	bool AddEventDeleteFile(DWORD pid, CString strPath);
	unsigned int AddEventBackupFile(DWORD pid, CString strPath);
	bool DoRecoveryFile(unsigned int num_back, CString strPath);
	bool DoBackupFile(CString strPath);
	bool DoCheckRansomware(CString strPath);
	bool DoKillRecoveryRansomware(DWORD pid);
	CString GetBackupFilePath(CString strPath);
	CEdit ctr_editTargetPid;
};
