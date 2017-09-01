// AntiRansomwareSettingsDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "AntiRansomwareUser.h"
#include "AntiRansomwareSettingsDlg.h"
#include "afxdialogex.h"


// CAntiRansomwareSettingsDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CAntiRansomwareSettingsDlg, CDialog)

CAntiRansomwareSettingsDlg::CAntiRansomwareSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ANTIRANSOMWARESETTINGSDLG, pParent)
{

}

CAntiRansomwareSettingsDlg::~CAntiRansomwareSettingsDlg()
{
}

void CAntiRansomwareSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_SETTINGS, ctr_tabSettings);
}


BEGIN_MESSAGE_MAP(CAntiRansomwareSettingsDlg, CDialog)
	ON_WM_CLOSE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SETTINGS, &CAntiRansomwareSettingsDlg::OnTcnSelchangeTabSettings)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB_SETTINGS, &CAntiRansomwareSettingsDlg::OnTcnSelchangingTabSettings)
	ON_BN_CLICKED(IDC_BUTTON_CONFIRM, &CAntiRansomwareSettingsDlg::OnBnClickedButtonConfirm)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CAntiRansomwareSettingsDlg::OnBnClickedButtonCancel)
END_MESSAGE_MAP()


// CAntiRansomwareSettingsDlg �޽��� ó�����Դϴ�.


BOOL CAntiRansomwareSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	CString strTab = _T("");

	strTab.Format(_T("Ž�� ����"));
	ctr_tabSettings.InsertItem(0, strTab, 0);

	strTab.Format(_T("��� ����"));
	ctr_tabSettings.InsertItem(1, strTab, 0);

	strTab.Format(_T("��Ÿ ����"));
	ctr_tabSettings.InsertItem(2, strTab, 0);

	CRect Rect;
	ctr_tabSettings.GetClientRect(&Rect);

	m_TabDetectionSettings.Create(IDD_TAB_DETECTIONSETTINGSDLG, &ctr_tabSettings);
	m_TabDetectionSettings.SetWindowPos(NULL, 2, 23, Rect.Width() - 6, Rect.Height() - 26, SWP_SHOWWINDOW | SWP_NOZORDER);
	m_pwndShow = &m_TabDetectionSettings;

	m_TabBackupSettings.Create(IDD_TAB_BACKUPSETTINGSDLG, &ctr_tabSettings);
	m_TabBackupSettings.SetWindowPos(NULL, 2, 23, Rect.Width() - 6, Rect.Height() - 26, SWP_NOZORDER);
	m_pwndShow = &m_TabBackupSettings;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


void CAntiRansomwareSettingsDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	DestroyWindow();

	CDialog::OnClose();
}


void CAntiRansomwareSettingsDlg::OnTcnSelchangeTabSettings(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	int nIndex = ctr_tabSettings.GetCurSel();

	m_pwndShow = NULL;
	m_TabDetectionSettings.ShowWindow(SW_HIDE);
	m_TabBackupSettings.ShowWindow(SW_HIDE);

	switch (nIndex)
	{
		case 0:
			m_TabDetectionSettings.ShowWindow(SW_SHOW);
			break;
		case 1:
			m_TabBackupSettings.ShowWindow(SW_SHOW);
			break;
	}

	*pResult = 0;
}


void CAntiRansomwareSettingsDlg::OnTcnSelchangingTabSettings(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_prevCurSel = ctr_tabSettings.GetCurSel();
	*pResult = 0;
}


void CAntiRansomwareSettingsDlg::OnBnClickedButtonConfirm()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	EndDialog(IDOK);
	DestroyWindow();
}


void CAntiRansomwareSettingsDlg::OnBnClickedButtonCancel()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	EndDialog(IDCANCEL);
	DestroyWindow();
}
