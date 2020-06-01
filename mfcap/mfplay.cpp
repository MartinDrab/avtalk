
#include <vector>
#include <cstdint>
#include <windows.h>
#include <objbase.h>
#include <mmdeviceapi.h>
#include <mfapi.h>
#include <propvarutil.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "mfcap.h"
#include "mfplay.h"





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
		tmpDevices = (PMFPLAY_DEVICE_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, tmpCount*sizeof(MFPLAY_DEVICE_INFO));
		if (tmpDevices != NULL) {
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

				if (pProps != NULL)
					pProps->Release();

				if (d != NULL)
					d->Release();
			}

			if (FAILED(ret))
				HeapFree(GetProcessHeap(), 0, tmpDevices);
		} else ret = E_OUTOFMEMORY;
	}

	if (SUCCEEDED(ret)) {
		*Devices = tmpDevices;
		*Count = tmpCount;
	}

	if (collection != NULL)
		collection->Release();

	if (pEnumerator != NULL)
		pEnumerator->Release();

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

		HeapFree(GetProcessHeap(), 0, Devices);
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
				tmpFormats = (PMFGEN_FORMAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, formats.size() * sizeof(tmpFormats[0]));
				if (tmpFormats == NULL)
					ret = E_OUTOFMEMORY;

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

	d = (PMFPLAY_DEVICE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MFPLAY_DEVICE));
	if (d == NULL)
		ret = E_OUTOFMEMORY;

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
		*Device = d;
	}

	MFGen_SafeRelease(s);
	MFGen_SafeRelease(attrs);
	if (FAILED(ret) && d != NULL)
		HeapFree(GetProcessHeap(), 0, d);

	return ret;
}


extern "C" HRESULT MFPlay_NewInstanceForWindow(HWND Window, PMFPLAY_DEVICE* Device)
{
	HRESULT ret = S_OK;
	IMFActivate* a = NULL;
	PMFPLAY_DEVICE d = NULL;
	IMFMediaSink* s = NULL;

	d = (PMFPLAY_DEVICE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MFPLAY_DEVICE));
	if (d == NULL)
		ret = E_OUTOFMEMORY;

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
		*Device = d;
	}

	MFGen_SafeRelease(s);
	MFGen_SafeRelease(a);
	if (FAILED(ret) && d != NULL)
		HeapFree(GetProcessHeap(), 0, d);

	return ret;
}


extern "C" void MFPlay_FreeInstance(PMFPLAY_DEVICE Device)
{
	Device->Sink->Shutdown();
	Device->Sink->Release();
	HeapFree(GetProcessHeap(), 0, Device);

	return;
}


extern "C" HRESULT MFPlay_CreateNodes(PMFPLAY_DEVICE Device, IMFTopologyNode*** Nodes, DWORD* Count)
{
	HRESULT ret = S_OK;
	DWORD tmpCount = 0;
	IMFStreamSink* s = NULL;
	IMFTopologyNode* node = NULL;
	IMFTopologyNode** tmpNodes = NULL;

	ret = Device->Sink->GetStreamSinkCount(&tmpCount);
	if (SUCCEEDED(ret) && tmpCount > 0) {
		tmpNodes = (IMFTopologyNode**)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, tmpCount*sizeof(IMFTopologyNode *));
		if (tmpNodes == NULL)
			ret = E_OUTOFMEMORY;

		if (SUCCEEDED(ret)) {
			for (DWORD i = 0; i < tmpCount; ++i) {
				ret = Device->Sink->GetStreamSinkByIndex(i, &s);
				if (SUCCEEDED(ret))
					ret = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &node);

				if (SUCCEEDED(ret))
					ret = node->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, TRUE);

				if (SUCCEEDED(ret))
					ret = node->SetObject(s);

				if (SUCCEEDED(ret)) {
					node->AddRef();
					tmpNodes[i] = node;
				}

				if (node != NULL)
					node->Release();

				if (s != NULL)
					s->Release();

				if (FAILED(ret)) {
					for (DWORD j = 0; j < i; ++j)
						tmpNodes[j]->Release();
					
					break;
				}
			}

			if (FAILED(ret))
				HeapFree(GetProcessHeap(), 0, tmpNodes);
		}
	}
	
	if (SUCCEEDED(ret)) {
		*Nodes = tmpNodes;
		*Count = tmpCount;
	}

	return ret;
}
