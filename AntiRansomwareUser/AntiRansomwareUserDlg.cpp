
// AntiRansomwareUserDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "AntiRansomwareUser.h"
#include "AntiRansomwareUserDlg.h"
#include "afxdialogex.h"


const char* g_szBackupPath = "\\_SafeBackup"; // 백업 폴더
const char* g_szBackupExt = "txt,hwp,doc,docx,ppt,pptx,xls,xlsx,c,cpp,h,hpp,bmp,jpg,gif,png,zip,rar"; // 보호 파일 확장자

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SYSTEMTIME g_time; // 시간 구조체.

HWND hWnd;
CAntiRansomwareUserDlg *g_pParent;

// Rename용 Driver Letter (임시)
wchar_t szLastDriveLetter[8];

// Backup용 Device Name (임시)
wchar_t g_szDeviceName[128];

UCHAR FoulString[] = "foul";

// CAntiRansomwareUserDlg 대화 상자



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


// CAntiRansomwareUserDlg 메시지 처리기

BOOL CAntiRansomwareUserDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	hWnd = AfxGetMainWnd()->m_hWnd; // GET MAIN HANDLE
	g_pParent = this; // 부모 개체 정의

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

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}


LRESULT CAntiRansomwareUserDlg::OnInitializationCompleted(WPARAM wParam, LPARAM lParam) // WM_INITIALIZATION_COMPLETED
{
	OnBnClickedButtonViewreport();
	return S_OK;
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CAntiRansomwareUserDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CAntiRansomwareUserDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CAntiRansomwareUserDlg::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialogEx::OnMoving(fwSide, pRect);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (m_pAntiRansomwareReportDlg.GetSafeHwnd() != NULL) {
		m_pAntiRansomwareReportDlg.SetWindowPos(NULL, pRect->right, pRect->top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}
}


void CAntiRansomwareUserDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

	// CRITICAL SECTION - Delete
	DeleteCriticalSection(&m_csScanLog);
}


void CAntiRansomwareUserDlg::OnBnClickedButtonViewreport()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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

	::ZeroMemory(reinterpret_cast<void*>(&g_time), sizeof(g_time)); // time 초기화.
	::GetLocalTime(&g_time);    // 현재시간을 얻음.

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


	//STEP 1: 장치명 분리.. \Device\HarddiskVolume2
	//첫번째 \Device\를 넘기위해 pPos +8 해줌
	pPos = wcschr(pPos + 8, L'\\');
	if (pPos == NULL) {
		return FALSE;
	}
	if (devicename != NULL && lenDevicename > 0)
		wcsncpy_s(devicename, lenDevicename, path, pPos - path);

	//STEP 2: 나머지 경로분리.. \MyTest\123.txt
	_wsplitpath_s(pPos, NULL, 0, dir, lenDir, fname, lenFname, ext, lenExt);

	return TRUE;

}

//\Device\HarddiskVolume2 --> c: 로 변환.. 
BOOL GetDeviceNameToDriveLetter(const wchar_t * pDevicePath, wchar_t * bufDriveLetter, size_t bufLen)
{
	WCHAR szDriverString[MAX_PATH] = { 0, };
	WCHAR swTempDeviceName[MAX_PATH];
	WCHAR szDriveLetter[8];
	LPCWSTR pPos = NULL;

	if (pDevicePath == NULL || bufDriveLetter == NULL || bufLen <= 0)
		return FALSE;

	// 전체 드라이브 문자열을 구함. Ex) "A:\ C:\ D:\ "
	GetLogicalDriveStringsW(_countof(szDriverString), szDriverString);
	pPos = szDriverString;
	while (*pPos != TEXT('\0'))
	{
		StringCchPrintfW(szDriveLetter, _countof(szDriveLetter), L"%c:", *pPos);
		if (QueryDosDeviceW(szDriveLetter, swTempDeviceName, _countof(swTempDeviceName)) > 0)
		{
			//비교.. 
			if (wcsncmp(swTempDeviceName, pDevicePath, _countof(swTempDeviceName)) == 0)
			{
				StringCchCopyW(bufDriveLetter, bufLen, szDriveLetter);
				break;
			}
		}
		// 다음 디스크 (4 문자)
		pPos = &pPos[4];
	}

	return TRUE;
}

