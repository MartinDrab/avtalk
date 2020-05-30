
#include <vector>
#include <windows.h>
#include <initguid.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <ks.h>

#include <shlwapi.h>
#include <objbase.h>
#include <dshow.h>
#include <math.h>
#include "mfcap.h"

#pragma comment(lib, "strmiids")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Propsys.lib")



HRESULT MFCap_EnumMediaTypes(PMFCAP_DEVICE Device, PMFCAP_FORMAT *Types, UINT32 *Count, UINT32 *StreamCount)
{
	HRESULT hr = S_OK;
	UINT32 index = 0;
	UINT32 streamIndex = 0;
	MFCAP_FORMAT format;
	IMFMediaType* nativeType = NULL;
	std::vector<MFCAP_FORMAT> types;
	PMFCAP_FORMAT tmpFormats = NULL;
	GUID formatGuids[] = {
		MFVideoFormat_NV12,
		MFVideoFormat_YUY2,
		MFVideoFormat_YV12,
		MFVideoFormat_YVU9,
		MFVideoFormat_MJPG,
		MFAudioFormat_PCM,
		MFAudioFormat_Float,
	};
	const wchar_t* formatNames[] = {
		L"NV12",
		L"YUV2",
		L"YV12",
		L"YVU9",
		L"MJPG",
		L"PCM",
		L"Float"
	};

	do {
		hr = S_OK;
		index = 0;
		while (hr == S_OK) {
			memset(&format, 0, sizeof(format));
			hr = Device->MediaSourceReader->GetNativeMediaType(streamIndex, index, &nativeType);
			if (FAILED(hr))
				break;

			hr = nativeType->GetGUID(MF_MT_MAJOR_TYPE, &format.TypeGuid);
			if (FAILED(hr))
				break;

			hr = nativeType->GetGUID(MF_MT_SUBTYPE, &format.SubtypeGuid);
			if (FAILED(hr))
				break;

			wcscpy(format.FriendlyName, L"UNKNOWN");
			for (size_t i = 0; i < sizeof(formatGuids) / sizeof(formatGuids[0]); ++i) {
				if (formatGuids[i] == format.SubtypeGuid) {
					wcscpy(format.FriendlyName, formatNames[i]);
					break;
				}
			}
			
			format.StreamIndex = streamIndex;
			format.Selected = (Device->StreamSelectionMask & (1 << format.StreamIndex)) != 0;
			format.Index = index;
			if (format.TypeGuid == MFMediaType_Video) {
				format.Type = mcftVideo;
				hr = MFGetAttributeSize(nativeType, MF_MT_FRAME_SIZE, &format.Video.Width, &format.Video.Height);
				if (FAILED(hr))
					break;

				nativeType->GetUINT32(MF_MT_AVG_BITRATE, &format.Video.BitRate);
				nativeType->GetUINT32(MF_MT_FRAME_RATE, &format.Video.Framerate);
			} else if (format.TypeGuid == MFMediaType_Audio) {
				format.Type = mcftAudio;
				nativeType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &format.Audio.BitsPerSample);
				nativeType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &format.Audio.ChannelCount);
				nativeType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &format.Audio.SamplesPerSecond);
			} else format.Type = mcftUnknown;

			format.MediaType = nativeType;
			types.push_back(format);
			index++;
			nativeType = NULL;
		}

		if (FAILED(hr) && hr != MF_E_NO_MORE_TYPES)
			break;

		++streamIndex;
	} while (hr == MF_E_NO_MORE_TYPES);

	if (hr == MF_E_INVALIDSTREAMNUMBER)
		hr = S_OK;

	if (SUCCEEDED(hr)) {
		*StreamCount = streamIndex;
		*Count = (UINT32)types.size();
		*Types = NULL;
		if (types.size() > 0) {
			tmpFormats = (PMFCAP_FORMAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, types.size()*sizeof(tmpFormats[0]));
			if (tmpFormats == NULL)
				hr = E_OUTOFMEMORY;

			if (SUCCEEDED(hr)) {
				for (UINT32 i = 0; i < types.size(); ++i) {
					tmpFormats[i] = types[i];
					tmpFormats[i].MediaType->AddRef();
				}

				*Types = tmpFormats;
			}
		}
	}

	for (auto& mt : types)
		mt.MediaType->Release();

	types.clear();
	if (nativeType != NULL)
		nativeType->Release();

	return hr;
}


