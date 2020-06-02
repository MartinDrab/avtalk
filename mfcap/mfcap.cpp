
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
#include "mfgen.h"
#include "mfcap.h"

#pragma comment(lib, "strmiids")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Propsys.lib")



typedef struct _SAMPLING_THREAD_CONTEXT {
	PMFCAP_DEVICE Device;
	IMFSourceReader* MediaSourceReader;
} SAMPLING_THREAD_CONTEXT, * PSAMPLING_THREAD_CONTEXT;


static DWORD WINAPI _SamplingThreadRoutine(PVOID Context)
{
	HRESULT ret = S_OK;
	PSAMPLING_THREAD_CONTEXT ctx = (PSAMPLING_THREAD_CONTEXT)Context;
	LONGLONG timeStamp = 0;
	DWORD streamIndex = 0;
	IMFSample* sample = NULL;
	DWORD streamFlags = 0;
	UINT32 callbackFlags = 0;
	IMFMediaBuffer* buffer = NULL;
	void* data = NULL;
	DWORD len = 0;
	PMFCAP_DEVICE device = ctx->Device;

	ret = CoInitialize(NULL);
	if (SUCCEEDED(ret)) {
		while (SUCCEEDED(ret) && !device->SamplingTerminated) {
			timeStamp = 0;
			streamIndex = 0;
			streamFlags = 0;
			callbackFlags = 0;
			sample = NULL;
			data = NULL;
			len = 0;
			ret = ctx->MediaSourceReader->ReadSample(MF_SOURCE_READER_ANY_STREAM, 0, &streamIndex, &streamFlags, &timeStamp, &sample);
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
						ret = buffer->Lock((PBYTE*)&data, NULL, &len);
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

		ctx->MediaSourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
		ctx->MediaSourceReader->Flush(MF_SOURCE_READER_ALL_STREAMS);
		ctx->MediaSourceReader->Release();
		CoUninitialize();
	}

	MFGen_RefMemRelease(ctx);

	return ret;
}


extern "C" HRESULT MFCap_EnumMediaTypes(PMFCAP_DEVICE Device, PMFGEN_FORMAT *Types, UINT32 *Count, UINT32 *StreamCount)
{
	HRESULT hr = S_OK;
	MFGEN_FORMAT format;
	IMFMediaType* nativeType = NULL;
	std::vector<MFGEN_FORMAT> types;
	PMFGEN_FORMAT tmpFormats = NULL;
	IMFPresentationDescriptor* pd = NULL;
	DWORD streamCount = 0;
	DWORD typeCount = 0;
	BOOL selected = FALSE;

	pd = Device->PresentationDescriptor;
	hr = pd->GetStreamDescriptorCount(&streamCount);
	for (DWORD i = 0; i < streamCount; ++i) {
		IMFStreamDescriptor* sd = NULL;
		IMFMediaTypeHandler* mth = NULL;

		hr = pd->GetStreamDescriptorByIndex(i, &selected, &sd);
		if (SUCCEEDED(hr))
			hr = sd->GetMediaTypeHandler(&mth);

		if (SUCCEEDED(hr))
			hr = mth->GetMediaTypeCount(&typeCount);

		for (DWORD j = 0; j < typeCount; ++j) {
			nativeType = NULL;
			hr = mth->GetMediaTypeByIndex(j, &nativeType);
			if (SUCCEEDED(hr))
				hr = MFGen_MediaTypeToFormat(nativeType, &format);

			if (SUCCEEDED(hr)) {
				format.StreamIndex = i;
				format.Selected = (Device->StreamSelectionMask & (1 << format.StreamIndex)) != 0;
				format.Index = j;
				types.push_back(format);
			}

			MFGen_SafeRelease(nativeType);
			if (FAILED(hr))
				break;
		}

		MFGen_SafeRelease(mth);
		MFGen_SafeRelease(sd);
		if (FAILED(hr))
			break;
	}

	if (SUCCEEDED(hr)) {
		*StreamCount = streamCount;
		*Count = (UINT32)types.size();
		*Types = NULL;
		if (types.size() > 0) {
			hr = MFGen_RefMemAlloc(types.size() * sizeof(tmpFormats[0]), (void **)&tmpFormats);
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

	return hr;
}


extern "C" HRESULT MFCap_SelectStream(PMFCAP_DEVICE Device, UINT32 StreamIndex, BOOL Select)
{
	HRESULT hr = S_OK;
	UINT32 bitValue = 0;

	bitValue = (1 << StreamIndex);
	if (Select)
		Device->StreamSelectionMask |= bitValue;
	else Device->StreamSelectionMask &= ~bitValue;

	return hr;
}



#define __HRESULT_FROM_WIN32(x) ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) : ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)))


extern "C" HRESULT MFCap_Start(PMFCAP_DEVICE Device, MFCAP_SAMPLE_CALLBACK* Callback, void* Context)
{
	HRESULT ret = S_OK;
	IMFAttributes* msAttributes = NULL;
	IMFSourceReader *msr = NULL;
	PSAMPLING_THREAD_CONTEXT ctx = NULL;

	ret = MFCreateAttributes(&msAttributes, 1);
	if (SUCCEEDED(ret))
		ret = msAttributes->SetUINT32(MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN, TRUE);
	
	if (SUCCEEDED(ret))
		ret = MFCreateSourceReaderFromMediaSource(Device->MediaSource, msAttributes, &msr);

	if (SUCCEEDED(ret))
		ret = msr->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);

	if (SUCCEEDED(ret))
		ret = msr->Flush(MF_SOURCE_READER_ALL_STREAMS);

	if (SUCCEEDED(ret)) {
		Device->SamplingCallback = Callback;
		Device->SamplingContext = Context;
		for (UINT32 i = 0; i < 31; ++i) {
			if ((Device->StreamSelectionMask & (1 << i)) != 0)
				ret = msr->SetStreamSelection(i, TRUE);

			if (FAILED(ret)) {
				msr->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
				msr->Flush(MF_SOURCE_READER_ALL_STREAMS);
				break;
			}
		}
	}

	if (SUCCEEDED(ret)) {
		ret = MFGen_RefMemAlloc(sizeof(SAMPLING_THREAD_CONTEXT), (void **)&ctx);
		if (SUCCEEDED(ret)) {
			MFGen_RefMemAddRef(ctx);
			msr->AddRef();
			ctx->MediaSourceReader = msr;
			ctx->Device = Device;
			Device->SamplingThread = CreateThread(NULL, 0, _SamplingThreadRoutine, ctx, 0, NULL);
			if (Device->SamplingThread == NULL) {
				ret = __HRESULT_FROM_WIN32(GetLastError());
				msr->Release();
				MFGen_RefMemRelease(ctx);
			}

			MFGen_RefMemRelease(ctx);
		}

		if (FAILED(ret)) {
			msr->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
			msr->Flush(MF_SOURCE_READER_ALL_STREAMS);
		}
	}

	MFGen_SafeRelease(msr);
	MFGen_SafeRelease(msAttributes);

	return ret;
}


extern "C" void MFCap_Stop(PMFCAP_DEVICE Device)
{
	Device->SamplingTerminated = TRUE;
	WaitForSingleObject(Device->SamplingThread, INFINITE);
	CloseHandle(Device->SamplingThread);
	Device->SamplingThread = NULL;
	Device->SamplingTerminated = FALSE;

	return;
}


extern "C" void MFCap_QueryStreamSelection(PMFCAP_DEVICE Device, PUINT32 StreamMask)
{
	*StreamMask = Device->StreamSelectionMask;

	return;
}


extern "C" void MFCap_QueryCharacteristics(PMFCAP_DEVICE Device, PMFCAP_DEVICE_CHARACTERISTICS Characteristics)
{
	*Characteristics = Device->Characteristics;

	return;
}


extern "C" HRESULT MFCap_CreateStreamNodes(PMFCAP_DEVICE Device, IMFTopologyNode*** Nodes, DWORD* Count)
{
	HRESULT ret = S_OK;
	DWORD sdCount = 0;
	IMFStreamDescriptor* sd = NULL;
	IMFPresentationDescriptor* pd = NULL;
	IMFTopologyNode** tmpNodes = NULL;
	BOOL selected = FALSE;
	UINT32 nodeIndex = 0;
	IMFTopologyNode* tmpNode = NULL;

	pd = Device->PresentationDescriptor;
	ret = pd->GetStreamDescriptorCount(&sdCount);
	if (SUCCEEDED(ret) && sdCount > 0) {
		ret = MFGen_RefMemAlloc(sdCount * sizeof(IMFTopologyNode*), (void **)&tmpNodes);
		if (SUCCEEDED(ret)) {
			for (UINT32 i = 0; i < sdCount; ++i) {
				sd = NULL;
				tmpNode = NULL;
				if ((Device->StreamSelectionMask & (1 << i)) != 0)
					ret = pd->SelectStream(i);
				else ret = pd->DeselectStream(i);
				
				if (SUCCEEDED(ret))
					ret = pd->GetStreamDescriptorByIndex(i, &selected, &sd);
				
				if (SUCCEEDED(ret) && selected) {
					ret = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &tmpNode);
					if (SUCCEEDED(ret))
						ret = tmpNode->SetUnknown(MF_TOPONODE_SOURCE, Device->MediaSource);

					if (SUCCEEDED(ret))
						ret = tmpNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pd);

					if (SUCCEEDED(ret))
						ret = tmpNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, sd);

					if (SUCCEEDED(ret)) {
						tmpNode->AddRef();
						tmpNodes[nodeIndex] = tmpNode;
						++nodeIndex;
					}
				}

				MFGen_SafeRelease(tmpNode);
				MFGen_SafeRelease(pd);
				if (FAILED(ret)) {
					for (UINT32 j = 0; j < nodeIndex; ++j)
						tmpNodes[i]->Release();
					
					break;
				}
			}

			if (SUCCEEDED(ret))
				MFGen_RefMemAddRef(tmpNodes);

			MFGen_RefMemRelease(tmpNodes);
		}
	}

	if (SUCCEEDED(ret)) {
		*Nodes = tmpNodes;
		*Count = nodeIndex;
	}

	return ret;
}


