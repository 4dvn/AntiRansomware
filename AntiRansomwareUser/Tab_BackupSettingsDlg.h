#pragma once


// CTab_BackupSettingsDlg ��ȭ �����Դϴ�.

class CTab_BackupSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CTab_BackupSettingsDlg)

public:
	CTab_BackupSettingsDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CTab_BackupSettingsDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_BACKUPSETTINGSDLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};