BOOL ConvertDevicePathToDrivePath(const wchar_t * pDevicePath, wchar_t *bufPath, size_t bufPathLen)
{
	wchar_t szDeviceName[128] = { 0 };
	wchar_t szDirPath[MAX_PATH] = { 0 };
	wchar_t szFileName[MAX_PATH] = { 0 };
	wchar_t szExt[32] = { 0 };
	wchar_t szDriveLetter[8] = { 0 };

	if (pDevicePath == NULL || bufPath == NULL || bufPathLen <= 0)
		return FALSE;

	//STEP 1 : device 경로를 드라이브 경로로 변경한다. 
	//ex : \Device\HarddiskVolume2\MyTest\123.txt --> C:\MyTest\123.txt

	//STEP 1 : 디바이스 경로 산산히 분해.. 
	if (splitDevicePath(pDevicePath, szDeviceName, 128, szDirPath, MAX_PATH, szFileName, MAX_PATH, szExt, 32) == FALSE)
		return FALSE;

	//SETP 2 : 디바이스 명을 드라이브 레터로 변경.. 
	//ex : \Device\HarddiskVolume2\ --> C:\

	wcscpy(g_szDeviceName, szDeviceName); // backup용 (임시)

	if (GetDeviceNameToDriveLetter(szDeviceName, szDriveLetter, 8) == FALSE)
		return FALSE;

	// Reneme용 Drive Letter 저장 (임시)
	memcpy(szLastDriveLetter, szDriveLetter, sizeof(szLastDriveLetter));

	// 가끔 알수없는 경로 때문에.. ex. DR
	if (wcslen(szDriveLetter) == 0 || wcslen(szDirPath) == 0)
		return FALSE;

	//STEP 3 : 경로 재조립.. 
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
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PostCreate:
					strMsg.Format("===== [%d] fltType_PostCreate =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					nResult = g_pParent->RecordProcessBehavior(notification); // 프로세스 행위 기록
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
					nResult = g_pParent->RecordProcessBehavior(notification); // 프로세스 행위 기록
					break;
				case fltType_PostCleanup:
					strMsg.Format("===== [%d] fltType_PostCleanup =====", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					break;
				case fltType_PreWrite:
					strMsg.Format("===== [%d] fltType_PreWrite =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					nResult = g_pParent->RecordProcessBehavior(notification); // 프로세스 행위 기록
					break;
				case fltType_PostWrite:
					strMsg.Format("===== [%d] fltType_PostWrite =====", notification->ulPID);
					g_pParent->AddLogList(strMsg, true);
					//nResult = g_pParent->RecordProcessBehavior(notification); // 프로세스 행위 기록
					break;
				case fltType_PreSetInformation:
					strMsg.Format("===== [%d] fltType_PreSetInformation =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					nResult = g_pParent->RecordProcessBehavior(notification); // 프로세스 행위 기록
					break;
				case fltType_PostSetInformation:
					strMsg.Format("===== [%d] fltType_PostSetInformation =====", notification->ulPID);
					//g_pParent->AddLogList(strMsg, true);
					nResult = g_pParent->RecordProcessBehavior(notification); // 프로세스 행위 기록
					break;
			}
		}

		replyMessage.Reply.cmdType = nResult; // Command
		if (nResult == 100) { // Backup
			CString strBackupPath;
			int nPos;
			wchar_t szFilePath[MAX_FILE_PATH] = { 0 };
			wchar_t szBackupPath[MAX_FILE_PATH] = { 0 };
			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH) == FALSE)
				break;

			strBackupPath = g_pParent->GetBackupFilePath((CString)szFilePath);
			nPos = strBackupPath.Find('\\');
			if (nPos >= 0) {
				strBackupPath.Format("%S%s", g_szDeviceName, strBackupPath.Mid(nPos));
				g_pParent->AddLogList(strBackupPath);
			}
			memcpy(replyMessage.Reply.Contents, (const char *)strBackupPath, MAX_FILE_PATH);
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


inline void WaitG(double dwMillisecond)
{
	MSG msg;
	LARGE_INTEGER   st, ed, freq;
	double			WaitFrequency;
	QueryPerformanceFrequency(&freq);
	WaitFrequency = ((double)dwMillisecond / 1000) * ((double)freq.QuadPart);

	if (freq.QuadPart == 0)
	{
		//::SetDlgItemText(hWnd,IDC_EDIT_Status,"Warning! - 고해상도 타이머 지원 안함.");
		//AddListLog(1, "지원 안함.");
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

	strTemp.Format("■Kill: %d", pid);
	g_pParent->AddLogList(strTemp);
	DoKillProcess(pid);

	if (Process32First(handle, &pe))
	{
		do
		{
			if (pe.th32ParentProcessID == pid) {
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
		if (result == false)
			return false;
	}
	if (isFound == false) {
		return false;
	}

	return prev_pid;
}


int GetPermissionDirectory(CString strPath, DWORD pid = 0)
{
	int nPos;
	CString strSafePath;
	DWORD dwProcId;

	// 백업 디렉토리 설정
	nPos = strPath.Find('\\');
	if (nPos == -1)
		return 0;

	strSafePath.Format("%s%s", strPath.Left(nPos), g_szBackupPath);

	if (strPath.Find(strSafePath, 0) >= 0) {
		GetWindowThreadProcessId(hWnd, &dwProcId);
		if (pid != dwProcId) {
			//g_pParent->AddLogList("!! 권한 없음 !!");
			return 3;
		}
		else {
			return 2;
		}
	}
	else {
		return 1;
	}

	return 0;
}

bool RefreshDesktopDirectory()
{
	char szPath[1024];
	LPITEMIDLIST pItemIDList;
	SHGetSpecialFolderLocation(GetDesktopWindow(), CSIDL_DESKTOP, &pItemIDList);
	SHGetPathFromIDList(pItemIDList, szPath);
	
	// 바탕화면 새로고침
	SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT, szPath, NULL);

	return true;
}

// 프로세스 행위 기록
int CAntiRansomwareUserDlg::RecordProcessBehavior(PSCANNER_NOTIFICATION notification)
{
	bool result;
	int nResult;
	int nReturn = 0;
	CString strTemp;
	CString strBackupPath;
	wchar_t szFilePath[MAX_FILE_PATH] = { 0 };
	PROCESS_EVENT tmpPE;
	unsigned int numEvent;
	DWORD pid;

	// 신규 프로세스 등록
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

		strTemp.Format("map 생성: [%d] %s", notification->ulPID, (CString)pe.szExeFile);
		AddLogList(strTemp, true);
		
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
				nResult = GetPermissionDirectory((CString)szFilePath);
				if (nResult > 1)
					break;
				strTemp.Format("[신규] %s: %s", (notification->isDir)? "Dir" : "File", (CString)szFilePath);
				AddLogList(strTemp);
				AddEventNewFile(notification->ulPID, notification->isDir, (CString)szFilePath); // Add Event
			}
			else if (notification->CreateOptions == 2) {
				// Process Event
				m_mapProcessBehavior[notification->ulPID].cntWrite++;

				strTemp.Format("[덮어쓰기] %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
				AddLogList(strTemp);
				if(DoBackupFile((CString)szFilePath)) // 파일 백업
					nReturn = 100; // Backup!
			}
			else {
				if(!notification->isDir){
					// Process Event
					m_mapProcessBehavior[notification->ulPID].cntWrite++;

					strTemp.Format("[수정] %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
					AddLogList(strTemp);
					if (DoBackupFile((CString)szFilePath)) // 파일 백업
						nReturn = 100; // Backup!
				}
			}
			break;
		case fltType_PreCleanup:
			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH) == FALSE)
				break;

			strTemp.Format("[Cleanup] %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
			AddLogList(strTemp);
			if (!notification->isDir){
				result = DoCheckRansomware((CString)szFilePath); // 랜섬웨어 감염 확인
				if (result == true) { // 파일 변조됨
					if(m_mapProcessBehavior[notification->ulPID].cntWrite_sp < 10){
						AddEventWriteFile(notification->ulPID, (CString)szFilePath); // 의심
					}
					else {
						AddLogList("랜섬웨어 탐지!");
						ctr_editTargetPid.SetWindowTextA("랜섬웨어 탐지!");

						pid = FindRansomwareParantPID(notification->ulPID);
						DoKillProcessTree(pid); // 프로세스 트리 종료
						DoKillRecoveryRansomware(pid); // 파일 복구
						RefreshDesktopDirectory();
					}
				}
			}
			break;
		case fltType_PreWrite:
			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH) == FALSE)
				break;
			strTemp.Format("[쓰기] %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
			AddLogList(strTemp);
			break;
		case fltType_PreSetInformation:
			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH) == FALSE)
				break;

			if (!notification->modeDelete) {
				strTemp.Format("[이름 변경] 변경 전 - %s: %s%s", (notification->isDir) ? "Dir" : "File", (CString)szLastDriveLetter, (CString)(wchar_t*)notification->OrgFileName);
				AddLogList(strTemp);
				strTemp.Format("[이름 변경] 변경 후 - %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
				AddLogList(strTemp);
				AddEventRenameFile(notification->ulPID, (CString)szLastDriveLetter + (CString)(wchar_t*)notification->OrgFileName, (CString)szFilePath); // Add Event
			}
			break;
		case fltType_PostSetInformation:
			if (!notification->modeDelete) break;
			if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH) == FALSE)
				break;

			nResult = GetPermissionDirectory((CString)szFilePath, notification->ulPID);
			if (nResult == 3) {
				nReturn = 1; // 권한 없음
				break;
			}

			strTemp.Format("[삭제] %s: %s", (notification->isDir) ? "Dir" : "File", (CString)szFilePath);
			AddLogList(strTemp);
			AddEventDeleteFile(notification->ulPID, (CString)szFilePath); // Add Event
			if (DoBackupFile((CString)szFilePath)) // 파일 백업
				nReturn = 100; // Backup!
			break;
	}

	return nReturn;
}


