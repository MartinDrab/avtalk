
#include <vector>
#include <cstdint>
#include <initguid.h>
#include <windows.h>
#include <objbase.h>
#include <mmdeviceapi.h>
#include <mfapi.h>
#include <propvarutil.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "mfcap.h"
#include "mfplay.h"


#pragma comment(lib, "strmiids")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Propsys.lib")



extern "C" HRESULT MFPlay_EnumDevices(MFPLAY_DEVICE_STATE_MASK StateMask, PMFPLAY_DEVICE_INFO* Devices, uint32_t* Count)
{
	HRESULT ret = S_OK;
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDeviceCollection* collection = NULL;
	IMMDevice* d = NULL;
	uint32_t tmpCount = 0;
	PMFPLAY_DEVICE_INFO tmpDevices = NULL;
	wchar_t tmp[MAX_PATH];

	ret = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	if (SUCCEEDED(ret))
		ret = pEnumerator->EnumAudioEndpoints(eRender, StateMask.Value, &collection);
	
	if (SUCCEEDED(ret))
		ret = collection->GetCount(&tmpCount);

	if (SUCCEEDED(ret) && tmpCount > 0) {
		ret = MFGen_RefMemAlloc(tmpCount * sizeof(MFPLAY_DEVICE_INFO), (void **)&tmpDevices);
		if (SUCCEEDED(ret)) {
			for (UINT i = 0; i < tmpCount; ++i) {
				PROPVARIANT varName;
				PROPVARIANT varDesc;
				IPropertyStore* pProps = NULL;

				ret = collection->Item(i, &d);
				if (SUCCEEDED(ret))
					ret = d->GetId(&tmpDevices[i].EndpointId);

				if (SUCCEEDED(ret))
					ret = d->GetState(&tmpDevices[i].State);

				if (SUCCEEDED(ret))
					ret = d->OpenPropertyStore(STGM_READ, &pProps);

				if (SUCCEEDED(ret)) {
					PropVariantInit(&varName);
					ret = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
					if (SUCCEEDED(ret))
						ret = PropVariantToString(varName, tmp, sizeof(tmp) / sizeof(wchar_t));

					if (SUCCEEDED(ret)) {
						tmpDevices[i].Name = (wchar_t*)CoTaskMemAlloc((wcslen(tmp) + 1) * sizeof(wchar_t));
						if (tmpDevices[i].Name == NULL)
							ret = E_OUTOFMEMORY;

						if (SUCCEEDED(ret))
							memcpy(tmpDevices[i].Name, tmp, (wcslen(tmp) + 1) * sizeof(wchar_t));
					}

					PropVariantClear(&varName);
					ret = S_OK;
				}

				if (SUCCEEDED(ret)) {
					PropVariantInit(&varDesc);
					ret = pProps->GetValue(PKEY_Device_DeviceDesc, &varDesc);
					if (SUCCEEDED(ret))
						ret = PropVariantToString(varDesc, tmp, sizeof(tmp) / sizeof(wchar_t));

					if (SUCCEEDED(ret)) {
						tmpDevices[i].Description = (wchar_t *)CoTaskMemAlloc((wcslen(tmp) + 1) * sizeof(wchar_t));
						if (tmpDevices[i].Description == NULL)
							ret = E_OUTOFMEMORY;

						if (SUCCEEDED(ret))
							memcpy(tmpDevices[i].Description, tmp, (wcslen(tmp) + 1) * sizeof(wchar_t));
					}

					PropVariantClear(&varDesc);
				}

				MFGen_SafeRelease(pProps);
				MFGen_SafeRelease(d);
			}

			if (SUCCEEDED(ret))
				MFGen_RefMemAddRef(tmpDevices);

			MFGen_RefMemRelease(tmpDevices);
		}
	}

	if (SUCCEEDED(ret)) {
		*Devices = tmpDevices;
		*Count = tmpCount;
	}

	MFGen_SafeRelease(collection);
	MFGen_SafeRelease(pEnumerator);

	return ret;
}


