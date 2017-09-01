
// AntiRansomware - Process Behavior
//

#include "stdafx.h"
#include "AntiRansomwareRecord.h"
#include "AntiRansomwareUserDlg.h"

extern CAntiRansomwareUserDlg *g_pParent;


// Constructor
ArProcessBehavior::ArProcessBehavior(DWORD pid)
{
	CString strTemp;
	HANDLE handle = NULL;
	PROCESSENTRY32 pe = { 0 };
	char szPath[1024] = { 0 };

	pe.dwSize = sizeof(PROCESSENTRY32);
	handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(handle, &pe))
	{
		do
		{
			if (pe.th32ProcessID == pid)
				break;
		} while (Process32Next(handle, &pe));
	}

	m_pid = pid; // pid
	m_ppid = pe.th32ParentProcessID; // ppid
	m_strProcName = pe.szExeFile; // process name

	// process path
	handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, m_pid);
	if (GetModuleFileNameEx(handle, NULL, szPath, 1024)) {
		m_strProcPath = szPath;
		int nPos = m_strProcPath.ReverseFind('\\'); // �������� ��ο��� ���ϸ� ����
		if (nPos > 0)
			m_strProcPath = m_strProcPath.Left(nPos);
	}

	m_nTimeCount = 60;
	m_isRansomware = false;

	m_cntCreate = 0;
	m_cntDelete = 0;
	m_cntRename = 0;
	m_cntWrite = 0;
	m_cntWrite_sp = 0;

	// Process Info
	strTemp.Format("[%d] %s / ppid: %d", m_pid, m_strProcName, m_ppid);
	AddLogList(strTemp, true);
}


// ���μ��� ���� Ȯ��(Ÿ�̸�)
int ArProcessBehavior::CheckProcessInfo()
{
	// ���μ��� ���� ����

	// �������� ����
	if (!m_isRansomware && m_cntWrite_sp >= 10) {
		if(m_pid > 100)
			m_isRansomware = true;
		return 1;
	}

	// �Ϲ� ���μ��� ��Ž ����
	if (m_nTimeCount < 59){
		m_nTimeCount++;
	}
	else {
		m_nTimeCount = 0;
		if(m_cntWrite_sp > 1)
			m_cntWrite_sp--;
	}

	return 0;
}


