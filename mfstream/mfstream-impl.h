
#ifndef __MF_STREAM_IMPL_H__
#define __MF_STREAM_IMPL_H__


#include <windows.h>
#include <winerror.h>
#include <Mfidl.h>
#include <mfapi.h>
#include "mfgen.h"


typedef enum _EMFRWStreamOperationType {
	mrwsUnknown,
	mrwsRead,
	mrwsWrite,
	mrwsMax,
} EMFRWStreamOperationType, *PEMFRWStreamOperationType;

typedef struct _MFRW_STREAM_OPERATION {
	LIST_ENTRY Entry;
	EMFRWStreamOperationType Type;
	DWORD Length;
	void* Buffer;
	IMFAsyncCallback* Callback;
	IUnknown* State;
	IMFAsyncResult* AsyncResult;
	HRESULT Result;
	DWORD BytesTransferred;
	HANDLE Event;
} MFRW_STREAM_OPERATION, *PMFRW_STREAM_OPERATION;

template <typename T>
class CMFSmartMemory : public IUnknown {
public:
	// IUnknown interface
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
	{
		HRESULT ret = E_NOINTERFACE;

		if (ppvObject != NULL) {
			if (riid == IID_IUnknown) {
				AddRef();
				*ppvObject = this;
				ret = S_OK;
			}
		}
		else ret = E_POINTER;

		return ret;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return (ULONG)InterlockedIncrement(&refCount_);
	}
	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		LONG tmp = InterlockedDecrement(&refCount_);

		if (tmp == 0)
			delete this;

		return tmp;
	}
	CMFSmartMemory(T* Mem)
		: mem_(Mem)
	{
		InterlockedExchange(&refCount_, 1);

		return;
	}
	~CMFSmartMemory()
	{
		if (mem_ != NULL)
			HeapFree(GetProcessHeap(), 0, mem_);

		return;
	}
	T *get(void) const { return mem_; }
	void reset(T* NewMem)
	{
		mem_ = NewMem;

		return;
	}
private:
	volatile LONG refCount_;
	T* mem_;
};


class CMFRWStream : public virtual IMFByteStream {
public:
	// IUnknown interface
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
	{
		HRESULT ret = E_NOINTERFACE;

		if (ppvObject != NULL) {
			if (riid == IID_IUnknown ||
				riid == IID_IMFByteStream) {
				AddRef();
				*ppvObject = this;
				ret = S_OK;
			}
		} else ret = E_POINTER;

		return ret;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return (ULONG)InterlockedIncrement(&refCount_);
	}
	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		LONG tmp = InterlockedDecrement(&refCount_);

		if (tmp == 0)
			delete this;