extern "C" void MFPlay_FreeDeviceEnum(PMFPLAY_DEVICE_INFO Devices, uint32_t Count)
{
	if (Count > 0) {
		for (size_t i = 0; i < Count; ++i) {
			if (Devices[i].Description != NULL)
				CoTaskMemFree(Devices[i].Description);
			
			if (Devices[i].Name != NULL)
				CoTaskMemFree(Devices[i].Name);
			
			if (Devices[i].EndpointId != NULL)
				CoTaskMemFree(Devices[i].EndpointId);
		}

		MFGen_RefMemRelease(Devices);
	}

	return;
}


extern "C" HRESULT MFPlay_EnumFormats(PMFPLAY_DEVICE Device, PMFGEN_FORMAT* Formats, DWORD* Count, DWORD *StreamCount)
{
	HRESULT ret = S_OK;
	DWORD streamCount = 0;
	DWORD mtCount = 0;
	std::vector<MFGEN_FORMAT> formats;
	IMFStreamSink* ss = NULL;
	IMFMediaTypeHandler* mth = NULL;
	IMFMediaType* mt = NULL;
	MFGEN_FORMAT format;
	PMFGEN_FORMAT tmpFormats = NULL;

	ret = Device->Sink->GetStreamSinkCount(&streamCount);
	if (SUCCEEDED(ret)) {
		for (DWORD i = 0; i < streamCount; ++i) {
			ss = NULL;
			mth = NULL;
			ret = Device->Sink->GetStreamSinkByIndex(i, &ss);
			if (SUCCEEDED(ret))
				ret = ss->GetMediaTypeHandler(&mth);

			if (SUCCEEDED(ret))
				ret = mth->GetMediaTypeCount(&mtCount);

			if (SUCCEEDED(ret)) {
				for (DWORD j = 0; j < mtCount; ++j) {
					mt = NULL;
					ret = mth->GetMediaTypeByIndex(j, &mt);
					if (SUCCEEDED(ret))
						ret = MFGen_MediaTypeToFormat(mt, &format);

					if (SUCCEEDED(ret)) {
						format.StreamIndex = i;
						format.Index = j;
						formats.push_back(format);
					}

					MFGen_SafeRelease(mt);
					if (FAILED(ret))
						break;
				}
			}

			MFGen_SafeRelease(mth);
			MFGen_SafeRelease(ss);
			if (FAILED(ret))
				break;
		}

		if (SUCCEEDED(ret)) {
			*StreamCount = streamCount;
			*Count = (UINT32)formats.size();
			*Formats = NULL;
			if (formats.size() > 0) {
				ret = MFGen_RefMemAlloc(formats.size() * sizeof(tmpFormats[0]), (void **)&tmpFormats);
				if (SUCCEEDED(ret)) {
					for (size_t i = 0; i < formats.size(); ++i) {
						tmpFormats[i] = formats[i];
						tmpFormats[i].MediaType->AddRef();
					}

					*Formats = tmpFormats;
				}
			}
		}

		for (auto& mt : formats)
			mt.MediaType->Release();

		formats.clear();
	}

	return ret;
}


extern "C" HRESULT MFPlay_NewInstance(const MFPLAY_DEVICE_INFO *DeviceInfo, PMFPLAY_DEVICE * Device)
{
	HRESULT ret = S_OK;
	PMFPLAY_DEVICE d = NULL;
	IMFAttributes* attrs = NULL;
	IMFMediaSink* s = NULL;

	ret = MFGen_RefMemAlloc(sizeof(MFPLAY_DEVICE), (void**)&d);
	if (SUCCEEDED(ret))
		ret = MFCreateAttributes(&attrs, 2);

	if (SUCCEEDED(ret))
		ret = attrs->SetString(MF_AUDIO_RENDERER_ATTRIBUTE_ENDPOINT_ID, DeviceInfo->EndpointId);

	if (SUCCEEDED(ret))
		ret = attrs->SetUINT32(MF_AUDIO_RENDERER_ATTRIBUTE_FLAGS, MF_AUDIO_RENDERER_ATTRIBUTE_FLAGS_CROSSPROCESS);

	if (SUCCEEDED(ret))
		ret = MFCreateAudioRenderer(attrs, &s);

	if (SUCCEEDED(ret))
		ret = s->GetCharacteristics(&d->Characteristics);

	if (SUCCEEDED(ret)) {
		s->AddRef();
		d->Sink = s;
		MFGen_RefMemAddRef(d);
		*Device = d;
	}

	MFGen_SafeRelease(s);
	MFGen_SafeRelease(attrs);
	MFGen_RefMemRelease(d);

	return ret;
}


