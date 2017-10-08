
// AntiRansomwareUserDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "AntiRansomwareUser.h"
#include "AntiRansomwareUserDlg.h"
#include "afxdialogex.h"

#include "MemDC.h"

// ���� �м� & ������Ʈ ����
#define SERVER_IP "192.168.0.4"
#define SERVER_PORT "12345"

const char* g_szBackupPath = "\\_SafeBackup"; // ��� ����
const char* g_szBackupExt = "exe,txt,hwp,doc,docx,ppt,pptx,xls,xlsx,c,cpp,h,hpp,bmp,jpg,gif,png,zip,rar"; // ��ȣ ���� Ȯ����

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SYSTEMTIME g_time; // �ð� ����ü.

HWND hWnd;
CAntiRansomwareUserDlg *g_pParent;

CWinThread*	pThreadCheckRansomware = NULL;
static UINT CheckRansomwareWorker(LPVOID lpParam); // ���� ��ȣȭ ���� ������

//Ÿ�̸� �ڵ鷯 ����
MMRESULT g_idMNTimer1;
void CALLBACK OnTimerFunc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2);

UCHAR FoulString[] = "foul";

DWORD FindRansomwareParantPID(DWORD pid);
BOOL DoKillProcessTree(DWORD pid);

// CAntiRansomwareUserDlg ��ȭ ����



CAntiRansomwareUserDlg::CAntiRansomwareUserDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ANTIRANSOMWAREUSER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAntiRansomwareUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TARGET_PID, ctr_editTargetPid);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_gbtnClose);
	DDX_Control(pDX, IDC_BUTTON_MINIMUM, m_gbtnMinimum);
	DDX_Control(pDX, IDC_PICTURE_MENU1, ctr_picMenu1);
	DDX_Control(pDX, IDC_BUTTON_MENU2, m_gbtnMenu2);
	DDX_Control(pDX, IDC_BUTTON_MENU3, m_gbtnMenu3);
	DDX_Control(pDX, IDC_BUTTON_MENU4, m_gbtnMenu4);
	DDX_Control(pDX, IDC_BUTTON_MENU5, m_gbtnMenu5);
	DDX_Control(pDX, IDC_BUTTON_TOGGLE1, m_gbtnToggle1);
	DDX_Control(pDX, IDC_BUTTON_TOGGLE2, m_gbtnToggle2);
	DDX_Control(pDX, IDC_BUTTON_ViewReport, m_gbtnMenuB1);
	
}

BEGIN_MESSAGE_MAP(CAntiRansomwareUserDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(WM_INITIALIZATION_COMPLETED, OnInitializationCompleted) // WM_INITIALIZATION_COMPLETED
	ON_REGISTERED_MESSAGE(WM_POPUP_INFO_WINDOW, OnPopupInfoWindow) // WM_POPUP_INFO_WINDOW
	ON_BN_CLICKED(IDC_BUTTON_ViewReport, &CAntiRansomwareUserDlg::OnBnClickedButtonViewreport)
	ON_WM_MOVING()
	ON_BN_CLICKED(IDC_BUTTON_RECOVERY, &CAntiRansomwareUserDlg::OnBnClickedButtonRecovery)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CAntiRansomwareUserDlg::OnBnClickedButtonClose)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BUTTON_TOGGLE1, &CAntiRansomwareUserDlg::OnBnClickedButtonToggle1)
	ON_BN_CLICKED(IDC_BUTTON_TOGGLE2, &CAntiRansomwareUserDlg::OnBnClickedButtonToggle2)
	ON_BN_CLICKED(IDC_BUTTON_MENU2, &CAntiRansomwareUserDlg::OnBnClickedButtonMenu2)
	ON_BN_CLICKED(IDC_BUTTON_MENU3, &CAntiRansomwareUserDlg::OnBnClickedButtonMenu3)
	ON_BN_CLICKED(IDC_BUTTON_MENU4, &CAntiRansomwareUserDlg::OnBnClickedButtonMenu4)
	ON_BN_CLICKED(IDC_BUTTON_MENU5, &CAntiRansomwareUserDlg::OnBnClickedButtonMenu5)
	ON_BN_CLICKED(IDC_BUTTON_MINIMUM, &CAntiRansomwareUserDlg::OnBnClickedButtonMinimum)
END_MESSAGE_MAP()


// CAntiRansomwareUserDlg �޽��� ó����

BOOL CAntiRansomwareUserDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	hWnd = AfxGetMainWnd()->m_hWnd; // GET MAIN HANDLE
	g_pParent = this; // �θ� ��ü ����

	TCHAR szPath[_MAX_PATH];
	GetModuleFileName(AfxGetInstanceHandle(), szPath, _MAX_PATH); // ���� �������� ��� ���ϱ�
	appPath = szPath;
	int nPos = appPath.ReverseFind('\\'); // �������� ��ο��� ���ϸ� ����
	if (nPos > 0)
		appPath = appPath.Left(nPos);

	// Init UI
	InitDialogUI();

	m_listScanLog.clear();

	// CRITICAL SECTION - Initial
	InitializeCriticalSection(&m_csFileExt);
	InitializeCriticalSection(&m_csScanLog);
	InitializeCriticalSection(&m_csFileQueue);

	m_strBackingUpPath.Empty();

	InitMyScanner();

	m_nCountMonitor = 0;

	// Popup Message
	m_sPopupMessage.pid = 0;
	m_sPopupMessage.strProcName = "";
	m_sPopupMessage.strProcPath = "";

	/********** ��Ƽ�̵�� Ÿ�̸� ���� **********/
	//System���� ������ Resolution�� ���Ѵ�.
	//��Ƽ�̵�� Ÿ�̸��� Resolution�� �ּҷ� �ϱ� ���� �ڵ�    
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	unsigned int Resolution = min(max(tc.wPeriodMin, 0), tc.wPeriodMax);
	timeBeginPeriod(Resolution);

	//��Ƽ�̵�� Ÿ�̸� ����
	g_idMNTimer1 = timeSetEvent(
		1000,         //Timer Delay(ms)
		Resolution,  //delay�� ��Ȯ���� �ǹ�
		OnTimerFunc,  //callback Function
		(DWORD)this,   //CallBack�� ����� ���� ����
		TIME_PERIODIC);  //Ÿ�̸��� Ÿ��(�߻��ø��� �ݹ� Call)

						 /********** ��Ƽ�̵�� Ÿ�̸� �� **********/

	PostMessageA(WM_INITIALIZATION_COMPLETED, NULL, NULL); // WM_INITIALIZATION_COMPLETED

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