extern "C" void MFCap_FreeStreamNodes(IMFTopologyNode** Nodes, UINT32 Count)
{
	if (Count > 0) {
		for (UINT32 i = 0; i < Count; ++i)
			Nodes[i]->Release();

		MFGen_RefMemRelease(Nodes);
	}

	return;
}


extern "C" HRESULT MFCap_EnumDevices(EMFCapFormatType Type, PMFCAP_DEVICE_INFO* Devices, PUINT32 Count)
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

		MFGen_SafeRelease(attributes);
		if (FAILED(ret))
			break;
	}

	if (SUCCEEDED(ret)) {
		*Devices = NULL;
		*Count = NULL;
		if (ds.size() > 0) {
			ret = MFGen_RefMemAlloc(ds.size() * sizeof(tmpDevices[0]), (void **)&tmpDevices);
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


extern "C" void MFCap_FreeDeviceEnumeration(PMFCAP_DEVICE_INFO Devices, UINT32 Count)
{
	if (Count > 0) {
		for (UINT32 i = 0; i < Count; ++i) {
			CoTaskMemFree(Devices[i].SymbolicLink);
			CoTaskMemFree(Devices[i].FriendlyName);
		}

		MFGen_RefMemRelease(Devices);
	}

	return;
}


extern "C" HRESULT MFCap_GetDeviceCount(EMFCapFormatType Type, UINT32 *aCount)
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

	MFGen_SafeRelease(attributes);

	return hr;
}


extern "C" HRESULT MFCap_NewInstance(EMFCapFormatType Type, UINT32 Index, PMFCAP_DEVICE* aInstance)
{
	HRESULT ret = S_OK;
	UINT32 deviceCount = 0;
	IMFActivate** devices = NULL;
	IMFAttributes* attributes = NULL;
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
		ret = devices[Index]->ActivateObject(__uuidof(IMFMediaSource), (void**)&ms);
		if (SUCCEEDED(ret))
			ret = MFGen_RefMemAlloc(sizeof(MFCAP_DEVICE), (void **)&cam);

		if (SUCCEEDED(ret)) {
			cam->Type = Type;
			cam->StreamSelectionMask = 0xffffffff;
			ret = ms->GetCharacteristics(&cam->Characteristics.Value);
			if (SUCCEEDED(ret))
				ret = ms->CreatePresentationDescriptor(&cam->PresentationDescriptor);
		}

		if (SUCCEEDED(ret)) {
			ms->AddRef();
			cam->MediaSource = ms;
			MFGen_RefMemAddRef(cam);
			*aInstance = cam;
		}

		MFGen_RefMemRelease(cam);
		MFGen_SafeRelease(ms);
		for (UINT32 i = 0; i < deviceCount; ++i)
			devices[i]->Release();

		CoTaskMemFree(devices);
	}


	MFGen_SafeRelease(attributes);

	return ret;
}


