#ifndef __ATA_SMART_H__
#define __ATA_SMART_H__

#include <windows.h>

enum IO_CONTROL_CODE
{
        DFP_SEND_DRIVE_COMMAND	= 0x0007C084,
        DFP_RECEIVE_DRIVE_DATA	= 0x0007C088,
        IOCTL_SCSI_MINIPORT     = 0x0004D008,
        IOCTL_IDE_PASS_THROUGH  = 0x0004D028, // 2000 or later
        IOCTL_ATA_PASS_THROUGH  = 0x0004D02C, // XP SP2 and 2003 or later
};

typedef	struct _IDENTIFY_DEVICE_OUTDATA
{
        SENDCMDOUTPARAMS	SendCmdOutParam;
        BYTE				Data[IDENTIFY_BUFFER_SIZE - 1];
}  __attribute__((packed)) IDENTIFY_DEVICE_OUTDATA, *PIDENTIFY_DEVICE_OUTDATA;

typedef	struct _SMART_READ_DATA_OUTDATA
{
        SENDCMDOUTPARAMS	SendCmdOutParam;
        BYTE				Data[READ_ATTRIBUTE_BUFFER_SIZE - 1];
} __attribute__((packed)) SMART_READ_DATA_OUTDATA, *PSMART_READ_DATA_OUTDATA ;

typedef struct BIN_IDENTIFY_DEVICE
{
        BYTE		Bin[4096];
} BIN_IDENTIFY_DEVICE;

typedef struct NVME_IDENTIFY_DEVICE
{
        CHAR		Reserved1[4];
        CHAR		SerialNumber[20];
        CHAR		Model[40];
        CHAR		FirmwareRev[8];
        CHAR		Reserved2[9];
        CHAR		MinorVersion;
        SHORT		MajorVersion;
        CHAR		Reserved3[428];
        CHAR		Reserved4[3584];
} __attribute__((packed)) NVME_IDENTIFY_DEVICE;

typedef struct ATA_IDENTIFY_DEVICE
{
        WORD		GeneralConfiguration;					//0
        WORD		LogicalCylinders;						//1	Obsolete
        WORD		SpecificConfiguration;					//2
        WORD		LogicalHeads;							//3 Obsolete
        WORD		Retired1[2];							//4-5
        WORD		LogicalSectors;							//6 Obsolete
        DWORD		ReservedForCompactFlash;				//7-8
        WORD		Retired2;								//9
        CHAR		SerialNumber[20];						//10-19
        WORD		Retired3;								//20
        WORD		BufferSize;								//21 Obsolete
        WORD		Obsolute4;								//22
        CHAR		FirmwareRev[8];							//23-26
        CHAR		Model[40];								//27-46
        WORD		MaxNumPerInterupt;						//47
        WORD		Reserved1;								//48
        WORD		Capabilities1;							//49
        WORD		Capabilities2;							//50
        DWORD		Obsolute5;								//51-52
        WORD		Field88and7064;							//53
        WORD		Obsolute6[5];							//54-58
        WORD		MultSectorStuff;						//59
        DWORD		TotalAddressableSectors;				//60-61
        WORD		Obsolute7;								//62
        WORD		MultiWordDma;							//63
        WORD		PioMode;								//64
        WORD		MinMultiwordDmaCycleTime;				//65
        WORD		RecommendedMultiwordDmaCycleTime;		//66
        WORD		MinPioCycleTimewoFlowCtrl;				//67
        WORD		MinPioCycleTimeWithFlowCtrl;			//68
        WORD		Reserved2[6];							//69-74
        WORD		QueueDepth;								//75
        WORD		SerialAtaCapabilities;					//76
        WORD		SerialAtaAdditionalCapabilities;		//77
        WORD		SerialAtaFeaturesSupported;				//78
        WORD		SerialAtaFeaturesEnabled;				//79
        WORD		MajorVersion;							//80
        WORD		MinorVersion;							//81
        WORD		CommandSetSupported1;					//82
        WORD		CommandSetSupported2;					//83
        WORD		CommandSetSupported3;					//84
        WORD		CommandSetEnabled1;						//85
        WORD		CommandSetEnabled2;						//86
        WORD		CommandSetDefault;						//87
        WORD		UltraDmaMode;							//88
        WORD		TimeReqForSecurityErase;				//89
        WORD		TimeReqForEnhancedSecure;				//90
        WORD		CurrentPowerManagement;					//91
        WORD		MasterPasswordRevision;					//92
        WORD		HardwareResetResult;					//93
        WORD		AcoustricManagement;					//94
        WORD		StreamMinRequestSize;					//95
        WORD		StreamingTimeDma;						//96
        WORD		StreamingAccessLatency;					//97
        DWORD		StreamingPerformance;					//98-99
        ULONGLONG	MaxUserLba;								//100-103
        WORD		StremingTimePio;						//104
        WORD		Reserved3;								//105
        WORD		SectorSize;								//106
        WORD		InterSeekDelay;							//107
        WORD		IeeeOui;								//108
        WORD		UniqueId3;								//109
        WORD		UniqueId2;								//110
        WORD		UniqueId1;								//111
        WORD		Reserved4[4];							//112-115
        WORD		Reserved5;								//116
        DWORD		WordsPerLogicalSector;					//117-118
        WORD		Reserved6[8];							//119-126
        WORD		RemovableMediaStatus;					//127
        WORD		SecurityStatus;							//128
        WORD		VendorSpecific[31];						//129-159
        WORD		CfaPowerMode1;							//160
        WORD		ReservedForCompactFlashAssociation[7];	//161-167
        WORD		DeviceNominalFormFactor;				//168
        WORD		DataSetManagement;						//169
        WORD		AdditionalProductIdentifier[4];			//170-173
        WORD		Reserved7[2];							//174-175
        CHAR		CurrentMediaSerialNo[60];				//176-205
        WORD		SctCommandTransport;					//206
        WORD		ReservedForCeAta1[2];					//207-208
        WORD		AlignmentOfLogicalBlocks;				//209
        DWORD		WriteReadVerifySectorCountMode3;		//210-211
        DWORD		WriteReadVerifySectorCountMode2;		//212-213
        WORD		NvCacheCapabilities;					//214
        DWORD		NvCacheSizeLogicalBlocks;				//215-216
        WORD		NominalMediaRotationRate;				//217
        WORD		Reserved8;								//218
        WORD		NvCacheOptions1;						//219
        WORD		NvCacheOptions2;						//220
        WORD		Reserved9;								//221
        WORD		TransportMajorVersionNumber;			//222
        WORD		TransportMinorVersionNumber;			//223
        WORD		ReservedForCeAta2[10];					//224-233
        WORD		MinimumBlocksPerDownloadMicrocode;		//234
        WORD		MaximumBlocksPerDownloadMicrocode;		//235
        WORD		Reserved10[19];							//236-254
        WORD		IntegrityWord;							//255
} __attribute__((packed)) ATA_IDENTIFY_DEVICE;

