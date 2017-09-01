// Tab_DetectionSettingsDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "AntiRansomwareUser.h"
#include "Tab_DetectionSettingsDlg.h"
#include "afxdialogex.h"


// CTab_DetectionSettingsDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CTab_DetectionSettingsDlg, CDialog)

CTab_DetectionSettingsDlg::CTab_DetectionSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_TAB_DETECTIONSETTINGSDLG, pParent)
{

}

CTab_DetectionSettingsDlg::~CTab_DetectionSettingsDlg()
{
}

void CTab_DetectionSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_ENABLE_BEHAVIOR_DETECTION, ctr_checkEnableBehaviorDetection);
	DDX_Control(pDX, IDC_EDIT_PROTECTION_EXT, ctr_editProtectionExt);
	DDX_Control(pDX, IDC_COMBO_RANSOM_HANDLING, ctr_comboRansomHandling);
	DDX_Control(pDX, IDC_CHECK_ENABLE_SIGNATURE_DETECTION, ctr_checkEnableSignatureDetection);
	DDX_Control(pDX, IDC_CHECK_ENABLE_AUTOUPDATE, ctr_checkEnableAutoUpdate);
	DDX_Control(pDX, IDC_EDIT_UPDATE_TIME, ctr_editUpdateTime);
}


BEGIN_MESSAGE_MAP(CTab_DetectionSettingsDlg, CDialog)
END_MESSAGE_MAP()


// CTab_DetectionSettingsDlg �޽��� ó�����Դϴ�.


BOOL CTab_DetectionSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	// ������� Ž��
	ctr_checkEnableBehaviorDetection.SetCheck(1);

	ctr_editProtectionExt.SetWindowTextA("txt,hwp,doc,docx,ppt,pptx,xls,xlsx,c,cpp,h,hpp,bmp,jpg,gif,png,zip,rar");

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