extern "C" HRESULT MFCap_NewInstanceFromURL(PWCHAR URL, PMFCAP_DEVICE* Device)
{
	HRESULT ret = S_OK;
	PMFCAP_DEVICE d = NULL;
	IMFSourceResolver* r = NULL;
	MF_OBJECT_TYPE objectType;
	IMFMediaSource* ms = NULL;

	ret = MFGen_RefMemAlloc(sizeof(MFCAP_DEVICE), (void **)&d);
	if (SUCCEEDED(ret))
		ret = MFCreateSourceResolver(&r);
	
	if (SUCCEEDED(ret))
		ret = r->CreateObjectFromURL(URL, MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE | MF_RESOLUTION_READ, NULL, &objectType, (IUnknown **)&ms);

	if (SUCCEEDED(ret))
		ret = ms->GetCharacteristics(&d->Characteristics.Value);

	if (SUCCEEDED(ret))
		ret = ms->CreatePresentationDescriptor(&d->PresentationDescriptor);

	if (SUCCEEDED(ret)) {
		ms->AddRef();
		d->MediaSource = ms;
		MFGen_RefMemAddRef(d);
		*Device = d;
	}

	MFGen_SafeRelease(ms);
	MFGen_SafeRelease(r);
	MFGen_RefMemRelease(d);

	return ret;
}


