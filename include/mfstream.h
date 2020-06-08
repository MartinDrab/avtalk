
#ifndef __MF_STREAM_H__
#define __MF_STREAM_H__



#include <windows.h>
#include <Mfidl.h>




typedef HRESULT(MFSTREAM_READ_CALLBACK)(QWORD Position, void* Buffer, DWORD Length, PDWORD Read, void* Context);
typedef HRESULT(MFSTREAM_WRITE_CALLBACK)(QWORD Position, const void* Buffer, DWORD Length, PDWORD Written, void* Context);
typedef HRESULT(MFSTREAM_BYTES_AVAILABLE_CALLBACK)(PDWORD BytesAvailable, void* Context);

typedef struct _MFSTREAM_CALLBACKS {
	MFSTREAM_READ_CALLBACK* Read;
	void* ReadContext;
	MFSTREAM_WRITE_CALLBACK* Write;
	void* WriteContext;
	MFSTREAM_BYTES_AVAILABLE_CALLBACK* BytesAvailable;
	void* BytesAvailableContext;
} MFSTREAM_CALLBACKS, *PMFSTREAM_CALLBACKS;


HRESULT MFStream_NewInstance(const MFSTREAM_CALLBACKS* Callbacks, IMFByteStream** Stream);
void MFStream_FreeInstance(IMFByteStream* Stream);



#endif
