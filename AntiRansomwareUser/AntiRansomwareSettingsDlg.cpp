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
	DDX_Control(pDX, IDC_CHECK_ENABLE_BEHAVIOR_DETECTION, ctr_checkEnableBehaviorDetection);
	DDX_Control(pDX, IDC_EDIT_PROTECTION_EXT, ctr_editProtectionExt);
	DDX_Control(pDX, IDC_COMBO_BACKUP_DAYS, ctr_comboBackupDays);
	DDX_Control(pDX, IDC_CHECK_ENABLE_AUTOUPDATE, ctr_checkEnableAutoUpdate);
	DDX_Control(pDX, IDC_CHECK_ENABLE_SIGNATURE_DETECTION, ctr_checkEnableSignatureDetection);
	DDX_Control(pDX, IDC_EDIT_UPDATE_TIME, ctr_editUpdateTime);
	DDX_Control(pDX, IDC_COMBO_RANSOM_HANDLING, ctr_comboRansomHandling);
}


BEGIN_MESSAGE_MAP(CAntiRansomwareSettingsDlg, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CAntiRansomwareSettingsDlg �޽��� ó�����Դϴ�.


BOOL CAntiRansomwareSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	// ������� Ž��
	ctr_checkEnableBehaviorDetection.SetCheck(1);

	ctr_editProtectionExt.SetWindowTextA("txt,hwp,doc,docx,ppt,pptx,xls,xlsx,c,cpp,h,hpp,bmp,jpg,gif,png,zip,rar");

	ctr_comboBackupDays.AddString("1");
	ctr_comboBackupDays.AddString("3");
	ctr_comboBackupDays.AddString("7");
	ctr_comboBackupDays.AddString("14");
	ctr_comboBackupDays.AddString("30");
	ctr_comboBackupDays.AddString("180");
	ctr_comboBackupDays.AddString("365");
	ctr_comboBackupDays.SetCurSel(2);

	ctr_comboRansomHandling.AddString("����");
	ctr_comboRansomHandling.AddString("����� ó��");
	ctr_comboRansomHandling.SetCurSel(0);

	// �ñ״�ó Ž��
	ctr_checkEnableSignatureDetection.SetCheck(1);
	ctr_checkEnableAutoUpdate.SetCheck(1);
	ctr_editUpdateTime.SetWindowTextA("3");

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


void CAntiRansomwareSettingsDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	DestroyWindow();

	CDialog::OnClose();
}
