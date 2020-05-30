
#ifndef __MF_PLAY_H__
#define __MF_PLAY_H__

#include <cstdint>
#include <windows.h>
#include <objbase.h>
#include <mfidl.h>
#include <mfobjects.h>



typedef struct _MFPLAY_DEVICE_INFO {
	wchar_t* Name;
	wchar_t* Description;
	wchar_t *EndpointId;
	DWORD State;
	DWORD Characteristics;
} MFPLAY_DEVICE_INFO, *PMFPLAY_DEVICE_INFO;


typedef struct _MFPLAY_DEVICE {
	IMFActivate* Object;
	IMFMediaSink* Sink;
} MFPLAY_DEVICE, *PMFPLAY_DEVICE;



HRESULT MFPlay_EnumDevices(PMFPLAY_DEVICE_INFO* Devices, uint32_t* Count);
void MFPlay_FreeDeviceEnum(PMFPLAY_DEVICE_INFO Devices, uint32_t Count);



#endif
