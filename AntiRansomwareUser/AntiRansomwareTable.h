#pragma once

typedef struct sItemNewFile { // ���� ���� �̺�Ʈ
	unsigned int num;
	bool isDirectory;
	CString strPath;
} ITEM_NEW_FILE;

typedef struct sItemWriteFile { // ���� ���� �̺�Ʈ
	unsigned int num;
	CString strPath;
	unsigned int num_back;
	md5_byte_t pathHashMD5[16];
} ITEM_WRITE_FILE;

typedef struct sItemRenameFile { // ���� �̸� ���� �̺�Ʈ
	unsigned int num;
	CString strSrc;
	CString strDst;
	md5_byte_t dstHashMD5[16];
} ITEM_RENAME_FILE;

typedef struct sItemDeleteFile { // ���� ���� �̺�Ʈ
	unsigned int num;
	bool isDirectory;
	CString strPath;
	unsigned int num_back;
} ITEM_DELETE_FILE;

typedef struct sItemBackupFile { // ���� ��� ���
	unsigned int num;
	CString strPath;
} ITEM_BACKUP_FILE;

typedef struct sItemCheckFile { // �˻� ��� ����
	DWORD pid;
	CString strPath;
	int nCheckCount;
} ITEM_CHECK_FILE;

typedef struct sItemPopupMessage { // �˾� �޽���
	DWORD pid;
	CString strProcName;
	CString strProcPath;
	CString strTitle;
	CString strMessage;
} ITEM_POPUP_MESSAGE;