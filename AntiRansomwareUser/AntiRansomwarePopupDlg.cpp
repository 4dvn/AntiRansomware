// AntiRansomwarePopupDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "AntiRansomwareUser.h"
#include "AntiRansomwarePopupDlg.h"
#include "afxdialogex.h"


// CAntiRansomwarePopupDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CAntiRansomwarePopupDlg, CDialog)

CAntiRansomwarePopupDlg::CAntiRansomwarePopupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_ANTIRANSOMWAREPOPUPDLG, pParent)
{

}

CAntiRansomwarePopupDlg::~CAntiRansomwarePopupDlg()
{
}

void CAntiRansomwarePopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAntiRansomwarePopupDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CAntiRansomwarePopupDlg �޽��� ó�����Դϴ�.


BOOL CAntiRansomwarePopupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.  
	CRect rect;
	CRect cRect;

	GetClientRect(cRect);
	int m_Desktowidth = GetSystemMetrics(SM_CXSCREEN);
	int m_DesktopHeight = GetSystemMetrics(SM_CYSCREEN);

	HWND hTrayWnd = ::FindWindow("Shell_TrayWnd", NULL);
	if (hTrayWnd != NULL) {
		::GetWindowRect(hTrayWnd, &rect);
		if (rect.top > cRect.Height())
			SetWindowPos(NULL, m_Desktowidth - cRect.Width() - 10, rect.top - cRect.Height() - 10, 0, 0, SWP_NOSIZE);
		else
			SetWindowPos(NULL, m_Desktowidth - cRect.Width() - 10, m_DesktopHeight - cRect.Height() - 10, 0, 0, SWP_NOSIZE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


BOOL CAntiRansomwarePopupDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (!m_background.m_hObject)
		return true;

	CRect rect;
	GetClientRect(&rect);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap* pOldBitmap = dc.SelectObject(&m_background);

	BITMAP bmap;
	m_background.GetBitmap(&bmap);
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);

	//return CDialog::OnEraseBkgnd(pDC);
	return true;
}


void CAntiRansomwarePopupDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (point.y < 37)
		SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, 0);

	CDialog::OnLButtonDown(nFlags, point);
}


void CAntiRansomwarePopupDlg::InitPopupWindow(int type)
{
	m_typePopup = type;

	switch (m_typePopup)
	{
		case 0:
			this->SetWindowTextA("�������� Ž��");
			m_background.LoadBitmapA(IDB_BITMAP_POPUP_RED);
			break;
		case 1:
			this->SetWindowTextA("�˸�");
			m_background.LoadBitmapA(IDB_BITMAP_POPUP_BLUE);
			break;
	}
}