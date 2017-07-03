
#include "stdafx.h"
#include "UtilityFilesys.h"


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
	if (pPos == NULL) {
		return FALSE;
	}
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

BOOL ConvertDevicePathToDrivePath(const wchar_t * pDevicePath, wchar_t *bufPath, size_t bufPathLen, PATH_INFO_EX *pPathInfo)
{
	wchar_t szDeviceName[128] = { 0 };
	wchar_t szDirPath[MAX_PATH] = { 0 };
	wchar_t szFileName[MAX_PATH] = { 0 };
	wchar_t szExt[32] = { 0 };
	wchar_t szDriveLetter[8] = { 0 };

	if (pDevicePath == NULL || bufPath == NULL || bufPathLen <= 0)
		return FALSE;

	//STEP 1 : device ��θ� ����̺� ��η� �����Ѵ�. 
	//ex : \Device\HarddiskVolume2\MyTest\123.txt --> C:\MyTest\123.txt

	//STEP 1 : ����̽� ��� ����� ����.. 
	if (splitDevicePath(pDevicePath, szDeviceName, 128, szDirPath, MAX_PATH, szFileName, MAX_PATH, szExt, 32) == FALSE)
		return FALSE;

	//SETP 2 : ����̽� ���� ����̺� ���ͷ� ����.. 
	//ex : \Device\HarddiskVolume2\ --> C:\

	// backup�� (�ӽ�)
	wcscpy(pPathInfo->szDeviceName, szDeviceName);
	
	if (GetDeviceNameToDriveLetter(szDeviceName, szDriveLetter, 8) == FALSE)
		return FALSE;

	// Reneme�� Drive Letter ���� (�ӽ�)
	memcpy(pPathInfo->szLastDriveLetter, szDriveLetter, 8);

	// ���� �˼����� ��� ������.. ex. DR
	if (wcslen(szDriveLetter) == 0 || wcslen(szDirPath) == 0)
		return FALSE;

	//STEP 3 : ��� ������.. 
	StringCchPrintfW(bufPath, bufPathLen, L"%s%s%s%s", szDriveLetter, szDirPath, szFileName, szExt);

	return TRUE;
}

void CreateFolder(CString p_strTargetRoot)
{
	/* ������ �ִ��� �����ϰ� ������ �����Ѵ� */
	if (_waccess((LPWSTR)(LPCTSTR)p_strTargetRoot, 0) != 0) {
		BOOL l_bExtractSucc = TRUE;

		int j = 0;
		CString l_strDirPath = _T("");
		CString l_strNowPath = _T("");
		while (l_bExtractSucc)
		{
			/* ���� �������� ������ ������ */
			l_bExtractSucc = AfxExtractSubString(l_strNowPath, p_strTargetRoot, j, '\\');
			l_strDirPath += l_strNowPath + _T("\\");
			if (l_bExtractSucc && _waccess((LPWSTR)(LPCTSTR)l_strDirPath, 0) != 0) {
				CreateDirectory(l_strDirPath, NULL);
			}
			j++;
		}
	}
}

bool RefreshDesktopDirectory()
{
	char szPath[1024];
	LPITEMIDLIST pItemIDList;
	SHGetSpecialFolderLocation(GetDesktopWindow(), CSIDL_DESKTOP, &pItemIDList);
	SHGetPathFromIDList(pItemIDList, szPath);

	// ����ȭ�� ���ΰ�ħ
	SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH | SHCNF_FLUSHNOWAIT, szPath, NULL);

	return true;
}