
// AntiRansomwareUserDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "AntiRansomwareUser.h"
#include "AntiRansomwareUserDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SYSTEMTIME g_time; // �ð� ����ü.

HWND hWnd;
CAntiRansomwareUserDlg *g_pParent;

UCHAR FoulString[] = "foul";

// CAntiRansomwareUserDlg ��ȭ ����



CAntiRansomwareUserDlg::CAntiRansomwareUserDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ANTIRANSOMWAREUSER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAntiRansomwareUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAntiRansomwareUserDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(WM_INITIALIZATION_COMPLETED, OnInitializationCompleted) // WM_INITIALIZATION_COMPLETED
	ON_BN_CLICKED(IDC_BUTTON_ViewReport, &CAntiRansomwareUserDlg::OnBnClickedButtonViewreport)
	ON_WM_MOVING()
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

	m_listScanLog.clear();

	// CRITICAL SECTION - Initial
	InitializeCriticalSection(&m_csScanLog);

	InitMyScanner();

	PostMessageA(WM_INITIALIZATION_COMPLETED, NULL, NULL); // WM_INITIALIZATION_COMPLETED

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}


LRESULT CAntiRansomwareUserDlg::OnInitializationCompleted(WPARAM wParam, LPARAM lParam) // WM_INITIALIZATION_COMPLETED
{
	OnBnClickedButtonViewreport();
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

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CAntiRansomwareUserDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
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

	// CRITICAL SECTION - Delete
	DeleteCriticalSection(&m_csScanLog);
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


void CAntiRansomwareUserDlg::AddLogList(CString msg, bool wTime)
{
	int listNum;
	CString strTime;

	if (!m_isRunning)
		return;

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
	AddLogList("InitMyScanner()", true);

	pThreadCommunication = AfxBeginThread(CommunicationMyScanner, this, THREAD_PRIORITY_NORMAL, 0, 0);
	if (pThreadCommunication == NULL) {
		AddLogList("[Error] Fail to create moving thread.", true);
		return false;
	}

	return false;
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
					strMsg.Format("[%d] fltType_PreCreate", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PostCreate:
					strMsg.Format("[%d] fltType_PostCreate", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PreClose:
					strMsg.Format("[%d] fltType_PreClose", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PostClose:
					strMsg.Format("[%d] fltType_PostClose", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PreCleanup:
					strMsg.Format("[%d] fltType_PreCleanup", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PostCleanup:
					strMsg.Format("[%d] fltType_PostCleanup", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PreSetInformation:
					strMsg.Format("[%d] fltType_PreSetInformation", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PostSetInformation:
					strMsg.Format("[%d] fltType_PostSetInformation", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
			}
		}

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