void MFCap_FreeMediaTypes(PMFCAP_FORMAT Formats, UINT32 Count)
{
	if (Count > 0) {
		for (UINT32 i = 0; i < Count; ++i)
			Formats[i].MediaType->Release();

		HeapFree(GetProcessHeap(), 0, Formats);
	}

	return;
}


HRESULT MFCap_SelectStream(PMFCAP_DEVICE Device, UINT32 StreamIndex, BOOL Select)
{
	HRESULT hr = S_OK;
	UINT32 bitValue = 0;

	bitValue = (1 << StreamIndex);
	if (Select)
		Device->StreamSelectionMask |= bitValue;
	else Device->StreamSelectionMask &= ~bitValue;

	return hr;
}


static DWORD WINAPI _SamplingThreadRoutine(PVOID Context)
{
	HRESULT ret = S_OK;
	PMFCAP_DEVICE device = (PMFCAP_DEVICE)Context;
	LONGLONG timeStamp = 0;
	DWORD streamIndex = 0;
	IMFSample* sample = NULL;
	DWORD streamFlags = 0;
	UINT32 callbackFlags = 0;
	IMFMediaBuffer* buffer = NULL;
	void* data = NULL;
	DWORD len = 0;

	while (SUCCEEDED(ret) && !device->SamplingTerminated) {
		timeStamp = 0;
		streamIndex = 0;
		streamFlags = 0;
		callbackFlags = 0;
		sample = NULL;
		data = NULL;
		len = 0;
		ret = device->MediaSourceReader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, &streamIndex, &streamFlags, &timeStamp, &sample);
		if (SUCCEEDED(ret)) {
			if (streamFlags & MF_SOURCE_READERF_ERROR)
				callbackFlags |= MFCAP_CALLBACK_ERROR;
		
			if (streamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
				callbackFlags |= MFCAP_CALLBACK_ENDOFSTREAM;
			
			if (streamFlags & MF_SOURCE_READERF_STREAMTICK)
				callbackFlags |= MFCAP_CALLBACK_STREAMTICK;

			if (sample != NULL) {
				ret = sample->ConvertToContiguousBuffer(&buffer);
				if (SUCCEEDED(ret))
					ret = buffer->Lock((PBYTE *)&data, NULL, &len);
			}

			if (SUCCEEDED(ret))
				device->SamplingCallback(device, streamIndex, data, len, timeStamp, callbackFlags, device->SamplingContext);
			
			if (buffer != NULL) {
				if (data != NULL)
					buffer->Unlock();

				buffer->Release();
			}
		} else {
			callbackFlags = MFCAP_CALLBACK_ERROR;
			device->SamplingCallback(device, streamIndex, data, len, timeStamp, callbackFlags, device->SamplingContext);
		}
	}

	return ret;
}



#define __HRESULT_FROM_WIN32(x) ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) : ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)))


HRESULT MFCap_Start(PMFCAP_DEVICE Device, MFCAP_SAMPLE_CALLBACK* Callback, void* Context)
{
	HRESULT ret = S_OK;

	Device->SamplingCallback = Callback;
	Device->SamplingContext = Context;
	for (UINT32 i = 0; i < 31; ++i) {
		if ((Device->StreamSelectionMask & (1 << i)) != 0)
			ret = Device->MediaSourceReader->SetStreamSelection(i, TRUE);
		
		if (FAILED(ret)) {
			Device->MediaSourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
			Device->MediaSourceReader->Flush(MF_SOURCE_READER_ALL_STREAMS);
			break;
		}
	}

	if (SUCCEEDED(ret)) {
		Device->SamplingThread = CreateThread(NULL, 0, _SamplingThreadRoutine, Device, 0, NULL);
		if (Device->SamplingThread == NULL) {
			ret = __HRESULT_FROM_WIN32(GetLastError());
			Device->MediaSourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
			Device->MediaSourceReader->Flush(MF_SOURCE_READER_ALL_STREAMS);
		}
	}

	return ret;
}


void MFCap_Stop(PMFCAP_DEVICE Device)
{
	Device->MediaSourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
	Device->SamplingTerminated = TRUE;
	Device->MediaSourceReader->Flush(MF_SOURCE_READER_ALL_STREAMS);
	WaitForSingleObject(Device->SamplingThread, INFINITE);
	CloseHandle(Device->SamplingThread);
	Device->SamplingThread = NULL;
	Device->SamplingTerminated = FALSE;

	return;
}