// 파일, 디렉토리 생성 이벤트 추가
bool CAntiRansomwareUserDlg::AddEventNewFile(DWORD pid, bool isDirectory, CString strPath)
{
	PROCESS_EVENT tmpPE;
	ITEM_NEW_FILE tmpINF;
	unsigned int numEvent = m_numNewFile++;

	// New File Event
	tmpINF.num = numEvent;
	tmpINF.isDirectory = isDirectory;
	tmpINF.strPath = strPath;
	m_listNewFile.push_back(tmpINF);

	// Process Event
	m_mapProcessBehavior[pid].cntCreate++;

	tmpPE.mode = 0; // new
	tmpPE.numEvent = numEvent;
	m_mapProcessBehavior[pid].stackEventRecord.push(tmpPE);

	return true;
}


// 파일(디렉토리) 이름 변경 이벤트 추가
bool CAntiRansomwareUserDlg::AddEventRenameFile(DWORD pid, CString strSrc, CString strDst)
{
	PROCESS_EVENT tmpPE;
	ITEM_RENAME_FILE tmpIRF;
	unsigned int numEvent = m_numRenameFile++;

	// Rename File Event
	tmpIRF.num = numEvent;
	tmpIRF.strSrc = strSrc;
	tmpIRF.strDst = strDst;
	m_listRenameFile.push_back(tmpIRF);

	// Process Event
	m_mapProcessBehavior[pid].cntRename++;

	tmpPE.mode = 1; // rename
	tmpPE.numEvent = numEvent;
	m_mapProcessBehavior[pid].stackEventRecord.push(tmpPE);

	return true;
}


