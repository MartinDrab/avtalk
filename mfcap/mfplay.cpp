
#include <cstdint>
#include <windows.h>
#include <objbase.h>
#include <mmdeviceapi.h>
#include <mfapi.h>
#include <propvarutil.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "mfplay.h"





HRESULT MFPlay_EnumDevices(PMFPLAY_DEVICE_INFO* Devices, uint32_t* Count)
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
		ret = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATEMASK_ALL, &collection);

	if (SUCCEEDED(ret))
		ret = collection->GetCount(&tmpCount);

	if (SUCCEEDED(ret) && tmpCount > 0) {
		tmpDevices = (PMFPLAY_DEVICE_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, tmpCount*sizeof(MFPLAY_DEVICE));
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
				}

				if (SUCCEEDED(ret)) {
					PropVariantInit(&varDesc);
					ret = pProps->GetValue(PKEY_Device_DeviceDesc, &varDesc);
					if (SUCCEEDED(ret))
						ret = PropVariantToString(varDesc, tmp, sizeof(tmp) / sizeof(wchar_t));

					if (SUCCEEDED(ret)) {
						tmpDevices[i].Name = (wchar_t *)CoTaskMemAlloc((wcslen(tmp) + 1) * sizeof(wchar_t));
						if (tmpDevices[i].Name == NULL)
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



void MFPlay_FreeDeviceEnum(PMFPLAY_DEVICE_INFO Devices, uint32_t Count)
{
	if (Count > 0) {
		for (size_t i = 0; i < Count; ++i) {
			CoTaskMemFree(Devices[i].Description);
			CoTaskMemFree(Devices[i].Name);
			CoTaskMemFree(Devices[i].EndpointId);
		}

		HeapFree(GetProcessHeap(), 0, Devices);
	}

	return;
}
