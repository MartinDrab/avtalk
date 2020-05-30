
#include <windows.h>
#include <propvarutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mfcap.h"
#include "mfplay.h"




template <EMFCapFormatType Type>
HRESULT ProcessProperties(GUID* Guids, PROPVARIANT* Values, UINT32 Count)
{
	GUID g;
	HRESULT ret = S_OK;
	PWCHAR guidString = NULL;

	for (UINT32 k = 0; k < Count; ++k) {
		PROPVARIANT tmp;
		
		ret = MFCap_StringFromGuid(Guids + k, &guidString);
		if (FAILED(ret)) {
			fprintf(stderr, "%u: MFCap_StringFree: 0x%x\n", Type, ret);
			continue;
		}

		fprintf(stdout, "%u:   %ls\t%u\t", Type, guidString, Values[k].vt);
		MFCap_StringFree(guidString);
		switch (Values[k].vt) {
			case VT_CLSID:
				ret = PropVariantToCLSID(Values[k], &g);
				if (FAILED(ret)) {
					fprintf(stderr, "\n%u: PropVariantToCLSID: 0x%x\n", Type, ret);
					continue;
				}

				ret = MFCap_StringFromGuid(&g, &guidString);
				if (FAILED(ret)) {
					fprintf(stderr, "\n%u: MFCap_StringFromGuid: 0x%x\n", Type, ret);
					continue;
				}

				fprintf(stdout, "%ls\n", guidString);
				MFCap_StringFree(guidString);
				break;
			default:
				ret = PropVariantChangeType(&tmp, Values[k], 0, VT_LPWSTR);
				if (FAILED(ret)) {
					fprintf(stderr, "\n%u: PropVariantChangeType: 0x%x\n", Type, ret);
					continue;
				}

				fprintf(stdout, "%ls\n", tmp.pwszVal);
				PropVariantClear(&tmp);
				break;
		}
	}

	return ret;
}


