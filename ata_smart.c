#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "ata_smart.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) do { { if(p) { (p)->Release(); (p)=NULL; } } } while (0)
#endif

#ifndef safeCloseHandle
#define safeCloseHandle(h) do { if( h != NULL ) { CloseHandle(h); h = NULL; } } while (0)
#endif

#ifndef safeVirtualFree
#define safeVirtualFree(h, b, c) do { if( h != NULL ) { VirtualFree(h, b, c); h = NULL; } } while (0)
#endif

#ifndef pr_dbg
#define pr_dbg printf
#endif

static BOOL IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
        OSVERSIONINFOEXW osvi = { 0 };
        DWORDLONG const dwlConditionMask = VerSetConditionMask(
                VerSetConditionMask(
                        VerSetConditionMask(
                                0, VER_MAJORVERSION, VER_GREATER_EQUAL),
                        VER_MINORVERSION, VER_GREATER_EQUAL),
                VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

        osvi.dwOSVersionInfoSize = sizeof(osvi);
        osvi.dwMajorVersion = wMajorVersion;
        osvi.dwMinorVersion = wMinorVersion;
        osvi.wServicePackMajor = wServicePackMajor;

        return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR,
                                  dwlConditionMask) != FALSE;
}

static int bAtaPassThrough = FALSE;
static int bAtaPassThroughSmart = FALSE;

void AtaSmartInit(void)
{
        bAtaPassThrough = FALSE;
        bAtaPassThroughSmart = FALSE;

        if (IsWindowsVersionOrGreater(10, 0, 0)/*m_Os.dwMajorVersion >= 10*/) {
                bAtaPassThrough = TRUE;
                bAtaPassThroughSmart = TRUE;
        } else if (
                IsWindowsVersionOrGreater(6, 0, 0) ||
                IsWindowsVersionOrGreater(5, 2, 0)
                //m_Os.dwMajorVersion >= 6 || (m_Os.dwMajorVersion == 5 && m_Os.dwMinorVersion == 2)
                ) {
                bAtaPassThrough = TRUE;
                bAtaPassThroughSmart = TRUE;
        } else if (
                IsWindowsVersionOrGreater(5, 1, 0)//m_Os.dwMajorVersion == 5 && m_Os.dwMinorVersion == 1
                ) {
                //CString cstr;
                //cstr = m_Os.szCSDVersion;
                //cstr.Replace(_T("Service Pack "), _T(""));
                if (IsWindowsVersionOrGreater(5, 1, 2)/*_tstoi(cstr) >= 2*/) {
                        bAtaPassThrough = TRUE;
                        bAtaPassThroughSmart = TRUE;
                }
        }
}

HANDLE GetIoCtrlHandle(BYTE index)
{
        char hdd_path[256] = { 0 };

        if (snprintf(hdd_path, sizeof(hdd_path), "\\\\.\\PhysicalDrive%d", index) == sizeof(hdd_path))
                return NULL;

        return CreateFile(hdd_path, GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL, OPEN_EXISTING, 0, NULL);
}