void MFCap_QueryStreamSelection(PMFCAP_DEVICE Device, PUINT32 StreamMask)
{
	*StreamMask = Device->StreamSelectionMask;

	return;
}


void MFCap_QueryCharacteristics(PMFCAP_DEVICE Device, PMFCAP_DEVICE_CHARACTERISTICS Characteristics)
{
	*Characteristics = Device->Characteristics;

	return;
}


HRESULT MFCap_EnumDevices(EMFCapFormatType Type, PMFCAP_DEVICE_INFO* Devices, PUINT32 Count)
{
	HRESULT ret = S_OK;
	std::vector<MFCAP_DEVICE_INFO> ds;
	EMFCapFormatType deviceTypes[] = {
		mcftVideo,
		mcftAudio,
	};
	EMFCapFormatType dt = mcftUnknown;
	UINT32 deviceCount = 0;
	GUID captureGuid;
	IMFAttributes* attributes = NULL;
	IMFActivate** devices = NULL;
	GUID linkGuid;
	UINT32 len = 0;
	MFCAP_DEVICE_INFO info;
	PMFCAP_DEVICE_INFO tmpDevices = NULL;

	for (UINT32 i = 0; i < sizeof(deviceTypes) / sizeof(deviceTypes[0]); ++i) {
		dt = deviceTypes[i];
		if (Type != mcftUnknown && dt != Type)
			continue;

		switch (dt) {
			case mcftVideo:
				captureGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;
				linkGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK;
				break;
			case mcftAudio:
				captureGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID;
				linkGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK;
				break;
			default:
				ret = E_INVALIDARG;
				break;
		}

		if (SUCCEEDED(ret))
			ret = MFCreateAttributes(&attributes, 1);

		if (SUCCEEDED(ret))
			ret = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, captureGuid);

		if (SUCCEEDED(ret))
			ret = MFEnumDeviceSources(attributes, &devices, &deviceCount);

		if (SUCCEEDED(ret)) {
			for (UINT32 i = 0; i < deviceCount; ++i) {
				memset(&info, 0, sizeof(info));
				info.Type = dt;
				info.Index = i;
				ret = devices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &info.FriendlyName, NULL);
				if (FAILED(ret))
					break;

				ret = devices[i]->GetAllocatedString(linkGuid, &info.SymbolicLink, NULL);
				if (FAILED(ret)) {
					CoTaskMemFree(info.FriendlyName);
				}

				ds.push_back(info);
			}

			for (UINT32 i = 0; i < deviceCount; ++i)
				devices[i]->Release();

			CoTaskMemFree(devices);
		}

		if (attributes != NULL)
			attributes->Release();

		if (FAILED(ret))
			break;
	}

	if (SUCCEEDED(ret)) {
		*Devices = NULL;
		*Count = NULL;
		if (ds.size() > 0) {
			tmpDevices = (PMFCAP_DEVICE_INFO)CoTaskMemAlloc(ds.size()*sizeof(tmpDevices[0]));
			if (tmpDevices == NULL)
				ret = E_OUTOFMEMORY;

			if (SUCCEEDED(ret)) {
				for (auto i = 0; i < ds.size(); ++i)
					tmpDevices[i] = ds[i];

				*Count = (UINT32)ds.size();
				*Devices = tmpDevices;
			}
		}
	}

	if (FAILED(ret)) {
		for (auto& x : ds) {
			CoTaskMemFree(x.SymbolicLink);
			CoTaskMemFree(x.FriendlyName);
		}
	}

	ds.clear();

	return ret;
}


void MFCap_FreeDeviceEnumeration(PMFCAP_DEVICE_INFO Devices, UINT32 Count)
{
	if (Count > 0) {
		for (UINT32 i = 0; i < Count; ++i) {
			CoTaskMemFree(Devices[i].SymbolicLink);
			CoTaskMemFree(Devices[i].FriendlyName);
		}

		CoTaskMemFree(Devices);
	}

	return;
}