// 파일 쓰기 이벤트 추가
bool CAntiRansomwareUserDlg::AddEventWriteFile(DWORD pid, CString strPath)
{
	unsigned int tmpBackupNum = -1;
	PROCESS_EVENT tmpPE;
	ITEM_WRITE_FILE tmpIWF;
	list<ITEM_WRITE_FILE>::reverse_iterator ritorIWF; // write
	unsigned int numEvent = m_numWriteFile++;

	ritorIWF = m_listWriteFile.rbegin();
	while (ritorIWF != m_listWriteFile.rend())
	{
		if (pid == ritorIWF->pid && strPath.Compare(ritorIWF->strPath) == 0) {
			tmpBackupNum = ritorIWF->num_back;
			ritorIWF->num_back = -1; // 이전 백업 기록 삭제(중복 복구 방지)
			m_mapProcessBehavior[pid].cntWrite_sp--;
			break;
		}
		ritorIWF++;
	}

	// 백업 기록 없을 경우
	if (tmpBackupNum == -1) {
		tmpBackupNum = AddEventBackupFile(pid, strPath); // 백업 이벤트
	}

	// Write File Event
	tmpIWF.num = numEvent;
	tmpIWF.pid = pid;
	tmpIWF.num_back = tmpBackupNum;
	tmpIWF.strPath = strPath;
	m_listWriteFile.push_back(tmpIWF);

	// Process Event
	m_mapProcessBehavior[pid].cntWrite_sp++;

	tmpPE.mode = 2; // write
	tmpPE.numEvent = numEvent;
	m_mapProcessBehavior[pid].stackEventRecord.push(tmpPE);

	return true;
}