template <EMFCapFormatType Type>
HRESULT ProcessDeviceType(void)
{
	HRESULT ret = S_OK;
	UINT32 deviceCount = 0;
	GUID* guids = NULL;
	UINT32 valueCount = 0;
	PROPVARIANT* values = NULL;
	PWCHAR guidString = NULL;

	ret = MFCap_GetDeviceCount(Type, &deviceCount);
	if (SUCCEEDED(ret)) {
		fprintf(stdout, "%u: Number of devices: %u\n", Type, deviceCount);
		for (UINT32 j = 0; j < deviceCount; ++j) {
			ret = MFCap_GetProperties(Type, j, &guids, &values, &valueCount);
			if (FAILED(ret)) {
				fprintf(stderr, "%u: MFCap_GetProperties: 0x%x\n", Type, ret);
				continue;
			}
			
			fprintf(stdout, "%u: Device #%u\n", Type, j);
			ret = ProcessProperties<Type>(guids, values, valueCount);
			MFCap_FreeProperties(guids, values, valueCount);
			if (SUCCEEDED(ret)) {
				PMFCAP_DEVICE d = NULL;
				PMFCAP_FORMAT formats = NULL;
				PMFCAP_FORMAT tmpFormat = NULL;
				UINT32 formatCount = 0;
				UINT32 streamCount;
				PWCHAR typeGuidString = NULL;
				PWCHAR subtypeGuidString = NULL;
				MFCAP_DEVICE_ATTRIBUTES deviceAttrs;

				memset(&deviceAttrs, 0, sizeof(deviceAttrs));
				deviceAttrs.DisableConverters = FALSE;
				deviceAttrs.ExtraSoftwareProcessing = TRUE;
				deviceAttrs.HardwareTransforms = TRUE;
				ret = MFCap_NewInstance(Type, j, &deviceAttrs, &d);
				if (FAILED(ret)) {
					fprintf(stderr, "%u: MFCap_NewInstance: 0x%x\n", Type, ret);
					continue;
				}

				ret = MFCap_EnumMediaTypes(d, &formats, &formatCount, &streamCount);
				if (FAILED(ret)) {
					MFCap_FreeInstance(d);
					fprintf(stderr, "%u: MFCap_EnumMediaTypes: 0x%x\n", Type, ret);
					continue;
				}

				fprintf(stdout, "%u:  Total number of formats: %u\n", Type, formatCount);
				fprintf(stdout, "%u:  Stream count: %u\n", Type, streamCount);
				tmpFormat = formats;
				for (UINT32 k = 0; k < formatCount; ++k) {
					ret = MFCap_StringFromGuid(&tmpFormat->TypeGuid, &typeGuidString);
					if (FAILED(ret)) {
						fprintf(stderr, "%u: MFCap_StringFromGuid: 0x%x\n", Type, ret);
						continue;
					}

					ret = MFCap_StringFromGuid(&tmpFormat->SubtypeGuid, &subtypeGuidString);
					if (FAILED(ret)) {
						fprintf(stderr, "%u: MFCap_StringFromGuid: 0x%x\n", Type, ret);
						continue;
					}

					fprintf(stdout, "%u:    Stream index: %u\n", Type, tmpFormat->StreamIndex);
					fprintf(stdout, "%u:    Index: %u\n", Type, tmpFormat->Index);
					fprintf(stdout, "%u:    Selected: %u\n", Type, tmpFormat->Selected);
					fprintf(stdout, "%u:    Type: %u\n", Type, tmpFormat->Type);
					fprintf(stdout, "%u:    Type GUID: %ls\n", Type, typeGuidString);
					fprintf(stdout, "%u:    Subtype GUID: %ls\n", Type, subtypeGuidString);
					MFCap_StringFree(subtypeGuidString);
					MFCap_StringFree(typeGuidString);
					switch (tmpFormat->Type) {
						case mcftVideo:
							fprintf(stdout, "%u:    Width: %u\n", Type, tmpFormat->Video.Width);
							fprintf(stdout, "%u:    Height: %u\n", Type, tmpFormat->Video.Height);
							fprintf(stdout, "%u:    Framerate: %u\n", Type, tmpFormat->Video.Framerate);
							fprintf(stdout, "%u:    Bitrate: %u\n", Type, tmpFormat->Video.BitRate);
							break;
						case mcftAudio:
							fprintf(stdout, "%u:    Channels: %u\n", Type, tmpFormat->Audio.ChannelCount);
							fprintf(stdout, "%u:    Bits per sample: %u\n", Type, tmpFormat->Audio.BitsPerSample);
							fprintf(stdout, "%u:    Samples per second: %u\n", Type, tmpFormat->Audio.SamplesPerSecond);
							break;
					}

					ret = MFCap_GetFormatProperties(tmpFormat, &guids, &values, &valueCount);
					if (FAILED(ret)) {
						fprintf(stderr, "%u: MFCap_GetFormatProperties: 0x%x\n", Type, ret);
						continue;
					}

					ProcessProperties<Type>(guids, values, streamCount);
					MFCap_FreeProperties(guids, values, streamCount);
					fprintf(stdout, "%u:\n", Type);
					++tmpFormat;
				}

				MFCap_FreeMediaTypes(formats, formatCount);
				MFCap_FreeInstance(d);
			}
		}
	}

	return ret;
}



int wmain(int argc, wchar_t* argv[])
{
	int ret = 0;

	ret = MFCap_Init();
	if (FAILED(ret)) {
		fprintf(stderr, "MFCap_Init: 0x%x", ret);
		goto Exit;
	}

	ret = ProcessDeviceType<mcftVideo>();
	if (FAILED(ret)) {
		fprintf(stderr, "ProcessDeviceType: 0x%x\n", ret);
		goto FInitMFCap;
	}

	ret = ProcessDeviceType<mcftAudio>();
	if (FAILED(ret)) {
		fprintf(stderr, "ProcessDeviceType: 0x%x\n", ret);
		goto FInitMFCap;
	}

FInitMFCap:
	MFCap_Finit();
Exit:
	return ret;
}