HRESULT MFCap_GetDeviceCount(EMFCapFormatType Type, UINT32 *aCount)
{
	HRESULT hr = S_OK;
	IMFAttributes *attributes = NULL;
	IMFActivate** devices = NULL;
	UINT32 deviceCount = 0;
	GUID captureGuid;

	switch (Type) {
		case mcftVideo:
			captureGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;
			break;
		case mcftAudio:
			captureGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID;
			break;
		default:
			hr = E_INVALIDARG;
			break;
	}

	if (SUCCEEDED(hr))
		hr = MFCreateAttributes(&attributes, 1);
	
	if (SUCCEEDED(hr))
		hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, captureGuid);

	if (SUCCEEDED(hr))
		hr = MFEnumDeviceSources(attributes, &devices, &deviceCount);

	if (SUCCEEDED(hr)) {
		for (UINT32 i = 0; i < deviceCount; ++i)
			devices[i]->Release();

		CoTaskMemFree(devices);
		*aCount = deviceCount;
	}

	if (attributes != NULL)
		attributes->Release();

	return hr;
}


HRESULT MFCap_NewInstance(EMFCapFormatType Type, UINT32 Index, PMFCAP_DEVICE_ATTRIBUTES Attributes, PMFCAP_DEVICE* aInstance)
{
	HRESULT hr = S_OK;
	UINT32 deviceCount = 0;
	IMFActivate** devices = NULL;
	IMFAttributes* attributes = NULL;
	IMFActivate* d = NULL;
	IMFMediaSource* ms = NULL;
	PMFCAP_DEVICE  cam = NULL;
	GUID captureGuid;

	switch (Type) {
		case mcftVideo:
			captureGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;
			break;
		case mcftAudio:
			captureGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID;
			break;
		default:
			hr = E_INVALIDARG;
			break;
	}

	if (SUCCEEDED(hr))
		hr = MFCreateAttributes(&attributes, 1);
	
	if (SUCCEEDED(hr))
		hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, captureGuid);

	if (SUCCEEDED(hr))
		hr = MFEnumDeviceSources(attributes, &devices, &deviceCount);

	if (SUCCEEDED(hr)) {
		d = devices[Index];
		d->AddRef();
		for (UINT32 i = 0; i < deviceCount; ++i)
			devices[i]->Release();

		CoTaskMemFree(devices);
		hr = d->ActivateObject(__uuidof(IMFMediaSource), (void**)&ms);
		if (SUCCEEDED(hr)) {
			cam = (PMFCAP_DEVICE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MFCAP_DEVICE));
			if (cam == NULL)
				hr = E_OUTOFMEMORY;

			if (SUCCEEDED(hr)) {
				IMFAttributes *msAttributes = NULL;
				
				cam->Type = Type;
				cam->MediaSource = ms;
				cam->StreamSelectionMask = 0xffffffff;
				if (Attributes != NULL) {
					hr = MFCreateAttributes(&msAttributes, 4);
					if (SUCCEEDED(hr))
						hr = msAttributes->SetUINT32(MF_LOW_LATENCY, Attributes->LowLatencyDelay);
					
					if (SUCCEEDED(hr))
						hr = msAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, Attributes->DisableConverters);
					
					if (SUCCEEDED(hr))
						hr = msAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, Attributes->HardwareTransforms);
					
					if (SUCCEEDED(hr))
						hr = msAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, Attributes->ExtraSoftwareProcessing);
				}

				if (SUCCEEDED(hr))
					hr = ms->GetCharacteristics(&cam->Characteristics.Value);
				
				if (SUCCEEDED(hr))
					hr = MFCreateSourceReaderFromMediaSource(cam->MediaSource, msAttributes, &cam->MediaSourceReader);			
			
				if (SUCCEEDED(hr))
					hr = cam->MediaSourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
				
				if (SUCCEEDED(hr))
					hr = cam->MediaSourceReader->Flush(MF_SOURCE_READER_ALL_STREAMS);

				if (msAttributes != NULL)
					msAttributes->Release();
			}
		}

		if (SUCCEEDED(hr)) {
			ms->AddRef();
			*aInstance = cam;
			cam = NULL;
		}

		if (cam != NULL)
			HeapFree(GetProcessHeap(), 0, cam);

		if (ms != NULL)
			ms->Release();
	}

	if (attributes != NULL)
		attributes->Release();

	return hr;
}


void MFCap_FreeInstance(PMFCAP_DEVICE Instance)
{
	Instance->MediaSourceReader->Release();
	Instance->MediaSource->Release();
	HeapFree(GetProcessHeap(), 0, Instance);

	return;
}