bool CAntiRansomwareUserDlg::AddEventDeleteFile(DWORD pid, CString strPath)
{
	unsigned int tmpBackupNum = -1;
	PROCESS_EVENT tmpPE;
	ITEM_DELETE_FILE tmpIDF;
	list<ITEM_DELETE_FILE>::reverse_iterator ritorIDF; // delete
	unsigned int numEvent = m_numDeleteFile++;

	ritorIDF = m_listDeleteFile.rbegin();
	while (ritorIDF != m_listDeleteFile.rend())
	{
		if (pid == ritorIDF->pid && strPath.Compare(ritorIDF->strPath) == 0) {
			tmpBackupNum = ritorIDF->num_back;
			ritorIDF->num_back = -1; // 이전 백업 기록 삭제(중복 복구 방지)
			m_mapProcessBehavior[pid].cntDelete--;
			break;
		}
		ritorIDF++;
	}

	// 백업 기록 없을 경우
	if (tmpBackupNum == -1) {
		tmpBackupNum = AddEventBackupFile(pid, strPath); // 백업 이벤트
	}

	// Write File Event
	tmpIDF.num = numEvent;
	tmpIDF.pid = pid;
	tmpIDF.num_back = tmpBackupNum;
	tmpIDF.strPath = strPath;
	m_listDeleteFile.push_back(tmpIDF);

	// Process Event
	m_mapProcessBehavior[pid].cntDelete++;

	tmpPE.mode = 3; // delete
	tmpPE.numEvent = numEvent;
	m_mapProcessBehavior[pid].stackEventRecord.push(tmpPE);

	return true;
}


// 파일 백업 이벤트 추가
unsigned int CAntiRansomwareUserDlg::AddEventBackupFile(DWORD pid, CString strPath)
{
	bool result;
	ITEM_BACKUP_FILE tmpIBF;
	CString strBackupPath;
	unsigned int numEvent;

	numEvent = m_numBackupFile++;
	
	tmpIBF.num = numEvent;
	tmpIBF.strPath = GetBackupFilePath(strPath);
	m_listBackupFile.push_back(tmpIBF);

	return numEvent;
}