		return tmp;
	}
	// IMFByteStreamInterface
	virtual HRESULT STDMETHODCALLTYPE GetCapabilities(__RPC__out DWORD* pdwCapabilities)
	{
		DWORD caps = 0;
		HRESULT ret = S_OK;

		if (supportsRead_)
			caps |= MFBYTESTREAM_IS_READABLE;

		if (supportsWrite_)
			caps |= MFBYTESTREAM_IS_WRITABLE;


		*pdwCapabilities = caps;

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE GetLength(__RPC__out QWORD* pqwLength)
	{
		HRESULT ret = S_OK;

		*pqwLength = (QWORD)-1;

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE SetLength(QWORD qwLength)
	{
		HRESULT ret = S_OK;

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE GetCurrentPosition(__RPC__out QWORD* pqwPosition)
	{
		HRESULT ret = S_OK;

		*pqwPosition = 0;

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE SetCurrentPosition(QWORD qwPosition)
	{
		HRESULT ret = S_OK;

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE IsEndOfStream(__RPC__out BOOL* pfEndOfStream)
	{
		HRESULT ret = S_OK;

		*pfEndOfStream = TRUE;

		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE Read(__RPC__out_ecount_full(cb) BYTE* pb, ULONG cb, __RPC__out ULONG* pcbRead)
	{
		HRESULT ret = S_OK;
		PMFRW_STREAM_OPERATION r = NULL;

		ret = OpRecordAlloc(mrwsRead, (void*)pb, cb, NULL, NULL, &r);
		if (SUCCEEDED(ret)) {
			OpRecordInsert(r);
			OpRecordWait(r);
			if (pcbRead != NULL)
				*pcbRead = r->BytesTransferred;

			ret = r->Result;
			OpRecordFree(r);
		}

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE BeginRead(_Out_writes_bytes_(cb)  BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState)
	{
		HRESULT ret = S_OK;
		PMFRW_STREAM_OPERATION r = NULL;

		ret = OpRecordAlloc(mrwsRead, (void*)pb, cb, NULL, NULL, &r);
		if (SUCCEEDED(ret))
			OpRecordInsert(r);

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE EndRead(IMFAsyncResult* pResult, _Out_  ULONG* pcbRead)
	{
		HRESULT ret = S_OK;
		PMFRW_STREAM_OPERATION r = NULL;
		CMFSmartMemory<MFRW_STREAM_OPERATION> *sm = NULL;

		if (pcbRead != NULL)
			*pcbRead = 0;

		ret = pResult->GetObjectW((IUnknown **)&sm);
		if (SUCCEEDED(ret)) {
			r = sm->get();
			sm->reset(NULL);
			delete sm;
			OpRecordWait(r);
			if (pcbRead != NULL)
				*pcbRead = r->BytesTransferred;

			OpRecordFree(r);
		}

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE Write(__RPC__in_ecount_full(cb) const BYTE* pb, ULONG cb, __RPC__out ULONG* pcbWritten)
	{
		HRESULT ret = S_OK;
		PMFRW_STREAM_OPERATION r = NULL;

		ret = OpRecordAlloc(mrwsWrite, (void*)pb, cb, NULL, NULL, &r);
		if (SUCCEEDED(ret)) {
			OpRecordInsert(r);
			OpRecordWait(r);
			if (pcbWritten != NULL)
				*pcbWritten = r->BytesTransferred;

			ret = r->Result;
			OpRecordFree(r);
		}

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE BeginWrite(_In_reads_bytes_(cb)  const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState)
	{
		HRESULT ret = S_OK;
		PMFRW_STREAM_OPERATION r = NULL;

		ret = OpRecordAlloc(mrwsWrite, (void*)pb, cb, NULL, NULL, &r);
		if (SUCCEEDED(ret))
			OpRecordInsert(r);

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE EndWrite(IMFAsyncResult* pResult, _Out_  ULONG* pcbWritten)
	{
		HRESULT ret = S_OK;
		PMFRW_STREAM_OPERATION r = NULL;
		CMFSmartMemory<MFRW_STREAM_OPERATION>* sm = NULL;

		if (pcbWritten != NULL)
			*pcbWritten = 0;

		ret = pResult->GetObjectW((IUnknown**)&sm);
		if (SUCCEEDED(ret)) {
			r = sm->get();
			sm->reset(NULL);
			delete sm;
			OpRecordWait(r);
			if (pcbWritten != NULL)
				*pcbWritten = r->BytesTransferred;

			OpRecordFree(r);
		}

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD dwSeekFlags, __RPC__out QWORD* pqwCurrentPosition)
	{
		return E_INVALIDARG;
	}
	virtual HRESULT STDMETHODCALLTYPE Flush(void)
	{
		HRESULT ret = S_OK;

		return ret;
	}
	virtual HRESULT STDMETHODCALLTYPE Close(void)
	{
		HRESULT ret = S_OK;

		closed_ = true;
		ReleaseSemaphore(opListSemaphore_, 1, NULL);

		return ret;
	}

	CMFRWStream(bool ReadSupport, bool WriteSupport)
		: supportsRead_(ReadSupport), supportsWrite_(WriteSupport), closed_(false)
	{
		InitializeCriticalSection(&opListLock_);
		opListHead_.Blink = &opListHead_;
		opListHead_.Flink = &opListHead_;
		opListSemaphore_ = CreateSemaphoreW(NULL, 0, 0x7FFFFFFF, NULL);
		if (opListSemaphore_ == NULL) {
			errorCode_ = GetLastError();
			DeleteCriticalSection(&opListLock_);
			return;
		}

		opThread_ = CreateThread(NULL, 0, _StreamThreadROutine, this, 0, NULL);
		if (opThread_ == NULL) {
			errorCode_ = GetLastError();
			CloseHandle(opListSemaphore_);
			DeleteCriticalSection(&opListLock_);
			return;
		}

		WaitForSingleObject(opListSemaphore_, INFINITE);
		if (FAILED(errorCode_)) {
			WaitForSingleObject(opThread_, INFINITE);
			CloseHandle(opListSemaphore_);
			DeleteCriticalSection(&opListLock_);
			return;
		}

		InterlockedExchange(&refCount_, 1);
		errorCode_ = S_OK;

		return;
	}
	~CMFRWStream(void)
	{
		if (SUCCEEDED(errorCode_)) {
			closed_ = true;
			WaitForSingleObject(opThread_, INFINITE);
			CloseHandle(opThread_);
			CloseHandle(opListSemaphore_);
			DeleteCriticalSection(&opListLock_);
		}

		return;
	}
private:
	bool supportsRead_;
	bool supportsWrite_;
	volatile bool closed_;
	HRESULT errorCode_;
	volatile LONG refCount_;
	LIST_ENTRY opListHead_;
	CRITICAL_SECTION opListLock_;
	HANDLE opListSemaphore_;
	HANDLE opThread_;
	HRESULT OpRecordAlloc(EMFRWStreamOperationType Type, void *Buffer, ULONG Length, IMFAsyncCallback *Callback, IUnknown *State, PMFRW_STREAM_OPERATION* Record)
	{
		HRESULT ret = S_OK;
		PMFRW_STREAM_OPERATION tmpRecord = NULL;
		CMFSmartMemory<MFRW_STREAM_OPERATION>* sm = NULL;

		tmpRecord = (PMFRW_STREAM_OPERATION)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(MFRW_STREAM_OPERATION));
		if (tmpRecord != NULL) {
			tmpRecord->Entry.Flink = &tmpRecord->Entry;
			tmpRecord->Entry.Blink = &tmpRecord->Entry;
			tmpRecord->Type = Type;
			tmpRecord->Buffer = Buffer;
			tmpRecord->Length = Length;
			tmpRecord->Event = CreateEventW(NULL, TRUE, FALSE, NULL);
			if (tmpRecord->Event != NULL) {
				tmpRecord->Callback = Callback;
				tmpRecord->State = State;
				if (tmpRecord->Callback != NULL) {
					sm = new CMFSmartMemory<MFRW_STREAM_OPERATION>(tmpRecord);
					ret = MFCreateAsyncResult(sm, tmpRecord->Callback, tmpRecord->State, &tmpRecord->AsyncResult);
					if (SUCCEEDED(ret))
						tmpRecord->Callback->AddRef();
				}

				if (SUCCEEDED(ret)) {
					if (tmpRecord->State != NULL)
						tmpRecord->State->AddRef();

					*Record = tmpRecord;
				}

				if (FAILED(ret)) {
					if (sm != NULL) {
						sm->reset(NULL);
						delete sm;
					}

					CloseHandle(tmpRecord->Event);
				}
			} else ret = GetLastError() | 0x80070000;
			
			if (FAILED(ret))
				HeapFree(GetProcessHeap(), 0, tmpRecord);
		} else ret = E_OUTOFMEMORY;

		return ret;
	}
	void OpRecordInsert(PMFRW_STREAM_OPERATION Record)
	{
		EnterCriticalSection(&opListLock_);
		Record->Entry.Flink = &opListHead_;
		Record->Entry.Blink = opListHead_.Blink;
		opListHead_.Blink->Flink = &Record->Entry;
		opListHead_.Blink = &Record->Entry;
		LeaveCriticalSection(&opListLock_);
		ReleaseSemaphore(opListSemaphore_, 1, NULL);

		return;
	}
	void OpRecordWait(PMFRW_STREAM_OPERATION Record)
	{
		WaitForSingleObject(Record->Event, INFINITE);

		return;
	}
	void OpRecordFree(PMFRW_STREAM_OPERATION Record)
	{
		MFGen_SafeRelease(Record->AsyncResult);
		MFGen_SafeRelease(Record->State);
		MFGen_SafeRelease(Record->Callback);
		HeapFree(GetProcessHeap(), 0, Record);

		return;
	}
	static DWORD WINAPI _StreamThreadROutine(PVOID Context)
	{
		HRESULT ret = S_OK;
		CMFRWStream* s = (CMFRWStream*)Context;
		PMFRW_STREAM_OPERATION r = NULL;
		IMFAsyncResult* ar = NULL;

		ret = CoInitialize(NULL);
		s->errorCode_ = ret;
		ReleaseSemaphore(s->opListSemaphore_, 1, NULL);
		if (SUCCEEDED(ret)) {
			while (!s->closed_) {
				r = NULL;
				WaitForSingleObject(s->opListSemaphore_, INFINITE);
				EnterCriticalSection(&s->opListLock_);
				if (s->opListHead_.Flink != &s->opListHead_) {
					r = CONTAINING_RECORD(s->opListHead_.Flink, MFRW_STREAM_OPERATION, Entry);
					r->Entry.Flink->Blink = r->Entry.Blink;
					r->Entry.Blink->Flink = r->Entry.Flink;
				}

				LeaveCriticalSection(&s->opListLock_);
				if (r != NULL) {
					// TODO: Process the operation
					r->BytesTransferred = r->Length;
					r->Result = S_OK;
					SetEvent(r->Event);
					if (r->Callback != NULL) {
						ar = r->AsyncResult;
						ar->AddRef();
						ar->SetStatus(r->Result);
						r->Callback->Invoke(ar);
						ar->Release();
					}
				}
			}

			CoUninitialize();
		}

		return ret;
	}
};




#endif