bool CAntiRansomwareUserDlg::InitDialogUI()
{
	this->SetWindowTextA("Anti-Ransomware");

	m_background.LoadBitmapA(IDB_BITMAP_MAIN);

	m_gbtnClose.LoadStdImage(IDB_PNG_CLOSE_NORMAL, _T("PNG"));
	m_gbtnClose.LoadSthImage(IDB_PNG_CLOSE_ACTIVE, _T("PNG"));
	m_gbtnMinimum.LoadStdImage(IDB_PNG_MINIMUM_NORMAL, _T("PNG"));
	m_gbtnMinimum.LoadSthImage(IDB_PNG_MINIMUM_ACTIVE, _T("PNG"));

	// MENU1
	lamp_image.LoadBitmap(IDB_BITMAP_MENU1_SAFETY);
	HBITMAP h_old_bitmap = ctr_picMenu1.SetBitmap(lamp_image);
	if (h_old_bitmap != NULL) ::DeleteObject(h_old_bitmap);
	lamp_image.Detach();

	m_gbtnMenu2.LoadStdImage(IDB_PNG_MENU2_NORMAL, _T("PNG"));
	m_gbtnMenu2.LoadSthImage(IDB_PNG_MENU2_ACTIVE, _T("PNG"));
	m_gbtnMenu3.LoadStdImage(IDB_PNG_MENU3_NORMAL, _T("PNG"));
	m_gbtnMenu3.LoadSthImage(IDB_PNG_MENU3_ACTIVE, _T("PNG"));
	m_gbtnMenu4.LoadStdImage(IDB_PNG_MENU4_NORMAL, _T("PNG"));
	m_gbtnMenu4.LoadSthImage(IDB_PNG_MENU4_ACTIVE, _T("PNG"));
	m_gbtnMenu5.LoadStdImage(IDB_PNG_MENU5_NORMAL, _T("PNG"));
	m_gbtnMenu5.LoadSthImage(IDB_PNG_MENU5_ACTIVE, _T("PNG"));

	m_gbtnToggle1.LoadStdImage(IDB_PNG_TOGGLE_OFF_NORMAL, _T("PNG"));
	m_gbtnToggle1.LoadSthImage(IDB_PNG_TOGGLE_OFF_ACTIVE, _T("PNG"));
	m_gbtnToggle1.LoadAltImage(IDB_PNG_TOGGLE_ON_NORMAL, _T("PNG"));
	m_gbtnToggle1.LoadAlhImage(IDB_PNG_TOGGLE_ON_ACTIVE, _T("PNG"));
	m_gbtnToggle1.EnableToggle(TRUE);
	m_gbtnToggle1.SetToggle(TRUE);

	m_gbtnToggle2.LoadStdImage(IDB_PNG_TOGGLE_OFF_NORMAL, _T("PNG"));
	m_gbtnToggle2.LoadSthImage(IDB_PNG_TOGGLE_OFF_ACTIVE, _T("PNG"));
	m_gbtnToggle2.LoadAltImage(IDB_PNG_TOGGLE_ON_NORMAL, _T("PNG"));
	m_gbtnToggle2.LoadAlhImage(IDB_PNG_TOGGLE_ON_ACTIVE, _T("PNG"));
	m_gbtnToggle2.EnableToggle(TRUE);
	m_gbtnToggle2.SetToggle(TRUE);

	m_gbtnMenuB1.LoadStdImage(IDB_PNG_MENU_B1_NORMAL, _T("PNG"));
	m_gbtnMenuB1.LoadSthImage(IDB_PNG_MENU_B1_ACTIVE, _T("PNG"));

	return true;
}


LRESULT CAntiRansomwareUserDlg::OnInitializationCompleted(WPARAM wParam, LPARAM lParam) // WM_INITIALIZATION_COMPLETED
{
	OnBnClickedButtonViewreport();
	return S_OK;
}

LRESULT CAntiRansomwareUserDlg::OnPopupInfoWindow(WPARAM wParam, LPARAM lParam) // WM_POPUP_INFO_WINDOW
{
	ArProcessBehavior* itemArProcessBehavior;

	if (wParam == 0) { // ������� Ž��
		itemArProcessBehavior = m_mapProcessBehavior[lParam];

		m_sPopupMessage.typePopup = 1;
		m_sPopupMessage.strTitle = "�������� Ž��!";
		m_sPopupMessage.strMessage1 = "�������� �ǽ� ������ Ž���Ǿ����ϴ�. (Behavior)";
		m_sPopupMessage.strMessage2 = "�ش� ���� ������ �����Ͻðڽ��ϱ� ?";
		m_sPopupMessage.pid = lParam;
		m_sPopupMessage.strProcName = itemArProcessBehavior->GetProcessName(PB_PROC_NAME);
		m_sPopupMessage.strProcPath = itemArProcessBehavior->GetProcessName(PB_PROC_PATH);
	}
	else if (wParam == 1) { // �ñ״�ó Ž��
		m_sPopupMessage.typePopup = 2;
		m_sPopupMessage.strTitle = "�������� Ž��!";
		m_sPopupMessage.strMessage1 = "������� Ž���Ǿ����ϴ�. (Signature)";
		m_sPopupMessage.strMessage2 = "������� Ž���Ǿ� �ش� ���� ������ �����Ͽ����ϴ�.";
		m_sPopupMessage.pid = -1;
		m_sPopupMessage.strProcName = PathFindFileName(m_strDetectedPath.GetBuffer());
		m_sPopupMessage.strProcPath = m_strDetectedPath;
	}

	g_pParent->DoPopupInfoWindow(); // �˾�â
	return S_OK;
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CAntiRansomwareUserDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


BOOL CAntiRansomwareUserDlg::OnEraseBkgnd(CDC* pDC)
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
	//pDC->StretchBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, bmap.bmWidth, bmap.bmHeight, SRCCOPY);
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);

	//return CDialogEx::OnEraseBkgnd(pDC);
	return true;
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CAntiRansomwareUserDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAntiRansomwareUserDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	// ����ǥ���� Ŭ�� �� â �̵�
	if(point.y < 37)
		SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, 0);

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CAntiRansomwareUserDlg::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnMoving(fwSide, pRect);

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	if (m_pAntiRansomwareReportDlg.GetSafeHwnd() != NULL) {
		m_pAntiRansomwareReportDlg.SetWindowPos(NULL, pRect->right, pRect->top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}


void CAntiRansomwareUserDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	m_isRunning = 0;

	// CRITICAL SECTION - Delete
	DeleteCriticalSection(&m_csFileExt);
	DeleteCriticalSection(&m_csScanLog);
	DeleteCriticalSection(&m_csFileQueue);
}


void CAntiRansomwareUserDlg::OnBnClickedButtonViewreport()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_pAntiRansomwareReportDlg.GetSafeHwnd() == NULL) {
		m_pAntiRansomwareReportDlg.Create(IDD_ANTIRANSOMWAREREPORTDLG);
		//m_pAntiRansomwareReportDlg.CenterWindow(CWnd::FromHandle(this->m_hWnd));
	}
	m_pAntiRansomwareReportDlg.ShowWindow(SW_SHOW);
}