void CreateFolder(CString p_strTargetRoot)
{
	/* 폴더가 있는지 조사하고 없으면 생성한다 */
	if (_waccess((LPWSTR)(LPCTSTR)p_strTargetRoot, 0) != 0) {
		BOOL l_bExtractSucc = TRUE;

		int j = 0;
		CString l_strDirPath = _T("");
		CString l_strNowPath = _T("");
		while (l_bExtractSucc)
		{
			/* 상위 폴더부터 생성해 나간다 */
			l_bExtractSucc = AfxExtractSubString(l_strNowPath, p_strTargetRoot, j, '\\');
			l_strDirPath += l_strNowPath + _T("\\");
			if (l_bExtractSucc && _waccess((LPWSTR)(LPCTSTR)l_strDirPath, 0) != 0) {
				CreateDirectory(l_strDirPath, NULL);
			}
			j++;
		}
	}
}

CString CAntiRansomwareUserDlg::GetBackupFilePath(CString strPath)
{
	int nPos;
	CString strNewPath;

	// 백업 디렉토리 설정
	nPos = strPath.Find('\\');
	strNewPath.Format("%s%s%s", strPath.Left(nPos), g_szBackupPath, strPath.Mid(nPos));

	return strNewPath;
}

bool CAntiRansomwareUserDlg::DoBackupFile(CString strPath)
{
	int nResult;
	DWORD nFileSize;
	CString strNewDir;
	CString strNewPath;
	HANDLE hFile;

	nResult = GetPermissionDirectory(strPath);
	if (nResult > 1){
		AddLogList("백업 보류: Safe Directory");
		return false;
	}
	/*
	hFile = CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		AddLogList("백업 실패: INVALID_HANDLE_VALUE");
		return false;
	}

	nFileSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);

	if (nFileSize < 10 || nFileSize > 20971520){
		AddLogList("백업 보류: file size");
		return false; // 백업 안함 (10byte 미만 or 20MB 초과)
	}
	*/
	// 백업 경로 구하기
	strNewPath = GetBackupFilePath(strPath);

	// 백업 하위 디렉토리 생성
	strNewDir.SetString(strNewPath);
	PathRemoveFileSpec((LPSTR)(LPCTSTR)strNewDir);
	CreateFolder(strNewDir);

	// 미니필터에서 백업
	/*
	// 파일 백업 - 덮어쓰기 안함
	AddLogList("Backup: " + strNewPath);
	nResult = CopyFile(strPath, strNewPath, TRUE);
	*/

	return true;
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


bool CAntiRansomwareUserDlg::DoCheckRansomware(CString strPath)
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

	// 백업 경로 구하기
	strBackupPath = GetBackupFilePath(strPath);

	// 백업 파일 존재 확인
	if (PathFileExists(strBackupPath) == FALSE) {
		AddLogList("백업 파일 없음: " + strBackupPath);
		return false;
	}

	// 비교 파일 열기
	fpTarget = fopen((LPSTR)(LPCTSTR)strPath, "rb");
	if (fpTarget == NULL){
		AddLogList("파일 열기 실패 : fpTarget");
		return false;
	}

	fpBackup = fopen((LPSTR)(LPCTSTR)strBackupPath, "rb");
	if (fpBackup == NULL){
		AddLogList("파일 열기 실패 : fpBackup");
		fclose(fpTarget);
		return false;
	}

	// STEP1: 파일 시그니처 비교
	size_target = fread(buf_sigTarget, 1, 4, fpTarget);
	size_backup = fread(buf_sigBackup, 1, 4, fpBackup);
	if (size_target == size_backup) {
		nResult = memcmp(buf_sigTarget, buf_sigBackup, 4);
		if (nResult != 0) {
			AddLogList("Warning: 파일 시그니처 변조");
			fclose(fpTarget);
			fclose(fpBackup);
			return true;
		}
	}
	else {
		AddLogList("Warning: 파일 시그니처 제거");
		fclose(fpTarget);
		fclose(fpBackup);
		return true;
	}

	// STEP2: 파일 정보 엔트로피 분석(text)
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
	strTemp.Format("엔트로피 차: %02.5f, %s", abs(entropy_target - entropy_backup), (isChangeEntropy)? "변화됨" : "변화 없음");
	AddLogList(strTemp);
	
	if(isChangeEntropy){
		return true;
	}

	// STEP3: ssdeep? (binary)


	fclose(fpTarget);
	fclose(fpBackup);

	// 백업 파일 삭제(다른 백업 이벤트가 없다면)
	ritorIBF = m_listBackupFile.rbegin();
	while (ritorIBF != m_listBackupFile.rend())
	{
		if (strBackupPath.Compare(ritorIBF->strPath) == 0) {
			AddLogList("백업 파일 삭제 안함");
			isDeleteBackupFile = false;
			break;
		}
		ritorIBF++;
	}
	if (isDeleteBackupFile == true) {
		AddLogList("백업 파일 삭제: " + strBackupPath);
		DeleteFile(strBackupPath); // 원본 파일 삭제
	}

	return false;
}


