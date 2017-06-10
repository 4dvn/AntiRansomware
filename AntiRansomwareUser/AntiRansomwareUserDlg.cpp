
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

// Rename�� Driver Letter (�ӽ�)
wchar_t szLastDriveLetter[8];

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
	DDX_Control(pDX, IDC_EDIT_TARGET_PID, ctr_editTargetPid);
}

BEGIN_MESSAGE_MAP(CAntiRansomwareUserDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(WM_INITIALIZATION_COMPLETED, OnInitializationCompleted) // WM_INITIALIZATION_COMPLETED
	ON_BN_CLICKED(IDC_BUTTON_ViewReport, &CAntiRansomwareUserDlg::OnBnClickedButtonViewreport)
	ON_WM_MOVING()
	ON_BN_CLICKED(IDC_BUTTON_RECOVERY, &CAntiRansomwareUserDlg::OnBnClickedButtonRecovery)
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

	m_numNewFile = 0;
	m_numWriteFile = 0;
	m_numRenameFile = 0;
	m_numDeleteFile = 0;
	m_numBackupFile = 0;

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


BOOL splitDevicePath(const wchar_t * path,
	wchar_t * devicename, size_t lenDevicename,
	wchar_t * dir, size_t lenDir,
	wchar_t * fname, size_t lenFname,
	wchar_t * ext, size_t lenExt)
{
	size_t pathLength;
	const wchar_t * pPos = path;

	StringCchLengthW(path, MAX_PATH, &pathLength);
	if (path == NULL || pathLength <= 8)
		return FALSE;

	//path EX: \Device\HarddiskVolume2\MyTest\123.txt 
	if (path == NULL)
		return FALSE;


	//STEP 1: ��ġ�� �и�.. \Device\HarddiskVolume2
	//ù��° \Device\�� �ѱ����� pPos +8 ����
	pPos = wcschr(pPos + 8, L'\\');
	if (devicename != NULL && lenDevicename > 0)
		wcsncpy_s(devicename, lenDevicename, path, pPos - path);

	//STEP 2: ������ ��κи�.. \MyTest\123.txt
	_wsplitpath_s(pPos, NULL, 0, dir, lenDir, fname, lenFname, ext, lenExt);

	return TRUE;

}

//\Device\HarddiskVolume2 --> c: �� ��ȯ.. 
BOOL GetDeviceNameToDriveLetter(const wchar_t * pDevicePath, wchar_t * bufDriveLetter, size_t bufLen)
{
	WCHAR szDriverString[MAX_PATH] = { 0, };
	WCHAR swTempDeviceName[MAX_PATH];
	WCHAR szDriveLetter[8];
	LPCWSTR pPos = NULL;

	if (pDevicePath == NULL || bufDriveLetter == NULL || bufLen <= 0)
		return FALSE;

	// ��ü ����̺� ���ڿ��� ����. Ex) "A:\ C:\ D:\ "
	GetLogicalDriveStringsW(_countof(szDriverString), szDriverString);
	pPos = szDriverString;
	while (*pPos != TEXT('\0'))
	{
		StringCchPrintfW(szDriveLetter, _countof(szDriveLetter), L"%c:", *pPos);
		if (QueryDosDeviceW(szDriveLetter, swTempDeviceName, _countof(swTempDeviceName)) > 0)
		{
			//��.. 
			if (wcsncmp(swTempDeviceName, pDevicePath, _countof(swTempDeviceName)) == 0)
			{
				StringCchCopyW(bufDriveLetter, bufLen, szDriveLetter);
				break;
			}
		}
		// ���� ��ũ (4 ����)
		pPos = &pPos[4];
	}

	return TRUE;
}

BOOL ConvertDevicePathToDrivePath(const wchar_t * pDevicePath, wchar_t *bufPath, size_t bufPathLen)
{
	wchar_t szDeviceName[128];
	wchar_t szDirPath[MAX_PATH];
	wchar_t szFileName[MAX_PATH];
	wchar_t szExt[32];
	wchar_t szDriveLetter[8];

	if (pDevicePath == NULL || bufPath == NULL || bufPathLen <= 0)
		return FALSE;

	//STEP 1 : device ��θ� ����̺� ��η� �����Ѵ�. 
	//ex : \Device\HarddiskVolume2\MyTest\123.txt --> C:\MyTest\123.txt

	//STEP 1 : ����̽� ��� ����� ����.. 
	if (splitDevicePath(pDevicePath, szDeviceName, 128, szDirPath, MAX_PATH, szFileName, MAX_PATH, szExt, 32) == FALSE)
		return FALSE;

	//SETP 2 : ����̽� ���� ����̺� ���ͷ� ����.. 
	//ex : \Device\HarddiskVolume2\ --> C:\

	if (GetDeviceNameToDriveLetter(szDeviceName, szDriveLetter, 8) == FALSE)
		return FALSE;

	// Reneme�� Drive Letter ���� (�ӽ�)
	memcpy(szLastDriveLetter, szDriveLetter, sizeof(szLastDriveLetter));

	//STEP 3 : ��� ������.. 
	StringCchPrintfW(bufPath, bufPathLen, L"%s%s%s%s", szDriveLetter, szDirPath, szFileName, szExt);

	return TRUE;
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
					strMsg.Format("===== [%d] fltType_PreCreate =====", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PostCreate:
					strMsg.Format("===== [%d] fltType_PostCreate =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
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
					break;
				case fltType_PostCleanup:
					strMsg.Format("===== [%d] fltType_PostCleanup =====", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PreWrite:
					strMsg.Format("===== [%d] fltType_PreWrite =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
				case fltType_PostWrite:
					strMsg.Format("===== [%d] fltType_PostWrite =====", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					//g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
				case fltType_PreSetInformation:
					strMsg.Format("===== [%d] fltType_PreSetInformation =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
					break;
				case fltType_PostSetInformation:
					strMsg.Format("===== [%d] fltType_PostSetInformation =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					g_pParent->RecordProcessBehavior(notification); // ���μ��� ���� ���
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


bool CAntiRansomwareUserDlg::RecordProcessBehavior(PSCANNER_NOTIFICATION notification)
{
	CString strTemp;
	wchar_t szFilePath[MAX_PATH];
	PROCESS_EVENT tmpPE;
	unsigned int numEvent;

	// �ű� ���μ��� ���
	if (m_mapProcessBehavior.find(notification->ulPID) == m_mapProcessBehavior.end())
	{
		PROCESS_BEHAVIOR tmpPB;
		HANDLE handle = NULL;
		PROCESSENTRY32 pe = { 0 };
		pe.dwSize = sizeof(PROCESSENTRY32);
		handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (Process32First(handle, &pe))
		{
			do
			{
				if (pe.th32ProcessID == notification->ulPID)
					break;
			} while (Process32Next(handle, &pe));
		}

		//g_pParent->AddLogList((CString)pe.szExeFile, false);
		//strTemp.Format("map ����: %d", notification->ulPID);
		//AddLogList(strTemp, true);
		
		tmpPB.cntCreate = 0;
		tmpPB.cntDelete = 0;
		tmpPB.cntRename = 0;
		tmpPB.cntWrite = 0;
		tmpPB.cntWrite_sp = 0;
		tmpPB.pid = notification->ulPID; // pid
		tmpPB.ppid = pe.th32ParentProcessID; // ppid
		tmpPB.strName = pe.szExeFile; // process name

		m_mapProcessBehavior[notification->ulPID] = tmpPB;
	}

	// Process Info
	//strTemp.Format("%s / ppid: %d", m_mapProcessBehavior[notification->ulPID].strName, m_mapProcessBehavior[notification->ulPID].ppid);
	//AddLogList(strTemp, true);

	switch (notification->Reserved)
	{
		case fltType_PostCreate:
			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH) == FALSE)
				break;

			if (notification->CreateOptions == 1) {
				strTemp.Format("[�ű�] %s: %s", (notification->isDir)? "Dir" : "File", (CString)szFilePath);
				AddLogList(strTemp);
				m_mapProcessBehavior[notification->ulPID].cntCreate++;
				numEvent = AddEventNewFile(notification->isDir, (CString)szFilePath);
				tmpPE.mode = 0; // new
				tmpPE.numEvent = numEvent;
				m_mapProcessBehavior[notification->ulPID].stackEventRecord.push(tmpPE);
			}
			else if (notification->CreateOptions == 2) {
				strTemp.Format("[�����] %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
				AddLogList(strTemp);
			}
			else {
				if(!notification->isDir){
					strTemp.Format("[����] %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
					AddLogList(strTemp);
				}
			}
			break;
		case fltType_PreWrite:
			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH) == FALSE)
				break;
			strTemp.Format("[����] %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
			AddLogList(strTemp);
			break;
		case fltType_PreSetInformation:
			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH) == FALSE)
				break;

			if (!notification->modeDelete) {
				strTemp.Format("[�̸� ����] ���� �� - %s: %s%s", (notification->isDir) ? "Dir" : "File", (CString)szLastDriveLetter, (CString)(wchar_t*)notification->OrgFileName);
				AddLogList(strTemp);
				strTemp.Format("[�̸� ����] ���� �� - %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
				AddLogList(strTemp);
			}
			break;
		case fltType_PostSetInformation:
			if (!notification->modeDelete) break;
			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH) == FALSE)
				break;

			strTemp.Format("[����] %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
			AddLogList(strTemp);
			break;
	}


	return true;
}


unsigned int CAntiRansomwareUserDlg::AddEventNewFile(bool isDirectory, CString strPath)
{
	ITEM_NEW_FILE tmpINF;
	unsigned int numEvent = m_numNewFile++;

	tmpINF.num = numEvent;
	tmpINF.isDirectory = isDirectory;
	tmpINF.strPath = strPath;
	m_listNewFile.push_back(tmpINF);

	return numEvent;
}


bool CAntiRansomwareUserDlg::RecoveryProcessBehavior(DWORD pid)
{
	PROCESS_EVENT tmpPE;
	list<ITEM_NEW_FILE>::iterator itorINF;
	ITEM_NEW_FILE tmpINF;

	while (m_mapProcessBehavior[pid].stackEventRecord.empty() == false) {
		tmpPE = m_mapProcessBehavior[pid].stackEventRecord.top();
		m_mapProcessBehavior[pid].stackEventRecord.pop();
		switch (tmpPE.mode)
		{
			case 0:
				itorINF = m_listNewFile.begin();
				while (itorINF != m_listNewFile.end())
				{
					if (tmpPE.numEvent == itorINF->num) {
						AddLogList("Delete: " + itorINF->strPath, true);
						if (itorINF->isDirectory) {
							RemoveDirectory(itorINF->strPath);
						}else{
							DeleteFile(itorINF->strPath);
						}
						m_listNewFile.erase(itorINF);
						break;
					}
					itorINF++;
				}
				break;
		}
	}

	return false;
}

void CAntiRansomwareUserDlg::OnBnClickedButtonRecovery()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CString strTemp;
	ctr_editTargetPid.GetWindowTextA(strTemp);
	RecoveryProcessBehavior((DWORD)atoi((LPSTR)(LPCTSTR)strTemp));
}