typedef union IDENTIFY_DEVICE
{
        ATA_IDENTIFY_DEVICE	 A;
        NVME_IDENTIFY_DEVICE N;
        BIN_IDENTIFY_DEVICE	 B;
} IDENTIFY_DEVICE;

static const int ATA_FLAGS_DRDY_REQUIRED = 0x01;
static const int ATA_FLAGS_DATA_IN       = 0x02;
static const int ATA_FLAGS_DATA_OUT      = 0x04;
static const int ATA_FLAGS_48BIT_COMMAND = 0x08;

typedef struct _ATA_PASS_THROUGH_EX
{
        WORD    Length;
        WORD    AtaFlags;
        BYTE    PathId;
        BYTE    TargetId;
        BYTE    Lun;
        BYTE    ReservedAsUchar;
        DWORD   DataTransferLength;
        DWORD   TimeOutValue;
        DWORD   ReservedAsUlong;
        //	DWORD   DataBufferOffset;
#ifdef _WIN64
        DWORD	padding;
#endif
        DWORD_PTR   DataBufferOffset;
        IDEREGS PreviousTaskFile;
        IDEREGS CurrentTaskFile;
} __attribute__((packed)) ATA_PASS_THROUGH_EX, *PCMD_ATA_PASS_THROUGH_EX;

typedef struct
{
        ATA_PASS_THROUGH_EX Apt;
        DWORD Filer;
        BYTE  Buf[512];
} __attribute__((packed)) ATA_PASS_THROUGH_EX_WITH_BUFFERS;

void AtaSmartInit(void);
BOOL WakeUp(INT physicalDriveId);
BOOL DoIdentifyDevicePd(INT physicalDriveId, BYTE target, IDENTIFY_DEVICE *data);
BOOL SendAtaCommandPd(INT physicalDriveId, BYTE target, BYTE main, BYTE sub, BYTE param, PBYTE data, DWORD dataSize);
BOOL SendAtaCommand(INT phy_id, DWORD target, BYTE main, BYTE sub, BYTE param);
BOOL EnableApm(INT phy_id, DWORD target, BYTE param);
BOOL DisableApm(INT phy_id, DWORD target);
BYTE GetApmValue(IDENTIFY_DEVICE *i);
BYTE GetRecommandedApmValue(IDENTIFY_DEVICE *i);

#endif /* __ATA_SMART_H__ */
