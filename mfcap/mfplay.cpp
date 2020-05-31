
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


extern "C" HRESULT MFPlay_EnumFormats(PMFPLAY_DEVICE Device, PMFCAP_FORMAT* Formats, DWORD* Count, DWORD *StreamCount)
{
	HRESULT ret = S_OK;
	DWORD streamCount = 0;
	DWORD mtCount = 0;
	std::vector<MFCAP_FORMAT> formats;
	IMFStreamSink* ss = NULL;
	IMFMediaTypeHandler* mth = NULL;
	IMFMediaType* mt = NULL;
	MFCAP_FORMAT format;
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
	PMFCAP_FORMAT tmpFormats = NULL;

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
					memset(&format, 0, sizeof(format));
					mt = NULL;
					ret = mth->GetMediaTypeByIndex(j, &mt);
					if (SUCCEEDED(ret))
						ret = mt->GetGUID(MF_MT_MAJOR_TYPE, &format.TypeGuid);

					if (SUCCEEDED(ret))
						ret = mt->GetGUID(MF_MT_SUBTYPE, &format.SubtypeGuid);

					if (SUCCEEDED(ret)) {
						wcscpy(format.FriendlyName, L"UNKNOWN");
						for (size_t k = 0; k < sizeof(formatGuids) / sizeof(formatGuids[0]); ++k) {
							if (formatGuids[k] == format.SubtypeGuid) {
								wcscpy(format.FriendlyName, formatNames[k]);
								break;
							}
						}

						format.StreamIndex = i;
						format.Index = j;
						if (format.TypeGuid == MFMediaType_Video) {
							format.Type = mcftVideo;
							MFGetAttributeSize(mt, MF_MT_FRAME_SIZE, &format.Video.Width, &format.Video.Height);
							mt->GetUINT32(MF_MT_AVG_BITRATE, &format.Video.BitRate);
							mt->GetUINT32(MF_MT_FRAME_RATE, &format.Video.Framerate);
						} else if (format.TypeGuid == MFMediaType_Audio) {
							format.Type = mcftAudio;
							mt->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &format.Audio.BitsPerSample);
							mt->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &format.Audio.ChannelCount);
							mt->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &format.Audio.SamplesPerSecond);
						}
						else format.Type = mcftUnknown;

						format.MediaType = mt;
						mt->AddRef();
						formats.push_back(format);
					}

					if (mt != NULL)
						mt->Release();

					if (FAILED(ret))
						break;
				}
			}

			if (mth != NULL)
				mth->Release();

			if (ss != NULL)
				ss->Release();

			if (FAILED(ret))
				break;
		}

		if (ret == MF_E_INVALIDSTREAMNUMBER)
			ret = S_OK;

		if (SUCCEEDED(ret)) {
			*StreamCount = streamCount;
			*Count = (UINT32)formats.size();
			*Formats = NULL;
			if (formats.size() > 0) {
				tmpFormats = (PMFCAP_FORMAT)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, formats.size() * sizeof(tmpFormats[0]));
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
		ret = MFCreateAudioRenderer(attrs, &d->Sink);

	if (SUCCEEDED(ret))
		ret = d->Sink->GetCharacteristics(&d->Characteristics);

	if (SUCCEEDED(ret))
		d->Sink->AddRef();

	if (d->Sink != NULL)
		d->Sink->Release();

	if (SUCCEEDED(ret))
		*Device = d;

	if (attrs != NULL)
		attrs->Release();

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