void CALLBACK OnTimerFunc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	DWORD pid;
	int nResult;
	CString strTemp;

	if (wTimerID == g_idMNTimer1) {
		map<unsigned int, ArProcessBehavior*>::iterator itor;
		ArProcessBehavior* itemArProcessBehavior;

		// ���μ��� ��� ��ȸ
		for (itor = g_pParent->m_mapProcessBehavior.begin(); itor != g_pParent->m_mapProcessBehavior.end(); ++itor) {
			nResult = itor->second->CheckProcessInfo();
			if(nResult == 1){
				strTemp.Format("�������� Ž��! - pid: %d", itor->first);
				g_pParent->AddLogList(strTemp);
				pid = FindRansomwareParantPID(itor->second->GetProcessInfo(PB_PROC_PID));
				DoKillProcessTree(pid); // ���μ��� Ʈ�� ����
				g_pParent->DoKillRecoveryRansomware(pid); // ���� ����

				PostMessageA(g_pParent->m_hWnd, WM_POPUP_INFO_WINDOW, 0, pid); // �˾�â
				RefreshDesktopDirectory();
			}
		}
	}
}

void CAntiRansomwareUserDlg::AddLogList(CString msg, bool wTime)
{
	int listNum;
	CString strTime;

	::ZeroMemory(reinterpret_cast<void*>(&g_time), sizeof(g_time)); // time �ʱ�ȭ.
	::GetLocalTime(&g_time);    // ����ð��� ����.

	SCAN_LOG tmpScanLog;

	strTime.Format("%02d:%02d:%02d", g_time.wHour, g_time.wMinute, g_time.wSecond);
	tmpScanLog.timeStamp.SetString(strTime);
	tmpScanLog.content.SetString(msg);

	EnterCriticalSection(&m_csScanLog);
	m_listScanLog.push_back(tmpScanLog);
	LeaveCriticalSection(&m_csScanLog);
}


bool CAntiRansomwareUserDlg::InitMyScanner()
{
	m_isRunning = 1;
	AddLogList("InitMyScanner()", true);

	// �˻� ��� ���� Ȯ���� ���
	SetFileExtList();

	// �̴����� ��� ������ ����
	pThreadCommunication = AfxBeginThread(CommunicationMyScanner, this, THREAD_PRIORITY_NORMAL, 0, 0);
	if (pThreadCommunication == NULL) {
		m_isRunning = 0;
		AddLogList("[Error] Fail to create 'Communication' thread.", true);
		return false;
	}

	// ���� ��ȣȭ ���� �̺�Ʈ
	m_hEventCheckRansomware = CreateEvent(NULL, TRUE, FALSE, NULL);

	// ���� ��ȣȭ ���� ������ ����
	pThreadCheckRansomware = AfxBeginThread(CheckRansomwareWorker, this, THREAD_PRIORITY_NORMAL, 0, 0);
	if (pThreadCheckRansomware == NULL) {
		m_isRunning = 0;
		AddLogList("[Error] Fail to create 'CheckRansomware' thread.", true);
		return 0;
	}

	return false;
}


bool CAntiRansomwareUserDlg::AddCheckFileExtension(CString strExt)
{
	bool result;
	list<CString>::iterator itor;

	EnterCriticalSection(&m_csFileExt);

	// �ߺ� �˻�
	itor = m_listFileExt.begin();
	while (itor != m_listFileExt.end()) {
		if (strExt.Compare(*itor) == 0) {
			LeaveCriticalSection(&m_csFileExt);
			return false;
		}
		itor++;
	}

	// Ȯ���� �߰�
	m_listFileExt.push_back(strExt);

	LeaveCriticalSection(&m_csFileExt);

	return true;
}


bool CAntiRansomwareUserDlg::DoCheckFileExtension(CString strPath)
{
	bool result;
	CString strExt;
	list<CString>::iterator itor;

	// Ȯ���� �и�
	strExt = PathFindExtension(strPath);
	strExt = strExt.Mid(1);

	EnterCriticalSection(&m_csFileExt);

	// �ߺ� �˻�
	itor = m_listFileExt.begin();
	while (itor != m_listFileExt.end()) {
		if (strExt.Compare(*itor) == 0) {
			LeaveCriticalSection(&m_csFileExt);
			return true;
		}
		itor++;
	}

	LeaveCriticalSection(&m_csFileExt);

	return false;
}


void CAntiRansomwareUserDlg::SetFileExtList()
{
	char *token;
	char *szBackupExt;
	unsigned int nSize = strlen(g_szBackupExt);

	szBackupExt = new char[nSize + 1];
	ZeroMemory(szBackupExt, nSize + 1);
	memcpy(szBackupExt, g_szBackupExt, nSize);

	token = strtok((char*)szBackupExt, ",");
	while (token) {
		AddCheckFileExtension((CString)token); // Add
		token = strtok(NULL, ",");
	}

	delete szBackupExt;
}


BOOL
ScanBuffer(
	__in_bcount(BufferSize) PUCHAR Buffer,
	__in ULONG BufferSize
)
/*++

Routine Description

Scans the supplied buffer for an instance of FoulString.

Note: Pattern matching algorithm used here is just for illustration purposes,
there are many better algorithms available for real world filters

Arguments

Buffer      -   Pointer to buffer
BufferSize  -   Size of passed in buffer

Return Value

TRUE        -    Found an occurrence of the appropriate FoulString
FALSE       -    Buffer is ok

--*/
{
	PUCHAR p;
	ULONG searchStringLength = sizeof(FoulString) - sizeof(UCHAR);

	for (p = Buffer;
		p <= (Buffer + BufferSize - searchStringLength);
		p++) {

		if (RtlEqualMemory(p, FoulString, searchStringLength)) {

			printf("Found a string\n");

			//
			//  Once we find our search string, we're not interested in seeing
			//  whether it appears again.
			//

			return TRUE;
		}
	}

	return FALSE;
}