extern "C" void MFCap_FreeInstance(PMFCAP_DEVICE Instance)
{
	Instance->PresentationDescriptor->Release();
//	Instance->MediaSource->Shutdown();
	Instance->MediaSource->Release();
	MFGen_RefMemRelease(Instance);

	return;
}


extern "C" HRESULT MFCap_GetProperties(EMFCapFormatType Type, UINT32 Index, GUID** Guids, PROPVARIANT** Values, UINT32* Count)
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
		hr = MFGen_GetProperties(d, Guids, Values, Count);

	MFGen_SafeRelease(d);
	MFGen_SafeRelease(attributes);

	return hr;
}


extern "C" HRESULT MFCap_SetFormat(PMFCAP_DEVICE Device, UINT32 Stream, IMFMediaType *Format)
{
	HRESULT hr = S_OK;
	IMFStreamDescriptor* sd = NULL;
	IMFMediaTypeHandler* mth = NULL;
	BOOL selected = FALSE;

	hr = Device->PresentationDescriptor->GetStreamDescriptorByIndex(Stream, &selected, &sd);
	if (SUCCEEDED(hr))
		hr = sd->GetMediaTypeHandler(&mth);

	if (SUCCEEDED(hr))
		hr = mth->SetCurrentMediaType(Format);

	MFGen_SafeRelease(mth);
	MFGen_SafeRelease(sd);

	return hr;
}


extern "C" HRESULT MFCap_Init(void)
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


extern "C" void MFCap_Finit(void)
{
	MFShutdown();
	CoUninitialize();

	return;
}
