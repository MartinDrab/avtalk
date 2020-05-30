
#ifndef __AVCAP_CAMERA_H__
#define __AVCAP_CAMERA_H__


#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>


typedef enum _EMFCapFormatType {
	mcftUnknown,
	mcftVideo,
	mcftAudio,
	mcftMax,
} EMFCapFormatType, * PEMFCapFormatType;

typedef struct _MFCAP_FORMAT {
	GUID TypeGuid;
	GUID SubtypeGuid;
	IMFMediaType* MediaType;
	UINT32 StreamIndex;
	UINT32 Index;
	BOOL Selected;
	wchar_t FriendlyName[8];
	EMFCapFormatType Type;
	union {
		struct {
			UINT32 Width;
			UINT32 Height;
			UINT32 BitRate;
			UINT32 Framerate;
		} Video;
		struct {
			UINT32 ChannelCount;
			UINT32 SamplesPerSecond;
			UINT32 BitsPerSample;
		} Audio;
	};
} MFCAP_FORMAT, * PMFCAP_FORMAT;

typedef struct _MFCAP_DEVICE_CHARACTERISTICS {
	union {
		struct {
			int Live: 1;
			int Seek : 1;
			int Pause : 1;
			int SLowSeek : 1;
			int MultiplePresentations : 1;
			int SkipForward : 1;
			int SKipBackward : 1;
			int NoNetwork : 1;
		};
		ULONG Value;
	};
} MFCAP_DEVICE_CHARACTERISTICS, *PMFCAP_DEVICE_CHARACTERISTICS;


struct _MFCAP_DEVICE;
typedef BOOL(MFCAP_SAMPLE_CALLBACK)(struct _MFCAP_DEVICE *Device, UINT32 StreamIndex, const void* Data, UINT32 Length, LONGLONG Timestamp, UINT32 Flags, void* Context);

typedef struct _MFCAP_DEVICE {
	IMFMediaSource* MediaSource;
	IMFSourceReader* MediaSourceReader;
	_MFCAP_DEVICE_CHARACTERISTICS Characteristics;
	EMFCapFormatType Type;
	DWORD StreamSelectionMask;
	HANDLE SamplingThread;
	volatile BOOL SamplingTerminated;
	MFCAP_SAMPLE_CALLBACK* SamplingCallback;
	PVOID SamplingContext;
} MFCAP_DEVICE, *PMFCAP_DEVICE;


typedef struct _MFCAP_DEVICE_ATTRIBUTES {
	// MF_LOW_LATENCY(0=FALSE)
	UINT32 LowLatencyDelay;
	// MF_READWRITE_DISABLE_CONVERTERS(TRUE)
	BOOL DisableConverters;
	// MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS(FALSE)
	BOOL HardwareTransforms;
	// MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING (FALSE)
	BOOL ExtraSoftwareProcessing;
} MFCAP_DEVICE_ATTRIBUTES, *PMFCAP_DEVICE_ATTRIBUTES;


typedef struct _MFCAP_DEVICE_INFO {
	wchar_t* FriendlyName;
	wchar_t* SymbolicLink;
	UINT32 Index;
	EMFCapFormatType Type;
} MFCAP_DEVICE_INFO, *PMFCAP_DEVICE_INFO;

#define MFCAP_CALLBACK_STREAMTICK			1
#define MFCAP_CALLBACK_ENDOFSTREAM			2
#define MFCAP_CALLBACK_ERROR				4


HRESULT MFCap_EnumDevices(PMFCAP_DEVICE_INFO *Devices, PUINT32 Count);
void MFCap_FreeDeviceEnumeration(PMFCAP_DEVICE_INFO Devices, UINT32 Count);
HRESULT MFCap_GetDeviceCount(EMFCapFormatType Type, UINT32* aCount);
HRESULT MFCap_GetProperties(EMFCapFormatType Type, UINT32 Index, GUID** Guids, PROPVARIANT** Values, UINT32* Count);
HRESULT MFCap_GetFormatProperties(MFCAP_FORMAT* Format, GUID** Guids, PROPVARIANT** Values, UINT32* Count);
void MFCap_FreeProperties(GUID* Guids, PROPVARIANT* Values, UINT32 Count);

HRESULT MFCap_EnumMediaTypes(PMFCAP_DEVICE Device, PMFCAP_FORMAT* Types, UINT32* Count, UINT32* StreamCount);
void MFCap_FreeMediaTypes(PMFCAP_FORMAT Formats, UINT32 Count);
HRESULT MFCap_NewInstance(EMFCapFormatType Type, UINT32 Index, PMFCAP_DEVICE_ATTRIBUTES Attributes, PMFCAP_DEVICE* aInstance);
void MFCap_FreeInstance(PMFCAP_DEVICE Instance);
HRESULT MFCap_SetFormat(PMFCAP_DEVICE Device, UINT32 Stream, IMFMediaType* Format);
HRESULT MFCap_SelectStream(PMFCAP_DEVICE Device, UINT32 StreamIndex, BOOL Select);
HRESULT MFCap_Start(PMFCAP_DEVICE Device, MFCAP_SAMPLE_CALLBACK* Callback, void* Context);
void MFCap_Stop(PMFCAP_DEVICE Device);
void MFCap_QueryStreamSelection(PMFCAP_DEVICE Device, PUINT32 StreamMask);
void MFCap_QueryCharacteristics(PMFCAP_DEVICE Device, PMFCAP_DEVICE_CHARACTERISTICS Characteristics);

HRESULT MFCap_StringFromGuid(GUID* Guid, PWCHAR* String);
void MFCap_StringFree(PWCHAR String);

HRESULT MFCap_Init(void);
void MFCap_Finit(void);



#endif