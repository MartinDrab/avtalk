
#ifndef __MF_STREAM_IMPL_H__
#define __MF_STREAM_IMPL_H__


#include <windows.h>
#include <winerror.h>
#include <Mfidl.h>
#include <mfapi.h>
#include "mfgen.h"
#include "mfstreamop-impl.h"
#include "mfstream.h"




class CMFRWStream : public virtual IMFByteStream {
public:
	// IUnknown interface
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);
	// IMFByteStreamInterface
	virtual HRESULT STDMETHODCALLTYPE GetCapabilities(__RPC__out DWORD* pdwCapabilities);
	virtual HRESULT STDMETHODCALLTYPE GetLength(__RPC__out QWORD* pqwLength);
	virtual HRESULT STDMETHODCALLTYPE SetLength(QWORD qwLength);
	virtual HRESULT STDMETHODCALLTYPE GetCurrentPosition(__RPC__out QWORD* pqwPosition);
	virtual HRESULT STDMETHODCALLTYPE SetCurrentPosition(QWORD qwPosition);
	virtual HRESULT STDMETHODCALLTYPE IsEndOfStream(__RPC__out BOOL* pfEndOfStream);

	virtual HRESULT STDMETHODCALLTYPE Read(__RPC__out_ecount_full(cb) BYTE* pb, ULONG cb, __RPC__out ULONG* pcbRead);
	virtual HRESULT STDMETHODCALLTYPE BeginRead(_Out_writes_bytes_(cb)  BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState);
	virtual HRESULT STDMETHODCALLTYPE EndRead(IMFAsyncResult* pResult, _Out_  ULONG* pcbRead);
	virtual HRESULT STDMETHODCALLTYPE Write(__RPC__in_ecount_full(cb) const BYTE* pb, ULONG cb, __RPC__out ULONG* pcbWritten);
	virtual HRESULT STDMETHODCALLTYPE BeginWrite(_In_reads_bytes_(cb)  const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState);
	virtual HRESULT STDMETHODCALLTYPE EndWrite(IMFAsyncResult* pResult, _Out_  ULONG* pcbWritten);
	virtual HRESULT STDMETHODCALLTYPE Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD dwSeekFlags, __RPC__out QWORD* pqwCurrentPosition);
	virtual HRESULT STDMETHODCALLTYPE Flush(void);
	virtual HRESULT STDMETHODCALLTYPE Close(void);

	CMFRWStream(const MFSTREAM_CALLBACKS & Callbacks);
	~CMFRWStream(void);
private:
	volatile bool closed_;
	volatile HRESULT errorCode_;
	volatile LONG refCount_;
	LIST_ENTRY opListHead_;
	CRITICAL_SECTION opListLock_;
	volatile LONG pendingOpCount_;
	HANDLE flushedEvent_;
	HANDLE opListSemaphore_;
	HANDLE opThread_;
	MFSTREAM_CALLBACKS callbacks_;
	void OpRecordInsert(CMFRWStreamOp* Record);
	static DWORD WINAPI _StreamThreadROutine(PVOID Context);
};




#endif
