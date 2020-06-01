

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




typedef struct _GUID_STRING {
	GUID GUid;
	const wchar_t* String;
} GUID_STRING, * PGUID_STRING;


static GUID_STRING _guidMap[] = {
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
	{MF_MT_FRAME_RATE_RANGE_MIN, L"Min framerate"},
	{MF_MT_FRAME_RATE_RANGE_MAX, L"Max framerate"},
	{MF_MT_SAMPLE_SIZE, L"Sample size"},
	{MF_MT_DEFAULT_STRIDE, L"Default stride"},
	{MF_MT_FIXED_SIZE_SAMPLES, L"Fixed size samples"},
	{MF_MT_AM_FORMAT_TYPE, L"Format type"},
	//	EA031A62-8BBB-43C5-B5C4-572D2D231C18
	//	{MF_MT_FSSourceTypeDecoded, L"Decoded"},
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
		{MF_MT_ALL_SAMPLES_INDEPENDENT, L"Idependent"},
		{MF_MT_AUDIO_PREFER_WAVEFORMATEX, L"Prefer WaveEx"},
		// Audio formats
		{MFAudioFormat_Float, L"Float"},
		{MFAudioFormat_PCM, L"PCM"},
		{MFAudioFormat_Float_SpatialObjects, L"FloatSpatial"},
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


extern "C" HRESULT MFGen_StringFromGuid(GUID * Guid, PWCHAR * String)
{
	HRESULT hr = S_OK;
	wchar_t* tmp = NULL;

	for (size_t i = 0; i < sizeof(_guidMap) / sizeof(_guidMap[0]); ++i) {
		if (*Guid == _guidMap[i].GUid) {
			tmp = (wchar_t*)CoTaskMemAlloc((wcslen(_guidMap[i].String) + 1) * sizeof(wchar_t));
			if (tmp != NULL) {
				memcpy(tmp, _guidMap[i].String, (wcslen(_guidMap[i].String) + 1) * sizeof(wchar_t));
			}
			else hr = E_OUTOFMEMORY;

			break;
		}
	}

	if (SUCCEEDED(hr) && tmp == NULL)
		hr = StringFromIID(*Guid, &tmp);

	if (SUCCEEDED(hr))
		*String = tmp;

	return hr;
}


extern "C" void MFGen_StringFree(PWCHAR String)
{
	CoTaskMemFree(String);

	return;
}


extern "C" HRESULT MFGen_GetFormatProperties(MFGEN_FORMAT * Format, GUID * *Guids, PROPVARIANT * *Values, UINT32 * Count)
{
	HRESULT hr = S_OK;

	hr = MFGen_GetProperties(Format->MediaType, Guids, Values, Count);

	return hr;
}


extern "C" void MFGen_FreeProperties(GUID * Guids, PROPVARIANT * Values, UINT32 Count)
{
	if (Count > 0) {
		for (UINT32 i = 0; i < Count; ++i)
			PropVariantClear(Values + i);

		HeapFree(GetProcessHeap(), 0, Guids);
	}

	return;
}


extern "C" HRESULT MFGen_GetProperties(IMFAttributes* Attributes, GUID** Guids, PROPVARIANT** Values, UINT32* Count)
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