// ���μ��� ���� ���
int ArProcessBehavior::RecordProcessBehavior(PSCANNER_NOTIFICATION notification)
{
	bool result;
	int nResult;
	int nReturn = 0;
	CString strTemp;
	CString strFilePath;
	CString strOrgFileName;
	CString strBackupPath;
	wchar_t szFilePath[MAX_FILE_PATH] = { 0 };
	PROCESS_EVENT tmpPE;
	unsigned int numEvent;
	bool isCheckFileExt;
	int nPermission = 0;

	// Process Info
	//strTemp.Format("[%d] %s / ppid: %d", m_pid, m_strProcName, m_ppid);
	//AddLogList(strTemp, true);

	// ��� ��ȯ
	if (ConvertDevicePathToDrivePath((wchar_t*)notification->Contents, szFilePath, MAX_PATH, &m_pathInfoEx) == FALSE)
		return nReturn;

	strFilePath = szFilePath;
	isCheckFileExt = g_pParent->DoCheckFileExtension(strFilePath); // Ȯ���� Ȯ��
	if (!isCheckFileExt) {
		// ���� Ȯ����
		md5_byte_t dstHashMD5[16];
		GenerateMD5(dstHashMD5, strFilePath); // MD5 ����
		if (CheckRenameFilePath(dstHashMD5) == true)
			isCheckFileExt = true;
	}

	// ��� �� ���� �˻� ����
	if ((strFilePath).Compare(g_pParent->m_strBackingUpPath) == 0) {
		if (notification->Reserved == fltType_PreCleanup)
			g_pParent->m_strBackingUpPath.Empty(); // ��� �� ���� ����
		return 0;
	}

	// ���� Ȯ��
	nPermission = g_pParent->GetPermissionDirectory(strFilePath, m_pid);

	switch (notification->Reserved)
	{
	case fltType_PreCreate:
		if (nPermission == 3) {
			nReturn = 1; // ���� ����
			break;
		}
		if (!isCheckFileExt) break; // ���� Ȯ����
		AddEventWriteFile(strFilePath, isCheckFileExt); // Add Event
		strTemp.Format("[�����] %s: %s", (notification->isDir) ? "Dir" : "File", strFilePath);
		AddLogList(strTemp);
		if(DoBackupFile(strFilePath, 1)) // ���� ���
			nReturn = 100; // Backup!
		break;
	case fltType_PostCreate:
		if (notification->CreateOptions == 1) {
			if (nPermission > 1)
				break;
			AddEventNewFile(notification->isDir, strFilePath); // Add Event
			if (isCheckFileExt) {
				strTemp.Format("[�ű�] %s: %s", (notification->isDir) ? "Dir" : "File", strFilePath);
				AddLogList(strTemp);
			}
		}
		else if (notification->CreateOptions == 2) {
			// ����� Post
			if (nPermission == 3) {
				nReturn = 1; // ���� ����
				break;
			}
			if (!isCheckFileExt) break; // ���� Ȯ����
			// Process Event
		}
		else {
			if (!notification->isDir) {
				if (nPermission == 3) {
					nReturn = 1; // ���� ����
					break;
				}
				if (!isCheckFileExt) break; // ���� Ȯ����
				if (notification->modeDelete) {
					// ���� ����
				}
				else {
					// Process Event
					AddEventWriteFile(strFilePath, isCheckFileExt); // Add Event
					strTemp.Format("[����] %s: %s", (notification->isDir) ? "Dir" : "File", strFilePath);
					AddLogList(strTemp);
					nReturn = 100; // Backup!
				}
			}
		}
		break;
	case fltType_PreCleanup:
		if (!isCheckFileExt) break; // ���� Ȯ����
		strTemp.Format("[Cleanup] %s: %s", (notification->isDir) ? "Dir" : "File", strFilePath);
		AddLogList(strTemp);
		if (!notification->isDir) {
			md5_byte_t pathHashMD5[16];
			GenerateMD5(pathHashMD5, strFilePath); // MD5 ����
			if (CheckWriteFilePath(pathHashMD5) == true) {
				g_pParent->AddCheckRansomwareFile(m_pid, strFilePath);
			}
		}
		break;
	case fltType_PreWrite:
		if (!isCheckFileExt) break; // ���� Ȯ����
		strTemp.Format("[����] %s: %s", (notification->isDir) ? "Dir" : "File", strFilePath);
		AddLogList(strTemp);
		break;
	case fltType_PreSetInformation:
		strOrgFileName = (CString)m_pathInfoEx.szLastDriveLetter + (CString)(wchar_t*)notification->OrgFileName;
		nResult = g_pParent->GetPermissionDirectory(strOrgFileName);
		if (nResult == 3 || nPermission == 3) {
			nReturn = 1; // ���� ����
			break;
		}
		if (!notification->modeDelete) {
			isCheckFileExt = g_pParent->DoCheckFileExtension(strOrgFileName); // Ȯ���� Ȯ��(Org)
			if (!isCheckFileExt) {
				// ���� Ȯ����
				md5_byte_t dstHashMD5[16];
				GenerateMD5(dstHashMD5, strOrgFileName); // MD5 ����
				if (CheckRenameFilePath(dstHashMD5) == true)
					isCheckFileExt = true;
			}
			if (!isCheckFileExt) break; // ���� Ȯ����
			AddEventRenameFile(strOrgFileName, strFilePath); // Add Event
			strTemp.Format("[�̸� ����] ���� �� - %s: %s", (notification->isDir) ? "Dir" : "File", strOrgFileName);
			AddLogList(strTemp);
			strTemp.Format("[�̸� ����] ���� �� - %s: %s", (notification->isDir) ? "Dir" : "File", strFilePath);
			AddLogList(strTemp);
		}
		break;
	case fltType_PostSetInformation:
		if (!notification->modeDelete) break;
		if (nPermission == 3) {
			nReturn = 1; // ���� ����
			break;
		}
		if (!isCheckFileExt) break; // ���� Ȯ����
		AddEventDeleteFile(strFilePath, isCheckFileExt); // Add Event
		strTemp.Format("[����] %s: %s", (notification->isDir) ? "Dir" : "File", strFilePath);
		AddLogList(strTemp);
		if (isCheckFileExt) { // ��ȣ Ȯ����
			nReturn = 100; // Backup!
		}
		break;
	}

	return nReturn;
}