DWORD
ScannerWorker(
	__in PSCANNER_THREAD_CONTEXT Context
)
/*++

Routine Description

This is a worker thread that


Arguments

Context  - This thread context has a pointer to the port handle we use to send/receive messages,
and a completion port handle that was already associated with the comm. port by the caller

Return Value

HRESULT indicating the status of thread exit.

--*/
{
	PSCANNER_NOTIFICATION notification;
	SCANNER_REPLY_MESSAGE replyMessage;
	PSCANNER_MESSAGE message;
	LPOVERLAPPED pOvlp;
	BOOL result;
	DWORD outSize;
	HRESULT hr;
	ULONG_PTR key;

	int nResult = 0;
	CString strMsg;

#pragma warning(push)
#pragma warning(disable:4127) // conditional expression is constant

	while (TRUE) {

#pragma warning(pop)

		//
		//  Poll for messages from the filter component to scan.
		//

		result = GetQueuedCompletionStatus(Context->Completion, &outSize, &key, &pOvlp, INFINITE);

		//
		//  Obtain the message: note that the message we sent down via FltGetMessage() may NOT be
		//  the one dequeued off the completion queue: this is solely because there are multiple
		//  threads per single port handle. Any of the FilterGetMessage() issued messages can be
		//  completed in random order - and we will just dequeue a random one.
		//

		message = CONTAINING_RECORD(pOvlp, SCANNER_MESSAGE, Ovlp);

		if (!result) {

			//
			//  An error occured.
			//

			hr = HRESULT_FROM_WIN32(GetLastError());
			break;
		}

		//strMsg.Format("Received message, size %d", pOvlp->InternalHigh);
		//g_pParent->AddLogList(strMsg, 1, true);
		//printf("Received message, size %d\n", pOvlp->InternalHigh);

		notification = &message->Notification;

		if (notification->Reserved == 0)
		{
			assert(notification->BytesToScan <= SCANNER_READ_BUFFER_SIZE);
			__analysis_assume(notification->BytesToScan <= SCANNER_READ_BUFFER_SIZE);

			result = ScanBuffer(notification->Contents, notification->BytesToScan);

			replyMessage.ReplyHeader.Status = 0;
			replyMessage.ReplyHeader.MessageId = message->MessageHeader.MessageId;

			//
			//  Need to invert the boolean -- result is true if found
			//  foul language, in which case SafeToOpen should be set to false.
			//

			replyMessage.Reply.SafeToOpen = !result;

			//strMsg.Format("Replying message, SafeToOpen: %d", replyMessage.Reply.SafeToOpen);
			//g_pParent->AddLogList(strMsg, 1, true);
			//printf("Replying message, SafeToOpen: %d\n", replyMessage.Reply.SafeToOpen);

		}
		else
		{
			BOOL bRet;
			int i;
			char *pFilePath;
			wchar_t szDrivePath[1024] = { 0 };

			replyMessage.ReplyHeader.Status = 0;
			replyMessage.ReplyHeader.MessageId = message->MessageHeader.MessageId;
			replyMessage.Reply.SafeToOpen = TRUE;

			USES_CONVERSION;

			switch (notification->Reserved)
			{
				case fltType_PreCreate:
					strMsg.Format("===== [%d] fltType_PreCreate =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					nResult = g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
				case fltType_PostCreate:
					strMsg.Format("===== [%d] fltType_PostCreate =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					nResult = g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
				case fltType_PreClose:
					strMsg.Format("===== [%d] fltType_PreClose =====", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PostClose:
					strMsg.Format("===== [%d] fltType_PostClose =====", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PreCleanup:
					strMsg.Format("===== [%d] fltType_PreCleanup =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					nResult = g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
				case fltType_PostCleanup:
					strMsg.Format("===== [%d] fltType_PostCleanup =====", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PreWrite:
					strMsg.Format("===== [%d] fltType_PreWrite =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					//nResult = g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
				case fltType_PostWrite:
					strMsg.Format("===== [%d] fltType_PostWrite =====", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					//nResult = g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
				case fltType_PreSetInformation:
					strMsg.Format("===== [%d] fltType_PreSetInformation =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					nResult = g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
				case fltType_PostSetInformation:
					strMsg.Format("===== [%d] fltType_PostSetInformation =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					nResult = g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
			}
		}
		if (nResult == 100) { // Backup
			CString strBackupPath;
			CString strNewDir;
			int nPos;
			wchar_t szFilePath[MAX_FILE_PATH] = { 0 };
			wchar_t szBackupPath[MAX_FILE_PATH] = { 0 };
			PATH_INFO_EX pathInfoEx;

			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH, &pathInfoEx) == FALSE)
				break;

			// ��� ��� ���ϱ�
			strBackupPath = g_pParent->GetBackupFilePath((CString)szFilePath);

			// ��� ���� ���� Ȯ��
			if (PathFileExists(strBackupPath) == FALSE) {
				g_pParent->m_strBackingUpPath.SetString(strBackupPath); // ��� ���� ���� ���

				// ��� ���� ���丮 ����
				strNewDir.SetString(strBackupPath);
				PathRemoveFileSpec((LPSTR)(LPCTSTR)strNewDir);
				CreateFolder(strNewDir); // ��� ���丮 ����

				nPos = strBackupPath.Find('\\');
				if (nPos >= 0) {
					strBackupPath.Format("%S%s", pathInfoEx.szDeviceName, strBackupPath.Mid(nPos));
					//g_pParent->AddLogList(strBackupPath);
				}
				memcpy(replyMessage.Reply.Contents, (const char *)strBackupPath, MAX_FILE_PATH);
			}
			else {
				// ��� ���� ����
				nResult = 0;
			}
		}
		replyMessage.Reply.cmdType = nResult; // Command

		hr = FilterReplyMessage(Context->Port,
			(PFILTER_REPLY_HEADER)&replyMessage,
			sizeof(replyMessage));

		if (SUCCEEDED(hr)) {

			//strMsg.Format("Replied message");
			//g_pParent->AddLogList(strMsg, 1, true);
			printf("Replied message\n");

		}
		else {

			strMsg.Format("Scanner: Error replying message. Error = 0x%X", hr);
			g_pParent->AddLogList(strMsg, true);
			printf("Scanner: Error replying message. Error = 0x%X\n", hr);
			break;
		}

		memset(&message->Ovlp, 0, sizeof(OVERLAPPED));

		hr = FilterGetMessage(Context->Port,
			&message->MessageHeader,
			FIELD_OFFSET(SCANNER_MESSAGE, Ovlp),
			&message->Ovlp);

		if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {

			break;
		}

		if (!g_pParent->m_isRunning)
			break;
	}

	if (!SUCCEEDED(hr)) {

		if (hr == HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)) {

			//
			//  Scanner port disconncted.
			//

			strMsg.Format("Scanner: Port is disconnected, probably due to scanner filter unloading.");
			g_pParent->AddLogList(strMsg, true);
			printf("Scanner: Port is disconnected, probably due to scanner filter unloading.\n");

		}
		else {

			strMsg.Format("Scanner: Unknown error occured. Error = 0x%X", hr);
			g_pParent->AddLogList(strMsg, true);
			printf("Scanner: Unknown error occured. Error = 0x%X\n", hr);
		}
	}

	free(message);

	return hr;
}


UINT CAntiRansomwareUserDlg::CommunicationMyScanner(LPVOID lpParam)
{
	CAntiRansomwareUserDlg* pDlg = (CAntiRansomwareUserDlg*)lpParam;

	DWORD requestCount = SCANNER_DEFAULT_REQUEST_COUNT;
	DWORD threadCount = SCANNER_DEFAULT_THREAD_COUNT;
	HANDLE threads[SCANNER_MAX_THREAD_COUNT];
	SCANNER_THREAD_CONTEXT context;
	HANDLE port, completion;
	PSCANNER_MESSAGE msg;
	DWORD threadId;
	HRESULT hr;
	DWORD i, j;

	CString strMsg;

	pDlg->AddLogList("CommunicationMyScanner()", true);

	pDlg->AddLogList("Scanner: Connecting to the filter ...", true);
	printf("Scanner: Connecting to the filter ...\n");

	hr = FilterConnectCommunicationPort(ScannerPortName,
		0,
		NULL,
		0,
		NULL,
		&port);

	if (IS_ERROR(hr)) {
		strMsg.Format("ERROR: Connecting to filter port: 0x%08x", hr);
		pDlg->AddLogList(strMsg, true);
		printf("ERROR: Connecting to filter port: 0x%08x\n", hr);
		return 2;
	}

	//
	//  Create a completion port to associate with this handle.
	//

	completion = CreateIoCompletionPort(port,
		NULL,
		0,
		threadCount);

	if (completion == NULL) {

		printf("ERROR: Creating completion port: %d\n", GetLastError());
		CloseHandle(port);
		return 3;
	}

	strMsg.Format("Scanner: Port = 0x%p Completion = 0x%p", port, completion);
	pDlg->AddLogList(strMsg, true);
	printf("Scanner: Port = 0x%p Completion = 0x%p\n", port, completion);


	USER_NOTIFICATION usr_noti;
	DWORD dwProcId;
	char szRecvTest[100];
	DWORD rtnSz;

	GetWindowThreadProcessId(hWnd, &dwProcId);
	usr_noti.user_pid = dwProcId;

	hr = FilterSendMessage(port,
		&usr_noti,
		sizeof(USER_NOTIFICATION),
		szRecvTest,
		sizeof(szRecvTest),
		&rtnSz);

	context.Port = port;
	context.Completion = completion;

	//
	//  Create specified number of threads.
	//

	for (i = 0; i < threadCount; i++) {

		threads[i] = CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)ScannerWorker,
			&context,
			0,
			&threadId);

		if (threads[i] == NULL) {

			//
			//  Couldn't create thread.
			//

			hr = GetLastError();
			printf("ERROR: Couldn't create thread: %d\n", hr);
			goto main_cleanup;
		}

		for (j = 0; j < requestCount; j++) {

			//
			//  Allocate the message.
			//

#pragma prefast(suppress:__WARNING_MEMORY_LEAK, "msg will not be leaked because it is freed in ScannerWorker")
			msg = (PSCANNER_MESSAGE)malloc(sizeof(SCANNER_MESSAGE));

			if (msg == NULL) {

				hr = ERROR_NOT_ENOUGH_MEMORY;
				goto main_cleanup;
			}

			memset(&msg->Ovlp, 0, sizeof(OVERLAPPED));

			//
			//  Request messages from the filter driver.
			//

			hr = FilterGetMessage(port,
				&msg->MessageHeader,
				FIELD_OFFSET(SCANNER_MESSAGE, Ovlp),
				&msg->Ovlp);

			if (hr != HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {

				free(msg);
				goto main_cleanup;
			}
		}
	}

	hr = S_OK;

	WaitForMultipleObjectsEx(i, threads, TRUE, INFINITE, FALSE);

main_cleanup:

	printf("Scanner:  All done. Result = 0x%08x\n", hr);

	CloseHandle(port);
	CloseHandle(completion);

	pDlg->pThreadCommunication = NULL;
	return 0;
}


inline void WaitG(double dwMillisecond)
{
	MSG msg;
	LARGE_INTEGER   st, ed, freq;
	double			WaitFrequency;
	QueryPerformanceFrequency(&freq);
	WaitFrequency = ((double)dwMillisecond / 1000) * ((double)freq.QuadPart);

	if (freq.QuadPart == 0)
	{
		//::SetDlgItemText(hWnd,IDC_EDIT_Status,"Warning! - ���ػ� Ÿ�̸� ���� ����.");
		//AddListLog(1, "���� ����.");
		return;
	}

	QueryPerformanceCounter(&st);
	QueryPerformanceCounter(&ed);
	while ((double)(ed.QuadPart - st.QuadPart) < WaitFrequency)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(1);

		QueryPerformanceCounter(&ed);
	}
}


BOOL DoKillProcess(DWORD pid)
{
	BOOL bResult;
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	bResult = TerminateProcess(hProcess, 0);

	return bResult;
}

BOOL DoKillProcessTree(DWORD pid)
{
	CString strTemp;
	HANDLE handle = NULL;
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);
	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	strTemp.Format("��Kill: %d", pid);
	g_pParent->AddLogList(strTemp);
	if (pid == 0){
		g_pParent->AddLogList(" �� pass");
		return TRUE;
	}

	DoKillProcess(pid);

	if (Process32First(handle, &pe))
	{
		do
		{
			if (pe.th32ParentProcessID == pid) {
				if(pe.th32ProcessID != pid)
					DoKillProcessTree(pe.th32ProcessID);
			}
		} while (Process32Next(handle, &pe));
	}

	return TRUE;
}


bool GetProcessName(DWORD pid, DWORD *ppid, char *szName)
{
	HANDLE handle = NULL;
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);
	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(handle, &pe))
	{
		do
		{
			if (pe.th32ProcessID == pid) {
				strncpy(szName, pe.szExeFile, MAX_PATH);
				*ppid = pe.th32ParentProcessID;
				return true;
			}
		} while (Process32Next(handle, &pe));
	}

	return false;
}


DWORD FindRansomwareParantPID(DWORD pid)
{
	bool result;
	bool isFound = false;
	DWORD prev_pid = pid;
	DWORD ppid = pid;
	char szProcessName[MAX_PATH];
	CString strTemp;

	while (ppid != 0) {
		result = GetProcessName(pid, &ppid, szProcessName);
		if (strcmp(szProcessName, "explorer.exe") == 0) {
			isFound = true;
			break;
		}
		else {
			prev_pid = pid;
			pid = ppid;
		}
		if (result == false){
			isFound = true;
			break;
		}
	}
	if (isFound == false) {
		return false;
	}

	return prev_pid;
}


int CAntiRansomwareUserDlg::GetPermissionDirectory(CString strPath, DWORD pid)
{
	int nPos;
	int nResult = 0;
	CString strSafePath;
	DWORD dwProcId;
	CString strTemp;

	// ��� ���丮 ����
	nPos = strPath.Find('\\');
	if (nPos == -1)
		return 0;

	strSafePath.Format("%s%s", strPath.Left(nPos), g_szBackupPath);

	// My pid 
	GetWindowThreadProcessId(hWnd, &dwProcId);

	// ��� ���� ���� ����
	if (!g_pParent->m_strBackingUpPath.IsEmpty()) {
		if (strPath.Compare(g_pParent->m_strBackingUpPath) == 0)
			return 0;
	}

	// Safe Backup Directory �˻�
	if (strPath.Find(strSafePath, 0) >= 0) {
		nResult = 0;
		if (pid != dwProcId) {
			//strTemp.Format("!! ���� ����(Safe) !! : %s", strPath);
			//g_pParent->AddLogList(strTemp);
			nResult = 3; // ���� ����
		}
		else {
			nResult = 2; // ���� ����(me): Safe Backup Directory
		}
	}
	else {
		for(int i=0; i<5; i++){
			nResult = 0;
			EnterCriticalSection(&m_csFileQueue);
			// ��ȣ ���� ���� �˻�
			list<ITEM_CHECK_FILE>::iterator itor = m_listCheckFile.begin();
			while (itor != m_listCheckFile.end()) {
				if (strPath.Compare((CString)itor->strPath) == 0) {
					if (pid != dwProcId) {
						nResult = 3; // ���� ����
						break;
					}
					else {
						nResult = 1; // ���� ����(me): Scheduled File
						break;
					}
				}
				itor++;
			}
			LeaveCriticalSection(&m_csFileQueue);
			if (nResult == 3) {
				// ��� 
				//strTemp.Format("!! ���� ����(Scheduled) - Wait...(%d) !! : %s", i, strPath);
				//g_pParent->AddLogList(strTemp);
				Sleep(10); // Wait
			}
			else
				break;
		}
		if (nResult == 3) {
			//strTemp.Format("!! ���� ����(Scheduled) !! : %s", strPath);
			//g_pParent->AddLogList(strTemp);
		}
	}

	return nResult;
}

// ���μ��� ���� ����
int CAntiRansomwareUserDlg::RecordProcessBehavior(PSCANNER_NOTIFICATION notification)
{
	ArProcessBehavior* itemArProcessBehavior;
	int nReturn = 0;

	if (m_mapProcessBehavior.find(notification->ulPID) == m_mapProcessBehavior.end())
	{
		// �ű� ���μ��� ���
		ArProcessBehavior* newArProcessBehavior = new ArProcessBehavior(notification->ulPID);
		m_mapProcessBehavior[notification->ulPID] = newArProcessBehavior;

		itemArProcessBehavior = newArProcessBehavior;
	}
	else {
		itemArProcessBehavior = m_mapProcessBehavior[notification->ulPID];
	}

	// ���μ��� ���� ���
	if (!itemArProcessBehavior->m_isRansomware)
		nReturn = itemArProcessBehavior->RecordProcessBehavior(notification);
	else
		nReturn = 1;

	return nReturn;
}

CString CAntiRansomwareUserDlg::GetBackupFilePath(CString strPath)
{
	int nPos;
	CString strNewPath;

	// ��� ���丮 ����
	nPos = strPath.Find('\\');
	strNewPath.Format("%s%s%s", strPath.Left(nPos), g_szBackupPath, strPath.Mid(nPos));

	return strNewPath;
}


float entropy_calc(long byte_count[], int length)
{
	float entropy = 0.0;
	float count;
	int i;

	/* entropy calculation */
	for (i = 0; i < 256; i++) // 1byte? #define SIZE 256
	{
		if (byte_count[i] != 0)
		{
			count = (float)byte_count[i] / (float)length;
			entropy += -count * log2f(count);
		}
	}
	return entropy;
}


float GetFileEntropy(FILE *inFile, int offset)
{
	int             i;
	int             j;
	int             n;			// Bytes read by fread;
	int             length = 0;	// length of file
	float           count;
	float           entropy;
	long            byte_count[256];
	unsigned char   buffer[10240];

	memset(byte_count, 0, sizeof(long) * 256);

	fseek(inFile, offset, SEEK_SET);

	/* Read the whole file in parts of 1024 */
	while ((n = fread(buffer, 1, 10240, inFile)) != 0)
	{
		/* Add the buffer to the byte_count */
		for (i = 0; i < n; i++)
		{
			byte_count[(int)buffer[i]]++;
			length++;
		}
	}
	fclose(inFile);
	
	return entropy_calc(byte_count, length);;
}


int CAntiRansomwareUserDlg::DoCheckRansomware(CString strPath)
{
	int nPos;
	int nResult;
	bool isDeleteBackupFile = true;
	FILE* fpTarget;
	FILE* fpBackup;
	CString strBackupPath;
	ITEM_BACKUP_FILE tmpIBF;
	list<ITEM_BACKUP_FILE>::reverse_iterator ritorIBF; // backup
	unsigned char buf_sigTarget[4];
	unsigned char buf_sigBackup[4];
	unsigned int size_target;
	unsigned int size_backup;
	float entropy_target;
	float entropy_backup;
	bool isChangeEntropy = false;
	CString strTemp;
	HANDLE hFile;

	// ��� ��� ���ϱ�
	strBackupPath = GetBackupFilePath(strPath);

	// ��� ���� ���� Ȯ��
	if (PathFileExists(strBackupPath) == FALSE) {
		AddLogList("��� ���� ����: " + strBackupPath);
		return 0;
	}

	// �� ���� ����
	fpTarget = fopen((LPSTR)(LPCTSTR)strPath, "rb");
	if (fpTarget == NULL){
		//AddLogList("���� ���� ����(fpTarget) : " + strPath);
		return -1;
	}

	fpBackup = fopen((LPSTR)(LPCTSTR)strBackupPath, "rb");
	if (fpBackup == NULL){
		//AddLogList("���� ���� ����(fpBackup) : " + strBackupPath);
		fclose(fpTarget);
		return -1;
	}

	// STEP1: ���� �ñ״�ó ��
	size_target = fread(buf_sigTarget, 1, 4, fpTarget);
	size_backup = fread(buf_sigBackup, 1, 4, fpBackup);
	if (size_target == size_backup) {
		nResult = memcmp(buf_sigTarget, buf_sigBackup, 4);
		if (nResult != 0) {
			AddLogList("Warning: ���� �ñ״�ó ����");
			fclose(fpTarget);
			fclose(fpBackup);
			return 1;
		}
	}
	else {
		AddLogList("Warning: ���� �ñ״�ó ����");
		fclose(fpTarget);
		fclose(fpBackup);
		return 1;
	}

	// STEP2: ���� ���� ��Ʈ���� �м�(text)
	entropy_target = GetFileEntropy(fpTarget, 0);
	entropy_backup = GetFileEntropy(fpBackup, 0);

	if (entropy_backup < 6.5) {
		if (entropy_target > 7.0)
			isChangeEntropy = true;
	}
	else if (entropy_backup < 7.5) {
		if (entropy_target > 7.9)
			isChangeEntropy = true;
	}
	
	if(isChangeEntropy){
		strTemp.Format("Warning: ��Ʈ���� ��ȭ (%02.5f -> %02.5f, %02.5f)", entropy_backup, entropy_target, abs(entropy_target - entropy_backup));
		AddLogList(strTemp);
		return 1;
	}

	// STEP3: [ssdeep] Fuzzy Hash ��(binary)
	int nFuzzy;
	char * resultFuzzy, *resultFuzzy2;
	resultFuzzy = (char *)malloc(FUZZY_MAX_RESULT);
	resultFuzzy2 = (char *)malloc(FUZZY_MAX_RESULT);
	nResult = fuzzy_hash_filename((LPSTR)(LPCTSTR)strPath, resultFuzzy);
	nResult = fuzzy_hash_filename((LPSTR)(LPCTSTR)strBackupPath, resultFuzzy2);

	nFuzzy = fuzzy_compare(resultFuzzy, resultFuzzy2);
	if (nFuzzy >= 0) {
		strTemp.Format("Fuzzy Hash : score = %d\n", nFuzzy);
		AddLogList(strTemp);
		if (nFuzzy <= 20) {
			AddLogList("Warning: Fuzzy Hash ����ġ");
			return 1;
		}
	}

	free(resultFuzzy);
	free(resultFuzzy2);

	fclose(fpTarget);
	fclose(fpBackup);

	// ��� ���� ����(�ٸ� ��� �̺�Ʈ�� ���ٸ�)
	/*
	ritorIBF = m_listBackupFile.rbegin();
	while (ritorIBF != m_listBackupFile.rend())
	{
		if (strBackupPath.Compare(ritorIBF->strPath) == 0) {
			AddLogList("��� ���� ���� ����");
			isDeleteBackupFile = false;
			break;
		}
		ritorIBF++;
	}
	*/
	if (isDeleteBackupFile == true) {
		AddLogList("��� ���� ����: " + strBackupPath);
		DeleteFile(strBackupPath); // ��� ���� ����
	}

	return 0;
}

bool CAntiRansomwareUserDlg::DoPopupInfoWindow()
{
	// �˸�
	if (m_pAntiRansomwarePopupDlg.GetSafeHwnd() == NULL) {
		m_pAntiRansomwarePopupDlg.Create(IDD_ANTIRANSOMWAREPOPUPDLG);
		m_pAntiRansomwarePopupDlg.InitPopupWindow(m_sPopupMessage);
	}
	m_pAntiRansomwarePopupDlg.ShowWindow(SW_SHOW);

	return true;
}


// ���� ��ȣȭ ���� ������
static UINT CheckRansomwareWorker(LPVOID lpParam)
{
	CAntiRansomwareUserDlg* pDlg = (CAntiRansomwareUserDlg*)lpParam;
	list<ITEM_CHECK_FILE> *listFile = (list<ITEM_CHECK_FILE>*)&pDlg->m_listCheckFile;
	list<ITEM_CHECK_FILE>::iterator itor;

	bool result;
	bool isNewFile;
	int nResult;
	CString strTemp;
	ArProcessBehavior* itemArProcessBehavior;

	while (pDlg->m_isRunning) {
		result = false;
		isNewFile = false;
		// Wait Event
		WaitForSingleObject(pDlg->m_hEventCheckRansomware, INFINITE);
		pDlg->m_nCountMonitor++;
		EnterCriticalSection(&pDlg->m_csFileQueue);

		if (!listFile->empty()) {
			for (itor = listFile->begin(); itor != listFile->end();) {
				if(&itor != NULL){
					// Ž�� �õ�
					if (itor->nCheckType == 0)								// ������� Ž��
						nResult = pDlg->DoCheckRansomware(itor->strPath);	// - �������� ���� Ȯ��
					else if (itor->nCheckType == 1)							// �ñ״�ó Ž��
						nResult = pDlg->DoMatchRansomwareDB(itor->strPath);	// - Yara rule

					// ���
					if (nResult == 1) { // ���� ������
						if (itor->nCheckType == 0) { // �������
							itemArProcessBehavior = pDlg->m_mapProcessBehavior[itor->pid];
							itemArProcessBehavior->AddEventWriteSpFile(itor->strPath, true); // �ǽ�
							strTemp.Format("[%d] PB_COUNT_WRITE_SP: %d", itor->pid, itemArProcessBehavior->GetCountBehavior(PB_COUNT_WRITE_SP));
							pDlg->AddLogList(strTemp);
						}
						else if (itor->nCheckType == 1) { // �ñ״�ó
							// ���� ����
							// �˸�
						}
						itor = pDlg->m_listCheckFile.erase(itor); // �׸� ����
					}
					else if (nResult == 0) { // ����
						itor = pDlg->m_listCheckFile.erase(itor); // �׸� ����
					}
					else if (nResult == -1) { // Open ����
						if (itor->nCheckCount < 10)
							isNewFile = true;
						itor->nCheckCount++;
						if (PathFileExists(itor->strPath) == FALSE) {
							strTemp.Format("[L] ���� ����: %s", itor->strPath);
							pDlg->AddLogList(strTemp);
							itor = pDlg->m_listCheckFile.erase(itor); // �׸� ����
						}
						else {
							if (itor->nCheckCount > 100) {
								pDlg->AddLogList("[L] ���� ���� ���� (check count)");
								itor = pDlg->m_listCheckFile.erase(itor); // �׸� ����
							}
							else {
								itor++;
							}
						}
					}
				}
			}
			result = true;
		}
		else {
			// Reset Event
			ResetEvent(pDlg->m_hEventCheckRansomware);
		}

		LeaveCriticalSection(&pDlg->m_csFileQueue);

		if(result){
			if(isNewFile)
				Sleep(1);
			else
				Sleep(10);
		}
		else{
			Sleep(10);
		}
	}

	pThreadCheckRansomware = NULL;
	return 0;
}


bool CAntiRansomwareUserDlg::DoKillRecoveryRansomware(DWORD pid)
{
	map<unsigned int, ArProcessBehavior*>::iterator itor;
	ArProcessBehavior* itemArProcessBehavior;

	// ���� ������ �˻�
	for (itor = m_mapProcessBehavior.begin(); itor != m_mapProcessBehavior.end(); ++itor) {
		itemArProcessBehavior = itor->second;
		if (itemArProcessBehavior->GetProcessInfo(PB_PROC_PID) == pid){
			itemArProcessBehavior->RecoveryProcessBehavior(); // ���� ����
		}
		else if (itemArProcessBehavior->GetProcessInfo(PB_PROC_PPID) == pid){
			if(itemArProcessBehavior->GetProcessInfo(PB_PROC_PPID) != itemArProcessBehavior->GetProcessInfo(PB_PROC_PID))
				DoKillRecoveryRansomware(itemArProcessBehavior->GetProcessInfo(PB_PROC_PID)); // ��� ȣ��
		}
	}

	return true;
}


void CAntiRansomwareUserDlg::OnBnClickedButtonRecovery()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CString strTemp;
	ArProcessBehavior* itemArProcessBehavior;
	DWORD tmpPid;

	ctr_editTargetPid.GetWindowTextA(strTemp);
	tmpPid = (DWORD)atoi((LPSTR)(LPCTSTR)strTemp);
	if (m_mapProcessBehavior.find(tmpPid) != m_mapProcessBehavior.end()){
		itemArProcessBehavior = m_mapProcessBehavior[tmpPid];
		itemArProcessBehavior->RecoveryProcessBehavior(); // ���� ����
	}
}


bool CAntiRansomwareUserDlg::AddCheckRansomwareFile(DWORD pid, CString strPath)
{
	CString strExt;
	CString strTemp;
	ITEM_CHECK_FILE tmpICF;

	tmpICF.pid = pid;
	tmpICF.strPath = strPath;
	tmpICF.nCheckCount = 0;

	strExt = PathFindExtension(strPath);
	strExt = strExt.Mid(1);
	if (strExt.Compare("exe") != 0)
		tmpICF.nCheckType = 0; // ������� Ž��
	else
		tmpICF.nCheckType = 1; // �ñ״�ó Ž��

	EnterCriticalSection(&m_csFileQueue);

	// �ߺ��˻�
	list<ITEM_CHECK_FILE>::iterator itor = m_listCheckFile.begin();
	while (itor != m_listCheckFile.end()) {
		if (strPath.Compare((CString)itor->strPath) == 0) {
			LeaveCriticalSection(&m_csFileQueue);
			return true;
		}
		itor++;
	}

	// �˻� ��� ���� ť�� �߰�
	m_listCheckFile.push_back(tmpICF);
	SetEvent(m_hEventCheckRansomware);

	LeaveCriticalSection(&m_csFileQueue);

	strTemp.Format("�˻� �׸� �߰�: %s", strPath);
	AddLogList(strTemp);

	return true;
}

void CAntiRansomwareUserDlg::OnBnClickedButtonClose()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	PostQuitMessage(WM_QUIT);
}