bool CAntiRansomwareUserDlg::DoKillRecoveryRansomware(DWORD pid)
{
	map<unsigned int, PROCESS_BEHAVIOR>::iterator itor;

	// 행위 데이터 검색
	for (itor = m_mapProcessBehavior.begin(); itor != m_mapProcessBehavior.end(); ++itor) {
		if (itor->second.ppid == pid) {
			RecoveryProcessBehavior(itor->second.pid); // 파일 복구
			DoKillRecoveryRansomware(itor->second.pid); // 재귀 호출
		}
	}

	RecoveryProcessBehavior(pid); // 파일 복구

	return true;
}


bool CAntiRansomwareUserDlg::DoRecoveryFile(unsigned int num_back, CString strPath)
{
	bool result = false;
	ITEM_BACKUP_FILE tmpIBF;
	list<ITEM_BACKUP_FILE>::reverse_iterator ritorIBF; // backup
	CString strRecoveryPath;

	ritorIBF = m_listBackupFile.rbegin();
	while (ritorIBF != m_listBackupFile.rend())
	{
		if (ritorIBF->num == num_back) {
			strRecoveryPath = ritorIBF->strPath;
			result = true;
			list<ITEM_BACKUP_FILE>::iterator itorIBF = ritorIBF.base();
			m_listBackupFile.erase(--itorIBF);
			break;
		}
		ritorIBF++;
	}
	if (!result){
		AddLogList("백업 리스트 없음", true);
		return false;
	}

	AddLogList("Recovery: " + strRecoveryPath, true);
	DeleteFile(strPath); // 원본 파일 삭제
	CopyFile(strRecoveryPath, strPath, FALSE); // 파일 복사(덮어쓰기)

	return true;
}

