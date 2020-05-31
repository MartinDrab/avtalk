
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
			fprintf(stderr, "MFCap_StringFree: 0x%x\n", ret);
			continue;
		}

		fprintf(stdout, "   %ls\t%u\t", guidString, Values[k].vt);
		MFCap_StringFree(guidString);
		switch (Values[k].vt) {
			case VT_CLSID:
				ret = PropVariantToCLSID(Values[k], &g);
				if (FAILED(ret)) {
					fprintf(stderr, "\nPropVariantToCLSID: 0x%x\n", ret);
					continue;
				}

				ret = MFCap_StringFromGuid(&g, &guidString);
				if (FAILED(ret)) {
					fprintf(stderr, "\nMFCap_StringFromGuid: 0x%x\n", ret);
					continue;
				}

				fprintf(stdout, "%ls\n", guidString);
				MFCap_StringFree(guidString);
				break;
			default:
				ret = PropVariantChangeType(&tmp, Values[k], 0, VT_LPWSTR);
				if (FAILED(ret)) {
					fprintf(stderr, "\nPropVariantChangeType: 0x%x\n", ret);
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
HRESULT ProcessMediaFormats(PMFCAP_FORMAT Formats, DWORD FormatCount)
{
	HRESULT ret = S_OK;
	PMFCAP_FORMAT tmpFormat = NULL;

	tmpFormat = Formats;
	for (UINT32 k = 0; k < FormatCount; ++k) {
		PWCHAR typeGuidString = NULL;
		PWCHAR subtypeGuidString = NULL;
		GUID* guids;
		PROPVARIANT* values;
		UINT32 valueCount = 0;

		ret = MFCap_StringFromGuid(&tmpFormat->TypeGuid, &typeGuidString);
		if (FAILED(ret)) {
			fprintf(stderr, "MFCap_StringFromGuid: 0x%x\n", ret);
			continue;
		}

		ret = MFCap_StringFromGuid(&tmpFormat->SubtypeGuid, &subtypeGuidString);
		if (FAILED(ret)) {
			fprintf(stderr, "MFCap_StringFromGuid: 0x%x\n", ret);
			continue;
		}

		fprintf(stdout, "    Stream index: %u\n", tmpFormat->StreamIndex);
		fprintf(stdout, "    Index: %u\n", tmpFormat->Index);
		fprintf(stdout, "    Selected: %u\n", tmpFormat->Selected);
		fprintf(stdout, "    Type: %u\n", tmpFormat->Type);
		fprintf(stdout, "    Type GUID: %ls\n", typeGuidString);
		fprintf(stdout, "    Subtype GUID: %ls\n", subtypeGuidString);
		MFCap_StringFree(subtypeGuidString);
		MFCap_StringFree(typeGuidString);
		switch (tmpFormat->Type) {
			case mcftVideo:
				fprintf(stdout, "    Width: %u\n", tmpFormat->Video.Width);
				fprintf(stdout, "    Height: %u\n", tmpFormat->Video.Height);
				fprintf(stdout, "    Framerate: %u\n", tmpFormat->Video.Framerate);
				fprintf(stdout, "    Bitrate: %u\n", tmpFormat->Video.BitRate);
				break;
			case mcftAudio:
				fprintf(stdout, "    Channels: %u\n", tmpFormat->Audio.ChannelCount);
				fprintf(stdout, "    Bits per sample: %u\n", tmpFormat->Audio.BitsPerSample);
				fprintf(stdout, "    Samples per second: %u\n", tmpFormat->Audio.SamplesPerSecond);
				break;
		}

		ret = MFCap_GetFormatProperties(tmpFormat, &guids, &values, &valueCount);
		if (FAILED(ret)) {
			fprintf(stderr, "MFCap_GetFormatProperties: 0x%x\n", ret);
			continue;
		}

		ProcessProperties<mcftAudio>(guids, values, valueCount);
		MFCap_FreeProperties(guids, values, valueCount);
		fprintf(stdout, "\n");
		++tmpFormat;
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
				ret = ProcessMediaFormats<Type>(formats, formatCount);
				MFCap_FreeMediaTypes(formats, formatCount);
				MFCap_FreeInstance(d);
			}
		}
	}

	return ret;
}


HRESULT ProcessAudoOoutput(void)
{
	uint32_t count = 0;
	HRESULT ret = S_OK;
	PMFPLAY_DEVICE_INFO devices = NULL;
	PMFPLAY_DEVICE_INFO tmp = NULL;
	MFPLAY_DEVICE_STATE_MASK stateMask;
	PMFPLAY_DEVICE instance = NULL;
	PMFCAP_FORMAT formats = NULL;
	DWORD formatCount = 0;
	DWORD streamCount = 0;

	stateMask.Value = 0;
	stateMask.Active = 1;
	ret = MFPlay_EnumDevices(stateMask, &devices, &count);
	if (SUCCEEDED(ret)) {
		tmp = devices;
		for (uint32_t i = 0; i < count; ++i) {
			fprintf(stdout, "Output device #%u\n", i);
			fprintf(stdout, "  Name:\t%ls\n", tmp->Name);
			fprintf(stdout, "  Description:\t%ls\n", tmp->Description);
			fprintf(stdout, "  EndpointId:\t%ls\n", tmp->EndpointId);
			fprintf(stdout, "  State:\t0x%x\n", tmp->State);
			fprintf(stdout, "\n");
			++tmp;
			ret = MFPlay_NewInstance(tmp - 1, &instance);
			if (FAILED(ret)) {
				fprintf(stderr, "MFPlay_NewInstance: 0x%x", ret);
				continue;
			}
			
			ret = MFPlay_EnumFormats(instance, &formats, &formatCount, &streamCount);
			if (FAILED(ret)) {
				fprintf(stderr, "MFPlay_EnumFormats: 0x%x", ret);
				MFPlay_FreeInstance(instance);
				continue;
			}

			fprintf(stdout, "  Total number of formats: %u\n", formatCount);
			fprintf(stdout, "  Stream count: %u\n", streamCount);
			ProcessMediaFormats<mcftAudio>(formats, formatCount);
			MFCap_FreeMediaTypes(formats, formatCount);
			MFPlay_FreeInstance(instance);
		}

		MFPlay_FreeDeviceEnum(devices, count);
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

//	ret = ProcessDeviceType<mcftVideo>();
	if (FAILED(ret)) {
		fprintf(stderr, "ProcessDeviceType: 0x%x\n", ret);
		goto FInitMFCap;
	}

//	ret = ProcessDeviceType<mcftAudio>();
	if (FAILED(ret)) {
		fprintf(stderr, "ProcessDeviceType: 0x%x\n", ret);
		goto FInitMFCap;
	}

	ret = ProcessAudoOoutput();
	if (FAILED(ret)) {
		fprintf(stderr, "ProcessAUdoOoutput: 0x%x\n", ret);
		goto FInitMFCap;
	}

FInitMFCap:
	MFCap_Finit();
Exit:
	return ret;
}