void CAntiRansomwareUserDlg::OnBnClickedButtonMinimum()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_sPopupMessage.typePopup = 1;
	m_sPopupMessage.strTitle = "�������� Ž��!";
	m_sPopupMessage.strMessage1 = "�������� �ǽ� ������ Ž���Ǿ����ϴ�.";
	m_sPopupMessage.strMessage2 = "�ش� ���� ������ �����Ͻðڽ��ϱ� ?";
	m_sPopupMessage.pid = 1234;
	m_sPopupMessage.strProcName = "Name";
	m_sPopupMessage.strProcPath = "Path";

	g_pParent->DoPopupInfoWindow(); // �˾�â
}


void CAntiRansomwareUserDlg::OnBnClickedButtonToggle1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	SetDetectionEngine();
}


void CAntiRansomwareUserDlg::OnBnClickedButtonToggle2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	SetDetectionEngine();
}


bool CAntiRansomwareUserDlg::SetDetectionEngine()
{
	int nEngines = 0;
	if (m_gbtnToggle1.IsToggled() == TRUE) {
		nEngines += 1;
	}
	if (m_gbtnToggle2.IsToggled() == TRUE) {
		nEngines += 1;
	}

	switch (nEngines)
	{
		case 0:
			lamp_image.LoadBitmap(IDB_BITMAP_MENU1_DANGER);
			break;
		case 1:
			lamp_image.LoadBitmap(IDB_BITMAP_MENU1_WARNING);
			break;
		case 2:
			lamp_image.LoadBitmap(IDB_BITMAP_MENU1_SAFETY);
			break;
	}

	HBITMAP h_old_bitmap = ctr_picMenu1.SetBitmap(lamp_image);
	if (h_old_bitmap != NULL) ::DeleteObject(h_old_bitmap);
	lamp_image.Detach();
	
	m_gbtnToggle1.Invalidate(TRUE);
	m_gbtnToggle1.SetBkGnd();
	m_gbtnToggle2.Invalidate(TRUE);
	m_gbtnToggle2.SetBkGnd();
	
	return true;
}