// ���μ��� ���� ����
bool ArProcessBehavior::RecoveryProcessBehavior()
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

	strTemp.Format("========== ���� ����: [%d] %s ==========", m_pid, m_strProcName);
	AddLogList(strTemp, true);

	strTemp.Format("create: %d / rename: %d / write: %d(%d) / delete: %d", m_cntCreate
		, m_cntRename
		, m_cntWrite
		, m_cntWrite_sp
		, m_cntDelete
	);
	AddLogList(strTemp, true);

	// pid�� �ش��ϴ� ���μ����� ������ �������� ����
	while (!m_stackEventRecord.empty()) {
		tmpPE = m_stackEventRecord.top();
		m_stackEventRecord.pop();
		switch (tmpPE.mode)
		{
		case 0: // ����, ���丮 ���� -> ����, ���丮 ����
			ritorINF = m_listNewFile.rbegin();
			while (ritorINF != m_listNewFile.rend())
			{
				if (tmpPE.numEvent == ritorINF->num) {
					AddLogList("Delete: " + ritorINF->strPath, true);
					if (ritorINF->isDirectory) {
						RemoveDirectory(ritorINF->strPath); // Directory
					}
					else {
						DeleteFile(ritorINF->strPath); // File
					}
					list<ITEM_NEW_FILE>::iterator itorINF = ritorINF.base();
					m_listNewFile.erase(--itorINF);
					break;
				}
				ritorINF++;
			}
			break;
		case 1: // �̸� ���� -> �̸� ����
			ritorIRF = m_listRenameFile.rbegin();
			while (ritorIRF != m_listRenameFile.rend())
			{
				if (tmpPE.numEvent == ritorIRF->num) {
					AddLogList("Rename: ���� �� " + ritorIRF->strDst, true);
					AddLogList("Rename: ���� �� " + ritorIRF->strSrc, true);
					MoveFileEx(ritorIRF->strDst, ritorIRF->strSrc, MOVEFILE_COPY_ALLOWED); // ���� �̸� ����
					list<ITEM_RENAME_FILE>::iterator itorIRF = ritorIRF.base();
					m_listRenameFile.erase(--itorIRF);
					break;
				}
				ritorIRF++;
			}
			break;
		case 2: // ���� ���� -> ���� ����
			ritorIWF = m_listWriteSpFile.rbegin();
			while (ritorIWF != m_listWriteSpFile.rend())
			{
				if (tmpPE.numEvent == ritorIWF->num) {
					if (ritorIWF->num_back != -1) {
						DoRecoveryFile(ritorIWF->num_back, ritorIWF->strPath); // ���� ����
					}
					else {
						AddLogList("Pass: " + ritorIWF->strPath, true); // Pass
					}
					list<ITEM_WRITE_FILE>::iterator itorIWF = ritorIWF.base();
					m_listWriteSpFile.erase(--itorIWF);
					break;
				}
				ritorIWF++;
			}
			break;
		case 3: // ���� ���� -> ���� ����
			ritorIDF = m_listDeleteFile.rbegin();
			while (ritorIDF != m_listDeleteFile.rend())
			{
				if (tmpPE.numEvent == ritorIDF->num) {
					if (ritorIDF->num_back != -1) {
						DoRecoveryFile(ritorIDF->num_back, ritorIDF->strPath); // ���� ����
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


bool ArProcessBehavior::AddEventNewFile(bool isDirectory, CString strPath)
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
	m_cntCreate++;

	tmpPE.mode = 0; // new
	tmpPE.numEvent = numEvent;
	m_stackEventRecord.push(tmpPE);

	return true;
}

// ����(���丮) �̸� ���� �̺�Ʈ �߰�
bool ArProcessBehavior::AddEventRenameFile(CString strSrc, CString strDst)
{
	PROCESS_EVENT tmpPE;
	ITEM_RENAME_FILE tmpIRF;
	unsigned int numEvent = m_numRenameFile++;

	// Rename File Event
	tmpIRF.num = numEvent;
	tmpIRF.strSrc = strSrc;
	tmpIRF.strDst = strDst;
	GenerateMD5(tmpIRF.dstHashMD5, strDst); // MD5 ����
	m_listRenameFile.push_back(tmpIRF);

	/*
	// MD5 Test
	int di;
	char szStrMD5[32 + 1] = { 0 };
	for (di = 0; di < 16; ++di)
		sprintf(szStrMD5 + di * 2, "%02x", tmpIRF.dstHashMD5[di]);
	g_pParent->AddLogList((CString)"[Rename] MD5 : " + strDst + " -> " + (CString)szStrMD5);
	*/

	// Process Event
	m_cntRename++;

	tmpPE.mode = 1; // rename
	tmpPE.numEvent = numEvent;
	m_stackEventRecord.push(tmpPE);

	return true;
}

// ����(���丮) �̸� ���� �̺�Ʈ �˻�(MD5)
bool ArProcessBehavior::CheckRenameFilePath(md5_byte_t *md5)
{
	list<ITEM_RENAME_FILE>::reverse_iterator ritorIRF; // rename
	ritorIRF = m_listRenameFile.rbegin();
	while (ritorIRF != m_listRenameFile.rend())
	{
		if (memcmp(ritorIRF->dstHashMD5, md5, 16) == 0)
			return true;
		ritorIRF++;
	}
	return false;
}

// ���� ���� �̺�Ʈ �߰�
bool ArProcessBehavior::AddEventWriteFile(CString strPath, bool isBackup)
{
	unsigned int tmpBackupNum = -1;
	PROCESS_EVENT tmpPE;
	ITEM_WRITE_FILE tmpIWF;
	list<ITEM_WRITE_FILE>::reverse_iterator ritorIWF; // write
	unsigned int numEvent = m_numWriteFile++;

	if (isBackup) {
		ritorIWF = m_listWriteFile.rbegin();
		while (ritorIWF != m_listWriteFile.rend())
		{
			if (strPath.Compare(ritorIWF->strPath) == 0) {
				tmpBackupNum = ritorIWF->num_back;
				ritorIWF->num_back = -1; // ���� ��� ��� ����(�ߺ� ���� ����)
				m_cntWrite--;
				break;
			}
			ritorIWF++;
		}

		// ��� ��� ���� ���
		if (tmpBackupNum == -1) {
			tmpBackupNum = AddEventBackupFile(strPath); // ��� �̺�Ʈ
		}
	}

	// Write File Event
	tmpIWF.num = numEvent;
	tmpIWF.num_back = tmpBackupNum;
	tmpIWF.strPath = strPath;
	GenerateMD5(tmpIWF.pathHashMD5, strPath); // MD5 ����
	m_listWriteFile.push_back(tmpIWF);

	// Process Event
	m_cntWrite++;

	tmpPE.mode = 2; // write
	tmpPE.numEvent = numEvent;
	m_stackEventRecord.push(tmpPE);

	return true;
}

// ���� ���� ���� �̺�Ʈ �߰�
bool ArProcessBehavior::AddEventWriteSpFile(CString strPath, bool isBackup)
{
	unsigned int tmpBackupNum = -1;
	PROCESS_EVENT tmpPE;
	ITEM_WRITE_FILE tmpIWF;
	list<ITEM_WRITE_FILE>::reverse_iterator ritorIWF; // write
	unsigned int numEvent = m_numWriteFile++;

	if (isBackup) {
		ritorIWF = m_listWriteSpFile.rbegin();
		while (ritorIWF != m_listWriteSpFile.rend())
		{
			if (strPath.Compare(ritorIWF->strPath) == 0) {
				tmpBackupNum = ritorIWF->num_back;
				ritorIWF->num_back = -1; // ���� ��� ��� ����(�ߺ� ���� ����)
				m_cntWrite_sp--;
				break;
			}
			ritorIWF++;
		}

		// ��� ��� ���� ���
		if (tmpBackupNum == -1) {
			tmpBackupNum = AddEventBackupFile(strPath); // ��� �̺�Ʈ
		}
	}

	// Write File Event
	tmpIWF.num = numEvent;
	tmpIWF.num_back = tmpBackupNum;
	tmpIWF.strPath = strPath;
	GenerateMD5(tmpIWF.pathHashMD5, strPath); // MD5 ����
	m_listWriteSpFile.push_back(tmpIWF);

	// Process Event
	m_cntWrite_sp++;

	tmpPE.mode = 2; // write
	tmpPE.numEvent = numEvent;
	m_stackEventRecord.push(tmpPE);

	return true;
}

// ���� ���� �̺�Ʈ �˻�(MD5)
bool ArProcessBehavior::CheckWriteFilePath(md5_byte_t *md5)
{
	list<ITEM_WRITE_FILE>::reverse_iterator ritorIWF; // write
	ritorIWF = m_listWriteFile.rbegin();
	while (ritorIWF != m_listWriteFile.rend())
	{
		if (memcmp(ritorIWF->pathHashMD5, md5, 16) == 0)
			return true;
		ritorIWF++;
	}
	return false;
}

// ���� ���� �̺�Ʈ �߰�
bool ArProcessBehavior::AddEventDeleteFile(CString strPath, bool isBackup)
{
	unsigned int tmpBackupNum = -1;
	PROCESS_EVENT tmpPE;
	ITEM_DELETE_FILE tmpIDF;
	list<ITEM_DELETE_FILE>::reverse_iterator ritorIDF; // delete
	unsigned int numEvent = m_numDeleteFile++;

	if (isBackup) {
		ritorIDF = m_listDeleteFile.rbegin();
		while (ritorIDF != m_listDeleteFile.rend())
		{
			if (strPath.Compare(ritorIDF->strPath) == 0) {
				tmpBackupNum = ritorIDF->num_back;
				ritorIDF->num_back = -1; // ���� ��� ��� ����(�ߺ� ���� ����)
				m_cntDelete--;
				break;
			}
			ritorIDF++;
		}
	}

	// ��� ��� ���� ���
	if (tmpBackupNum == -1) {
		tmpBackupNum = AddEventBackupFile(strPath); // ��� �̺�Ʈ
	}

	// Write File Event
	tmpIDF.num = numEvent;
	tmpIDF.num_back = tmpBackupNum;
	tmpIDF.strPath = strPath;
	m_listDeleteFile.push_back(tmpIDF);

	// Process Event
	m_cntDelete++;

	tmpPE.mode = 3; // delete
	tmpPE.numEvent = numEvent;
	m_stackEventRecord.push(tmpPE);

	return true;
}

// ���� ��� �̺�Ʈ �߰�
unsigned int ArProcessBehavior::AddEventBackupFile(CString strPath)
{
	bool result;
	ITEM_BACKUP_FILE tmpIBF;
	CString strBackupPath;
	unsigned int numEvent;

	numEvent = m_numBackupFile++;

	tmpIBF.num = numEvent;
	tmpIBF.strPath = g_pParent->GetBackupFilePath(strPath);
	m_listBackupFile.push_back(tmpIBF);

	return numEvent;
}


void ArProcessBehavior::AddLogList(CString msg, bool wTime)
{
	g_pParent->AddLogList(msg, wTime);
}


bool ArProcessBehavior::DoBackupFile(CString strPath, int type)
{
	int nResult;
	DWORD nFileSize;
	CString strNewDir;
	CString strNewPath;
	HANDLE hFile;

	nResult = g_pParent->GetPermissionDirectory(strPath);
	if (nResult > 1) {
		//AddLogList("��� ����: Safe Directory");
		return false;
	}

	if (type == 1) {
		hFile = CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			AddLogList("��� ����: INVALID_HANDLE_VALUE");
			return false;
		}

		nFileSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);

		// 10byte �̸� or 20MB �ʰ� ���� - ��� ����
		if (nFileSize < 10 || nFileSize > 20971520) {
			//AddLogList("��� ����: file size");
			return false;
		}
	}

	// ��� ��� ���ϱ�
	strNewPath = g_pParent->GetBackupFilePath(strPath);

	// ��� ���� ���丮 ����
	strNewDir.SetString(strNewPath);
	PathRemoveFileSpec((LPSTR)(LPCTSTR)strNewDir);
	CreateFolder(strNewDir);

	if (type == 0) {
		// �̴����Ϳ��� ���
	}
	else if (type == 1) {
		// ���� ��� (Overwrite ��)
		// ���� ��� - ����� ����
		AddLogList("Backup: " + strNewPath);
		nResult = CopyFile(strPath, strNewPath, TRUE);
	}

	return true;
}


bool ArProcessBehavior::DoRecoveryFile(unsigned int num_back, CString strPath)
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
	if (!result) {
		AddLogList("��� ����Ʈ ����", true);
		return false;
	}

	AddLogList("Recovery: " + strRecoveryPath, true);
	DeleteFile(strPath); // ���� ���� ����
	CopyFile(strRecoveryPath, strPath, FALSE); // ���� ����(�����)

	return true;
}

DWORD ArProcessBehavior::GetProcessInfo(int type)
{
	switch (type)
	{
	case PB_PROC_PID:
		return m_pid;
	case PB_PROC_PPID:
		return m_ppid;
	default:
		return 0;
	}
}

CString ArProcessBehavior::GetProcessName(int type)
{
	switch (type)
	{
	case PB_PROC_NAME:
		return m_strProcName;
	case PB_PROC_PATH:
		return m_strProcPath;
	default:
		return (CString)"";
	}
}

unsigned int ArProcessBehavior::GetCountBehavior(int type)
{
	switch (type)
	{
	case PB_COUNT_WRITE_SP:
		return m_cntWrite_sp;
	default:
		return 0;
	}
}


// Destructor
ArProcessBehavior::~ArProcessBehavior()
{
	// delete
}