static HRESULT _GetProperties(IMFAttributes* Attributes, GUID** Guids, PROPVARIANT** Values, UINT32* Count)
{
	HRESULT hr = S_OK;
	UINT32 tmpCount = 0;
	PROPVARIANT* tmpValues = NULL;
	GUID* tmpTypes = NULL;

	hr = Attributes->GetCount(&tmpCount);
	if (SUCCEEDED(hr)) {
		*Count = tmpCount;
		*Guids = NULL;
		*Values = NULL;
		if (tmpCount > 0) {
			tmpTypes = (GUID*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, tmpCount * (sizeof(tmpTypes[0]) + sizeof(tmpValues[0])));
			if (tmpTypes == NULL)
				hr = E_OUTOFMEMORY;

			if (SUCCEEDED(hr)) {
				tmpValues = (PROPVARIANT*)(tmpTypes + tmpCount);
				for (UINT32 i = 0; i < tmpCount; ++i) {
					hr = Attributes->GetItemByIndex(i, tmpTypes + i, tmpValues + i);
					if (FAILED(hr)) {
						for (UINT32 j = 0; j < i; ++j)
							PropVariantClear(tmpValues + j);

						break;
					}
				}

				if (SUCCEEDED(hr)) {
					*Guids = tmpTypes;
					*Values = tmpValues;
				}

				if (FAILED(hr))
					HeapFree(GetProcessHeap(), 0, tmpTypes);
			}
		}
	}

	return hr;
}


HRESULT MFCap_GetProperties(EMFCapFormatType Type, UINT32 Index, GUID** Guids, PROPVARIANT** Values, UINT32* Count)
{
	HRESULT hr = S_OK;
	UINT32 deviceCount = 0;
	IMFActivate** devices = NULL;
	IMFAttributes* attributes = NULL;
	IMFActivate* d = NULL;
	GUID captureGuid;

	switch (Type) {
		case mcftVideo:
			captureGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;
			break;
		case mcftAudio:
			captureGuid = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID;
			break;
		default:
			hr = E_INVALIDARG;
			break;
	}

	if (SUCCEEDED(hr))
		hr = MFCreateAttributes(&attributes, 1);

	if (SUCCEEDED(hr))
		hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, captureGuid);

	if (SUCCEEDED(hr))
		hr = MFEnumDeviceSources(attributes, &devices, &deviceCount);

	if (SUCCEEDED(hr)) {
		d = devices[Index];
		d->AddRef();
		for (UINT32 i = 0; i < deviceCount; ++i)
			devices[i]->Release();

		CoTaskMemFree(devices);
	}

	if (SUCCEEDED(hr))
		hr = _GetProperties(d, Guids, Values, Count);

	if (d != NULL)
		d->Release();

	if (attributes != NULL)
		attributes->Release();

	return hr;
}


HRESULT MFCap_GetFormatProperties(MFCAP_FORMAT *Format, GUID** Guids, PROPVARIANT** Values, UINT32* Count)
{
	HRESULT hr = S_OK;

	hr = _GetProperties(Format->MediaType, Guids, Values, Count);

	return hr;
}

void MFCap_FreeProperties(GUID *Guids, PROPVARIANT* Values, UINT32 Count)
{
	if (Count > 0) {
		for (UINT32 i = 0; i < Count; ++i)
			PropVariantClear(Values + i);

		HeapFree(GetProcessHeap(), 0, Guids);
	}

	return;
}


HRESULT MFCap_SetFormat(PMFCAP_DEVICE Device, UINT32 Stream, IMFMediaType *Format)
{
	HRESULT hr = S_OK;

	hr = Device->MediaSourceReader->SetCurrentMediaType(Stream, NULL, Format);

	return hr;
}



typedef struct _GUID_STRING {
	GUID GUid;
	const wchar_t* String;
} GUID_STRING, *PGUID_STRING;