void CAntiRansomwareUserDlg::OnBnClickedButtonMenu2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_strDetectedPath = "c:\\test.exe";
	PostMessageA(WM_POPUP_INFO_WINDOW, 1, NULL); // �˾�â
}


void CAntiRansomwareUserDlg::OnBnClickedButtonMenu3()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	bool result;
	result = DoUpdateSignatureDB();
	
}


void CAntiRansomwareUserDlg::OnBnClickedButtonMenu4()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	int nResult;
	DoMatchRansomwareDB("E:\\��Clavis��\\��Programming��\\��Project��\\Anti_Ransomware\\TestRansomware\\Release\\TestRansomware.exe");
}


void CAntiRansomwareUserDlg::OnBnClickedButtonMenu5()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_pAntiRansomwareSettingsDlg.GetSafeHwnd() == NULL) {
		m_pAntiRansomwareSettingsDlg.Create(IDD_ANTIRANSOMWARESETTINGSDLG);
		m_pAntiRansomwareSettingsDlg.CenterWindow(CWnd::FromHandle(this->m_hWnd));
	}
	m_pAntiRansomwareSettingsDlg.ShowWindow(SW_SHOW);
}

bool CAntiRansomwareUserDlg::DoSendRansomwareFile(CString strPath)
{
	bool result;
	CString strHeader;
	CString strExt;

	result = m_fileTransfer.ConnectToServer(SERVER_IP, SERVER_PORT);
	if (result == false) {
		AfxMessageBox("Failed!");
	}

	// Ȯ���� �и�
	strExt = PathFindExtension(strPath);
	strExt = strExt.Mid(1);

	strHeader.Format("CheckVM Detected %s", strExt);
	m_fileTransfer.SendFile(strHeader.GetBuffer(), strPath.GetBuffer());
	

	m_fileTransfer.Disconnect();
	return true;
}

