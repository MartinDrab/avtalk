
#ifndef __MF_PLAY_H__
#define __MF_PLAY_H__

#include <cstdint>
#include <windows.h>
#include <objbase.h>
#include <mfidl.h>
#include <mfobjects.h>
#include "mfcap.h"



typedef struct _MFPLAY_DEVICE_INFO {
	wchar_t* Name;
	wchar_t* Description;
	wchar_t *EndpointId;
	DWORD State;
	DWORD Characteristics;
} MFPLAY_DEVICE_INFO, *PMFPLAY_DEVICE_INFO;

typedef struct _MFPLAY_DEVICE_STATE_MASK {
	union {
		struct {
			int Active : 1;
			int Disabled : 1;
			int NotPresent : 1;
			int Unplugged : 1;
		};
		int Value;
	};
} MFPLAY_DEVICE_STATE_MASK, *PMFPLAY_DEVICE_STATE_MASK;

typedef struct _MFPLAY_DEVICE {
	DWORD Characteristics;
	HWND Window;
	IMFMediaSink* Sink;
} MFPLAY_DEVICE, *PMFPLAY_DEVICE;



#ifdef __cplusplus
extern "C" {
#endif

HRESULT MFPlay_EnumDevices(MFPLAY_DEVICE_STATE_MASK StateMask, PMFPLAY_DEVICE_INFO* Devices, uint32_t* Count);
void MFPlay_FreeDeviceEnum(PMFPLAY_DEVICE_INFO Devices, uint32_t Count);
HRESULT MFPlay_EnumFormats(PMFPLAY_DEVICE Device, PMFGEN_FORMAT* Formats, DWORD* Count, DWORD* StreamCount);

HRESULT MFPlay_NewInstance(const MFPLAY_DEVICE_INFO * DeviceInfo, PMFPLAY_DEVICE * Device);
HRESULT MFPlay_NewInstanceForWindow(HWND Window, PMFPLAY_DEVICE* Device);
void MFPlay_FreeInstance(PMFPLAY_DEVICE Device);
HRESULT MFPlay_CreateStreamNodes(PMFPLAY_DEVICE Device, PMFGEN_STREAM_INFO * Nodes, DWORD * Count);

#ifdef __cplusplus
}
#endif


#endif
