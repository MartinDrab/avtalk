
#ifndef __MFGEN_H__
#define __MFGEN_H__

#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>


typedef enum _EMFCapFormatType {
	mcftUnknown,
	mcftVideo,
	mcftAudio,
	mcftMax,
} EMFCapFormatType, *PEMFCapFormatType;

typedef struct _MFGEN_FORMAT {
	GUID TypeGuid;
	GUID SubtypeGuid;
	IMFMediaType* MediaType;
	UINT32 StreamIndex;
	UINT32 Index;
	BOOL Selected;
	wchar_t FriendlyName[8];
	EMFCapFormatType Type;
	union {
		struct {
			UINT32 Width;
			UINT32 Height;
			UINT32 BitRate;
			UINT32 Framerate;
		} Video;
		struct {
			UINT32 ChannelCount;
			UINT32 SamplesPerSecond;
			UINT32 BitsPerSample;
		} Audio;
	};
} MFGEN_FORMAT, * PMFGEN_FORMAT;


typedef struct _MFGEN_STREAM_INFO {
	GUID MajorType;
	EMFCapFormatType Type;
	DWORD Index;
	DWORD Id;
	BOOL Selected;
	IMFTopologyNode* Node;
} MFGEN_STREAM_INFO, *PMFGEN_STREAM_INFO;

typedef struct _MF_LIST_ENTRY {
	struct _MF_LIST_ENTRY *Next;
	struct _MF_LIST_ENTRY *Prev;
} MF_LIST_ENTRY, *PMF_LIST_ENTRY;


#ifdef __cplusplus
extern "C" {
#endif

HRESULT MFGen_StringFromGuid(GUID * Guid, PWCHAR * String);
void MFGen_StringFree(PWCHAR String);
HRESULT MFGen_GetFormatProperties(MFGEN_FORMAT* Format, GUID** Guids, PROPVARIANT** Values, UINT32* Count);
HRESULT MFGen_GetProperties(IMFAttributes * Attributes, GUID * *Guids, PROPVARIANT * *Values, UINT32 * Count);
void MFGen_FreeProperties(GUID * Guids, PROPVARIANT * Values, UINT32 Count);
void MFGen_SafeRelease(IUnknown * Object);
HRESULT MFGen_MediaTypeToFormat(IMFMediaType* MediaType, PMFGEN_FORMAT Format);
void MFGen_FreeFormats(PMFGEN_FORMAT Formats, UINT32 Count);
void MFGen_FreeStreamNodes(PMFGEN_STREAM_INFO Nodes, UINT32 Count);

HRESULT MFGen_RefMemAlloc(size_t NumberOfBytes, void** Buffer);
void MFGen_RefMemAddRef(void* Buffer);
void MFGen_RefMemRelease(void* Buffer);

void MFList_Init(PMF_LIST_ENTRY ListHead);
int MFList_Empty(const MF_LIST_ENTRY *ListHead);
void MFList_InsertTail(PMF_LIST_ENTRY ListHead, PMF_LIST_ENTRY Entry);
void MFList_InsertHead(PMF_LIST_ENTRY ListHead, PMF_LIST_ENTRY Entry);
void MFList_Remove(PMF_LIST_ENTRY Entry);

HRESULT MFGen_Init(void);
void MFGen_Finit(void);

#ifdef __cplusplus
}
#endif



#endif