bool CAntiRansomwareUserDlg::DoUpdateSignatureDB()
{
	bool result;
	CString strHeader;
	CString strExt;
	HZIP hz;
	ZIPENTRY ze;
	char szFileName[1024] = { 0 };
	char szFilePath[1024] = { 0 };

	sprintf(szFilePath, "%s\\db\\", appPath);
	sprintf(szFileName, "%supdb_ar.zip", szFilePath);

	result = m_fileTransfer.ConnectToServer(SERVER_IP, SERVER_PORT);
	if (result == false) {
		AfxMessageBox("Failed!");
	}

	strHeader.Format("UpdateDB");
	m_fileTransfer.RecvFile(strHeader.GetBuffer(), szFileName);

	m_fileTransfer.Disconnect();
	
	hz = OpenZip(_T(szFileName), 0);
	SetUnzipBaseDir(hz, _T(szFilePath));
	GetZipItem(hz, -1, &ze);
	int numitems = ze.index;
	for (int zi = 0; zi<numitems; zi++)
	{
		GetZipItem(hz, zi, &ze);
		UnzipItem(hz, zi, ze.name);
	}
	CloseZip(hz);
	DeleteFile(szFileName); // ���� ���� ����

	return true;
}

int CAntiRansomwareUserDlg::DoMatchRansomwareDB(CString strPath)
{
	FILE* file;
	HANDLE hFile;
	char output[1024];
	char command[1024];
	CString strTemp;

	hFile = CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1; // Open ����
	else
		CloseHandle(hFile);

	// Temp - Yara Test
	sprintf(command, "%s\\yara32.exe %s\\db\\AR.yar %s", appPath, appPath, strPath);

	file = _popen(command, "r");
	fread(output, 1, sizeof(output), file);
	fclose(file);
	if (strstr(output, strPath)) {
		strTemp.Format("�������� Ž��! - Path: %s", strPath);
		g_pParent->AddLogList(strTemp);
		m_strDetectedPath = strPath;
		PostMessageA(WM_POPUP_INFO_WINDOW, 1, NULL); // �˾�â
		return 1;
	}
	return 0;
}