// 프로세스 행위 복구
bool CAntiRansomwareUserDlg::RecoveryProcessBehavior(DWORD pid)
{
	HANDLE handle = NULL;
	PROCESSENTRY32 pe = { 0 };
	PROCESS_EVENT tmpPE;
	list<ITEM_NEW_FILE>::reverse_iterator ritorINF; // new
	list<ITEM_RENAME_FILE>::reverse_iterator ritorIRF; // rename
	list<ITEM_WRITE_FILE>::reverse_iterator ritorIWF; // write
	list<ITEM_DELETE_FILE>::reverse_iterator ritorIDF; // delete
	ITEM_NEW_FILE tmpINF;
	CString strTemp;

	if (m_mapProcessBehavior.find(pid) == m_mapProcessBehavior.end())
		return false;

	strTemp.Format("========== 행위 복구: [%d] %s ==========", pid, m_mapProcessBehavior[pid].strName);
	AddLogList(strTemp, true);

	strTemp.Format("create: %d / rename: %d / write: %d(%d) / delete: %d", m_mapProcessBehavior[pid].cntCreate
																		 , m_mapProcessBehavior[pid].cntRename
																		 , m_mapProcessBehavior[pid].cntWrite
																		 , m_mapProcessBehavior[pid].cntWrite_sp
																		 , m_mapProcessBehavior[pid].cntDelete
	);
	AddLogList(strTemp, true);
	
	// pid에 해당하는 프로세스의 행위를 역순으로 복구
	while (!m_mapProcessBehavior[pid].stackEventRecord.empty()) {
		tmpPE = m_mapProcessBehavior[pid].stackEventRecord.top();
		m_mapProcessBehavior[pid].stackEventRecord.pop();
		switch (tmpPE.mode)
		{
			case 0: // 파일, 디렉토리 생성 -> 파일, 디렉토리 삭제
				ritorINF = m_listNewFile.rbegin();
				while (ritorINF != m_listNewFile.rend())
				{
					if (tmpPE.numEvent == ritorINF->num) {
						AddLogList("Delete: " + ritorINF->strPath, true);
						if (ritorINF->isDirectory) {
							RemoveDirectory(ritorINF->strPath); // Directory
						}else{
							DeleteFile(ritorINF->strPath); // File
						}
						list<ITEM_NEW_FILE>::iterator itorINF = ritorINF.base();
						m_listNewFile.erase(--itorINF);
						break;
					}
					ritorINF++;
				}
				break;
			case 1: // 이름 변경 -> 이름 변경
				ritorIRF = m_listRenameFile.rbegin();
				while (ritorIRF != m_listRenameFile.rend())
				{
					if (tmpPE.numEvent == ritorIRF->num) {
						AddLogList("Rename: 변경 전 " + ritorIRF->strDst, true);
						AddLogList("Rename: 변경 후 " + ritorIRF->strSrc, true);
						MoveFileEx(ritorIRF->strDst, ritorIRF->strSrc, MOVEFILE_COPY_ALLOWED); // 파일 이름 변경
						list<ITEM_RENAME_FILE>::iterator itorIRF = ritorIRF.base();
						m_listRenameFile.erase(--itorIRF);
						break;
					}
					ritorIRF++;
				}
				break;
			case 2: // 파일 쓰기 -> 파일 복구
				ritorIWF = m_listWriteFile.rbegin();
				while (ritorIWF != m_listWriteFile.rend())
				{
					if (tmpPE.numEvent == ritorIWF->num) {
						if(ritorIWF->num_back != -1){
							DoRecoveryFile(ritorIWF->num_back, ritorIWF->strPath); // 파일 복구
						}
						else {
							AddLogList("Pass: " + ritorIWF->strPath, true); // Pass
						}
						list<ITEM_WRITE_FILE>::iterator itorIWF = ritorIWF.base();
						m_listWriteFile.erase(--itorIWF);
						break;
					}
					ritorIWF++;
				}
				break;
			case 3: // 파일 삭제 -> 파일 복구
				ritorIDF = m_listDeleteFile.rbegin();
				while (ritorIDF != m_listDeleteFile.rend())
				{
					if (tmpPE.numEvent == ritorIDF->num) {
						if (ritorIDF->num_back != -1) {
							DoRecoveryFile(ritorIDF->num_back, ritorIDF->strPath); // 파일 복구
						}
						else {
							AddLogList("Pass: " + ritorIWF->strPath, true); // Pass
						}
						list<ITEM_DELETE_FILE>::iterator itorIDF = ritorIDF.base();
						m_listDeleteFile.erase(--itorIDF);
						break;
					}
					ritorIDF++;
				}
				break;
		}
	}

	return false;
}

void CAntiRansomwareUserDlg::OnBnClickedButtonRecovery()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strTemp;
	ctr_editTargetPid.GetWindowTextA(strTemp);
	RecoveryProcessBehavior((DWORD)atoi((LPSTR)(LPCTSTR)strTemp));
	//DoKillRecoveryRansomware((DWORD)atoi((LPSTR)(LPCTSTR)strTemp));
	//DoKillProcessTree((DWORD)atoi((LPSTR)(LPCTSTR)strTemp));
	//DeleteFile("F:\MyTest\\test - 복사본.txt");
	/*
	FILE* fp;
	float ent;

	fp = fopen("F:\\text.xxx", "rb");
	if (fp == NULL)
		return;

	ent = GetFileEntropy(fp, 0);

	strTemp.Format("%02.5f", ent);
	AddLogList(strTemp);

	fclose(fp);
	*/

}
