#pragma once

typedef struct sPathInfoEx { // ��� ���� ����
	wchar_t szLastDriveLetter[8];	// Rename�� Driver Letter (�ӽ�)
	wchar_t szDeviceName[128];	// Backup�� Device Name (�ӽ�)
} PATH_INFO_EX;

BOOL splitDevicePath(const wchar_t * path,
	wchar_t * devicename, size_t lenDevicename,
	wchar_t * dir, size_t lenDir,
	wchar_t * fname, size_t lenFname,
	wchar_t * ext, size_t lenExt);
BOOL GetDeviceNameToDriveLetter(const wchar_t * pDevicePath,wchar_t * bufDriveLetter, size_t bufLen);
BOOL ConvertDevicePathToDrivePath(const wchar_t * pDevicePath, wchar_t *bufPath, size_t bufPathLen, PATH_INFO_EX *pPathInfo);
void CreateFolder(CString p_strTargetRoot);
bool RefreshDesktopDirectory();
bool GenerateMD5(md5_byte_t *md5_out, CString strText);