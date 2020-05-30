
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


typedef struct _MFPLAY_DEVICE {
	DWORD Characteristics;
	IMFMediaSink* Sink;
} MFPLAY_DEVICE, *PMFPLAY_DEVICE;



#ifdef __cplusplus
extern "C" {
#endif

HRESULT MFPlay_EnumDevices(PMFPLAY_DEVICE_INFO* Devices, uint32_t* Count);
void MFPlay_FreeDeviceEnum(PMFPLAY_DEVICE_INFO Devices, uint32_t Count);
HRESULT MFPlay_EnumFormats(PMFPLAY_DEVICE Device, PMFCAP_FORMAT* Formats, DWORD* Count, DWORD* StreamCount);

#ifdef __cplusplus
}
#endif


#endif