BOOL WakeUp(INT physicalDriveId)
{
        HANDLE hFile = INVALID_HANDLE_VALUE;
        char hdd_path[256] = { 0 };
        if(physicalDriveId < 0)
        {
                return FALSE;
        }

        if (snprintf(hdd_path, sizeof(hdd_path), "\\\\.\\PhysicalDrive%d", physicalDriveId) == sizeof(hdd_path))
                return FALSE;

        hFile = CreateFile(hdd_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if(hFile != INVALID_HANDLE_VALUE)
        {
                BYTE buf[512];
                const DWORD bufSize = 512;
                DWORD readSize = 0;
                SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
                ReadFile(hFile, buf, bufSize, &readSize, NULL);
                safeCloseHandle(hFile);
        }

        return TRUE;
}

BOOL SendAtaCommandPd(INT physicalDriveId, BYTE target, BYTE main, BYTE sub, BYTE param, PBYTE data, DWORD dataSize)
{
        BOOL bRet = FALSE;
        HANDLE hIoCtrl = NULL;
        DWORD dwReturned = 0;

        hIoCtrl = GetIoCtrlHandle(physicalDriveId);
        if (!hIoCtrl || hIoCtrl == INVALID_HANDLE_VALUE) {
                return FALSE;
        }

        if (bAtaPassThrough) {
                ATA_PASS_THROUGH_EX_WITH_BUFFERS ab;
                ZeroMemory(&ab, sizeof(ab));
                ab.Apt.Length = sizeof(ATA_PASS_THROUGH_EX);
                ab.Apt.TimeOutValue = 2;
                DWORD size = offsetof(ATA_PASS_THROUGH_EX_WITH_BUFFERS, Buf);
                ab.Apt.DataBufferOffset = size;

                if (dataSize > 0) {
                        if (dataSize > sizeof(ab.Buf)) {
                                return FALSE;
                        }
                        ab.Apt.AtaFlags = ATA_FLAGS_DATA_IN;
                        ab.Apt.DataTransferLength = dataSize;
                        ab.Buf[0] = 0xCF; // magic number
                        size += dataSize;
                }

                ab.Apt.CurrentTaskFile.bFeaturesReg = sub;
                ab.Apt.CurrentTaskFile.bSectorCountReg = param;
                ab.Apt.CurrentTaskFile.bDriveHeadReg = target;
                ab.Apt.CurrentTaskFile.bCommandReg = main;

                if (main == SMART_CMD) {
                        ab.Apt.CurrentTaskFile.bCylLowReg = SMART_CYL_LOW;
                        ab.Apt.CurrentTaskFile.bCylHighReg = SMART_CYL_HI;
                        ab.Apt.CurrentTaskFile.bSectorCountReg = 1;
                        ab.Apt.CurrentTaskFile.bSectorNumberReg = 1;
                }

                bRet = DeviceIoControl(hIoCtrl, IOCTL_ATA_PASS_THROUGH,
                                       &ab, size, &ab, size, &dwReturned, NULL);
                safeCloseHandle(hIoCtrl);
                if (bRet && dataSize && data != NULL) {
                        memcpy_s(data, dataSize, ab.Buf, dataSize);
                }

                return bRet;
        } else if (!IsWindowsVersionOrGreater(5, 0, 0)/*m_Os.dwMajorVersion <= 4*/) {
                return FALSE;
        }
//        } else {
//                DWORD size = sizeof(CMD_IDE_PATH_THROUGH) - 1 + dataSize;
//                CMD_IDE_PATH_THROUGH *buf = (CMD_IDE_PATH_THROUGH *) VirtualAlloc(NULL, size, MEM_COMMIT,
//                                                                                  PAGE_READWRITE);
//                if (buf != NULL) {
//                        buf->reg.bFeaturesReg = sub;
//                        buf->reg.bSectorCountReg = param;
//                        buf->reg.bSectorNumberReg = 0;
//                        buf->reg.bCylLowReg = 0;
//                        buf->reg.bCylHighReg = 0;
//                        buf->reg.bDriveHeadReg = target;
//                        buf->reg.bCommandReg = main;
//                        buf->reg.bReserved = 0;
//                        buf->length = dataSize;
//
//                        bRet = ::DeviceIoControl(hIoCtrl, IOCTL_IDE_PASS_THROUGH,
//                                                 buf, size, buf, size, &dwReturned, NULL);
//                }
//                safeCloseHandle(hIoCtrl);
//                if (bRet && dataSize && data != NULL) {
//                        memcpy_s(data, dataSize, buf->buffer, dataSize);
//                }
//                safeVirtualFree(buf, 0, MEM_RELEASE);
//        }

        return FALSE;
}

BOOL DoIdentifyDevicePd(INT physicalDriveId, BYTE target, IDENTIFY_DEVICE *data)
{
        BOOL bRet = FALSE;
        HANDLE hIoCtrl = NULL;
        DWORD dwReturned = 0;
        int passthrough_sent = 0;

        IDENTIFY_DEVICE_OUTDATA sendCmdOutParam;
        SENDCMDINPARAMS sendCmd;

        if (data == NULL) {
                return FALSE;
        }

        memset(data, 0x00, sizeof(*data));

        if (bAtaPassThrough && bAtaPassThroughSmart) {
//                pr_dbg("SendAtaCommandPd - IDENTIFY_DEVICE (ATA_PASS_THROUGH)");
                bRet = SendAtaCommandPd(physicalDriveId, target, 0xEC, 0x00, 0x00, (PBYTE) data,
                                        sizeof(ATA_IDENTIFY_DEVICE));
                passthrough_sent = 1;
        }

        if (bRet == FALSE || !passthrough_sent) {
                ZeroMemory(data, sizeof(ATA_IDENTIFY_DEVICE));
                hIoCtrl = GetIoCtrlHandle(physicalDriveId);
                if (!hIoCtrl || hIoCtrl == INVALID_HANDLE_VALUE) {
                        return FALSE;
                }
                ZeroMemory(&sendCmdOutParam, sizeof(IDENTIFY_DEVICE_OUTDATA));
                ZeroMemory(&sendCmd, sizeof(SENDCMDINPARAMS));

                sendCmd.irDriveRegs.bCommandReg = ID_CMD;
                sendCmd.irDriveRegs.bSectorCountReg = 1;
                sendCmd.irDriveRegs.bSectorNumberReg = 1;
                sendCmd.irDriveRegs.bDriveHeadReg = target;
                sendCmd.cBufferSize = IDENTIFY_BUFFER_SIZE;

//                pr_dbg("SendAtaCommandPd - IDENTIFY_DEVICE");
                bRet = DeviceIoControl(hIoCtrl, DFP_RECEIVE_DRIVE_DATA,
                                       &sendCmd, sizeof(SENDCMDINPARAMS),
                                       &sendCmdOutParam, sizeof(IDENTIFY_DEVICE_OUTDATA),
                                       &dwReturned, NULL);

                safeCloseHandle(hIoCtrl);

                if (bRet == FALSE || dwReturned != sizeof(IDENTIFY_DEVICE_OUTDATA)) {
                        return FALSE;
                }

                memcpy_s(data, sizeof(ATA_IDENTIFY_DEVICE), sendCmdOutParam.SendCmdOutParam.bBuffer,
                         sizeof(ATA_IDENTIFY_DEVICE));
        }

        return TRUE;
}

BOOL SendAtaCommand(INT phy_id, DWORD target, BYTE main, BYTE sub, BYTE param)
{
        WakeUp(phy_id);

        return SendAtaCommandPd(phy_id, target, main, sub, param, NULL, 0);
}

BOOL EnableApm(INT phy_id, DWORD target, BYTE param)
{
        return SendAtaCommand(phy_id, target, 0xEF, 0x05, param);
}

BOOL DisableApm(INT phy_id, DWORD target)
{
        return SendAtaCommand(phy_id, target, 0xEF, 0x85, 0);
}

BYTE GetApmValue(IDENTIFY_DEVICE *i)
{
        return LOBYTE(i->A.CurrentPowerManagement);
}

BYTE GetRecommandedApmValue(IDENTIFY_DEVICE *i)
{
        return HIBYTE(i->A.CurrentPowerManagement);
}