static GUID_STRING _guidMap [] = {
	// General
	{MF_MT_MAJOR_TYPE, L"Major type"},
	{MF_MT_SUBTYPE, L"Subtype"},
	// Video formats
	{MFVideoFormat_NV12, L"NV12"},
	{MEDIASUBTYPE_YUY2, L"YUY2"},
	{MEDIASUBTYPE_YV12, L"YV12"},
	{MEDIASUBTYPE_YVU9, L"YVU9"},
	{MEDIASUBTYPE_MJPG, L"MJPG"},
	// Video attributes
	{MF_MT_FRAME_SIZE, L"Frame size"},
	{MF_MT_FRAME_RATE, L"Frame rate"},
	{MF_MT_PIXEL_ASPECT_RATIO, L"Pixel aspect ratio"},
	{MF_MT_DRM_FLAGS, L"DRM flags"},
	{MF_MT_TIMESTAMP_CAN_BE_DTS, L"DTS timestamp"},
	{MF_MT_PAD_CONTROL_FLAGS, L"PAD flags"},
	{MF_MT_SOURCE_CONTENT_HINT, L"Content hint"},
	{MF_MT_VIDEO_CHROMA_SITING, L"Chroma settings"},
	{MF_MT_INTERLACE_MODE, L"Interlace mode"},
	{MF_MT_TRANSFER_FUNCTION, L"Transfer function"},
	{MF_MT_VIDEO_PRIMARIES, L"Primaries"},
	{MF_MT_AVG_BITRATE, L"Avg bitrate"},
	{MF_MT_AVG_BIT_ERROR_RATE, L"Bit error rate"},
	{},
	// Audio parameters
	{MF_MT_AUDIO_AVG_BYTES_PER_SECOND, L"Avg bytes per second"},
	{MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND, L"Float samples per second"},
	{MF_MT_AUDIO_SAMPLES_PER_SECOND, L"Samples per second"},
	{MF_MT_AUDIO_NUM_CHANNELS, L"Channels"},
	{MF_MT_AUDIO_BLOCK_ALIGNMENT, L"Block align"},
	{MF_MT_AUDIO_BITS_PER_SAMPLE, L"Bits per sample"},
	{MF_MT_AUDIO_VALID_BITS_PER_SAMPLE, L"Valid bits per sample"},
	{MF_MT_AUDIO_SAMPLES_PER_BLOCK, L"Samples per block"},
	{MF_MT_AUDIO_CHANNEL_MASK, L"Channel mask"},
	// Audio formats
	{MFAudioFormat_Float, L"Float"},
	{MFAudioFormat_PCM, L"PCM"},
	// Audio device attributes
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID, L"Endpoint ID"},
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ROLE, L"Role"},
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, L"Source type"},
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_HW_SOURCE, L"HW source"},
	{MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, L"Friendly name"},
	{MF_DEVSOURCE_ATTRIBUTE_MEDIA_TYPE, L"Media type"},
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, L"Link"},
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK, L"Link"},
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_MAX_BUFFERS, L"Max buffers"},
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID, L"AudioCap"},
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID, L"VideoCap"},
	{MEDIATYPE_Audio, L"Audio"},
	{MEDIATYPE_Video, L"Video"},
	{MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_CATEGORY, L"Category"},
	{KSCATEGORY_VIDEO_CAMERA, L"Camera"},
	{MF_MT_YUV_MATRIX, L"YUV Matrix"},
	{MF_MT_VIDEO_LIGHTING, L"Lightning"},
	{MF_MT_VIDEO_NOMINAL_RANGE, L"Nominal range"},
	{MF_MT_GEOMETRIC_APERTURE, L"Geometric aperture"},
	{MF_MT_MINIMUM_DISPLAY_APERTURE, L"Display apperture"},
};


HRESULT MFCap_StringFromGuid(GUID* Guid, PWCHAR *String)
{
	HRESULT hr = S_OK;
	wchar_t* tmp = NULL;

	for (size_t i = 0; i < sizeof(_guidMap) / sizeof(_guidMap[0]); ++i) {
		if (*Guid == _guidMap[i].GUid) {
			tmp = (wchar_t*)CoTaskMemAlloc((wcslen(_guidMap[i].String) + 1)*sizeof(wchar_t));
			if (tmp != NULL) {
				memcpy(tmp, _guidMap[i].String, (wcslen(_guidMap[i].String) + 1) * sizeof(wchar_t));
			} else hr = E_OUTOFMEMORY;

			break;
		}
	}

	if (SUCCEEDED(hr) && tmp == NULL)
		hr = StringFromIID(*Guid, &tmp);

	if (SUCCEEDED(hr))
		*String = tmp;

	return hr;
}


void MFCap_StringFree(PWCHAR String)
{
	CoTaskMemFree(String);

	return;
}


HRESULT MFCap_Init(void)
{
	HRESULT hr = S_OK;

	hr = CoInitialize(NULL);
	if (SUCCEEDED(hr)) {
		hr = MFStartup(MF_SDK_VERSION, 0);
		if (FAILED(hr))
			CoUninitialize();
	}

	return hr;
}


void MFCap_Finit(void)
{
	MFShutdown();
	CoUninitialize();

	return;
}