extern "C" HRESULT MFPlay_NewInstanceForWindow(HWND Window, PMFPLAY_DEVICE* Device)
{
	HRESULT ret = S_OK;
	IMFActivate* a = NULL;
	PMFPLAY_DEVICE d = NULL;
	IMFMediaSink* s = NULL;

	ret = MFGen_RefMemAlloc(sizeof(MFPLAY_DEVICE), (void **)&d);
	if (SUCCEEDED(ret)) {
		d->Window = Window;
		ret = MFCreateVideoRendererActivate(Window, &a);
	}

	if (SUCCEEDED(ret))
		ret = a->ActivateObject(__uuidof(s), (void **)&s);

	if (SUCCEEDED(ret))
		ret = s->GetCharacteristics(&d->Characteristics);

	if (SUCCEEDED(ret)) {
		s->AddRef();
		d->Sink = s;
		MFGen_RefMemAddRef(d);
		*Device = d;
	}

	MFGen_SafeRelease(s);
	MFGen_SafeRelease(a);
	MFGen_RefMemRelease(d);

	return ret;
}


extern "C" void MFPlay_FreeInstance(PMFPLAY_DEVICE Device)
{
	Device->Sink->Shutdown();
	Device->Sink->Release();
	MFGen_RefMemRelease(Device);

	return;
}


extern "C" HRESULT MFPlay_CreateStreamNodes(PMFPLAY_DEVICE Device, PMFGEN_STREAM_INFO *Nodes, DWORD* Count)
{
	HRESULT ret = S_OK;
	DWORD tmpCount = 0;
	IMFStreamSink* s = NULL;
	PMFGEN_STREAM_INFO node = NULL;
	PMFGEN_STREAM_INFO tmpNodes = NULL;
	IMFMediaTypeHandler* mth = NULL;

	ret = Device->Sink->GetStreamSinkCount(&tmpCount);
	if (SUCCEEDED(ret) && tmpCount > 0) {
		ret = MFGen_RefMemAlloc(tmpCount*sizeof(MFGEN_STREAM_INFO), (void **)&tmpNodes);
		if (SUCCEEDED(ret)) {
			node = tmpNodes;
			for (DWORD i = 0; i < tmpCount; ++i) {
				s = NULL;
				mth = NULL;
				ret = Device->Sink->GetStreamSinkByIndex(i, &s);
				if (SUCCEEDED(ret))
					ret = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &node->Node);

				if (SUCCEEDED(ret))
					ret = node->Node->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, TRUE);

				if (SUCCEEDED(ret))
					ret = node->Node->SetObject(s);

				if (SUCCEEDED(ret))
					ret = s->GetIdentifier(&node->Id);

				if (SUCCEEDED(ret))
					ret = s->GetMediaTypeHandler(&mth);

				if (SUCCEEDED(ret))
					ret = mth->GetMajorType(&node->MajorType);

				if (SUCCEEDED(ret)) {
					node->Type = mcftUnknown;
					if (node->MajorType == MFMediaType_Video)
						node->Type = mcftVideo;
					else if (node->MajorType == MFMediaType_Audio)
						node->Type = mcftAudio;
				}

				if (SUCCEEDED(ret)) {
					node->Index = i;
					node->Node->AddRef();
					node->Selected = TRUE;
				}

				MFGen_SafeRelease(mth);
				MFGen_SafeRelease(node->Node);
				MFGen_SafeRelease(s);
				if (FAILED(ret)) {
					for (DWORD j = 0; j < i; ++j)
						tmpNodes[j].Node->Release();
					
					break;
				}

				++node;
			}

			if (SUCCEEDED(ret))
				MFGen_RefMemAddRef(tmpNodes);

			MFGen_RefMemRelease(tmpNodes);
		}
	}
	
	if (SUCCEEDED(ret)) {
		*Nodes = tmpNodes;
		*Count = tmpCount;
	}

	return ret;
}


HRESULT MFPlay_QueryCharacteristics(PMFPLAY_DEVICE Device, PDWORD Characteristics)
{
	HRESULT ret = S_OK;

	*Characteristics = Device->Characteristics;

	return ret;
}
