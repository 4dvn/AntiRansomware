// AntiRansomwareReportDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "AntiRansomwareUser.h"
#include "AntiRansomwareReportDlg.h"
#include "afxdialogex.h"
#include "AntiRansomwareUserDlg.h"

extern CAntiRansomwareUserDlg *g_pParent;


// CAntiRansomwareReportDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CAntiRansomwareReportDlg, CDialog)

CAntiRansomwareReportDlg::CAntiRansomwareReportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ANTIRANSOMWAREREPORTDLG, pParent)
{

}

CAntiRansomwareReportDlg::~CAntiRansomwareReportDlg()
{
}

void CAntiRansomwareReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, ctr_listLog);
}


BEGIN_MESSAGE_MAP(CAntiRansomwareReportDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_LOG, &CAntiRansomwareReportDlg::OnLvnGetdispinfoListLog)
END_MESSAGE_MAP()


// CAntiRansomwareReportDlg �޽��� ó�����Դϴ�.


BOOL CAntiRansomwareReportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	RECT  rectParent;
	CRect rect;

	((CAntiRansomwareUserDlg*)AfxGetMainWnd())->GetWindowRect(&rectParent);
	GetClientRect(&rect); // ����� ���̾�α��� ������ ��´�.
	CPoint pos;

	// �������� ����
	pos.x = rectParent.right;
	pos.y = rectParent.top;

	// ǥ����ġ ����
	SetWindowPos(NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE);

	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOG);
	if (pList->GetSafeHwnd())
		pList->MoveWindow(10, 10, rect.right - 20, rect.bottom - 20);

	ctr_listLog.DeleteAllItems(); // ��� ������ ����
	ctr_listLog.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER); // List Control ��Ÿ�� ����

	ctr_listLog.InsertColumn(0, _T("Time"), LVCFMT_LEFT, 65, -1);
	ctr_listLog.InsertColumn(1, _T("Log"), LVCFMT_LEFT, 450, -1);

	m_numLog = 0;

	SetTimer(1, 100, 0); // �α� Ÿ�̸�

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


void CAntiRansomwareReportDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	CRect rect(10, 10, cx - 10, cy - 10);
	CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_LIST_LOG);

	if (pList->GetSafeHwnd())
		pList->MoveWindow(rect);
}

void CAntiRansomwareReportDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (nIDEvent == 1)
	{
		UpdateLogList();
	}

	CDialog::OnTimer(nIDEvent);
}


void CAntiRansomwareReportDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	DestroyWindow();

	CDialog::OnClose();
}


void CAntiRansomwareReportDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
}

void CAntiRansomwareReportDlg::OnLvnGetdispinfoListLog(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	//Create a pointer to the item
	LV_ITEM* pItem = &(pDispInfo)->item;

	//Which item number?
	int itemid = pItem->iItem;

	if (g_pParent->m_listScanLog.size() <= itemid)
		return;

	if (pItem->mask & LVIF_TEXT)
	{
		CString text;

		//Which column?
		if (pItem->iSubItem == 0) {	// TimeStamp
			text = g_pParent->m_listScanLog[itemid].timeStamp;
		}
		else if (pItem->iSubItem == 1) {	// Content
			text = g_pParent->m_listScanLog[itemid].content;
		}

		//Copy the text to the LV_ITEM structure
		//Maximum number of characters is in pItem->cchTextMax
		lstrcpyn(pItem->pszText, text, pItem->cchTextMax);

	}

	*pResult = 0;
}

void CAntiRansomwareReportDlg::UpdateLogList()
{
	int listNum;
	CString strMsg;

	EnterCriticalSection(&g_pParent->m_csScanLog);
	ctr_listLog.SetItemCountEx(g_pParent->m_listScanLog.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);

	listNum = g_pParent->m_listScanLog.size() - 1;
	if(listNum > m_numLog){
		ctr_listLog.EnsureVisible(listNum, false);
		m_numLog = listNum;
	}

	LeaveCriticalSection(&g_pParent->m_csScanLog);
}