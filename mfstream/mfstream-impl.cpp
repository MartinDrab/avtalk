
#include <windows.h>
#include <initguid.h>
#include <winerror.h>
#include <mfidl.h>
#include <mfapi.h>
#include "mfstreamop-impl.h"
#include "mfstream-impl.h"


#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "mfuuid.lib")


HRESULT STDMETHODCALLTYPE CMFRWStream::QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
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


ULONG STDMETHODCALLTYPE CMFRWStream::AddRef(void)
{
	return (ULONG)InterlockedIncrement(&refCount_);
}


ULONG STDMETHODCALLTYPE CMFRWStream::Release(void)
{
	LONG tmp = InterlockedDecrement(&refCount_);

	if (tmp == 0)
		delete this;

	return tmp;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::GetCapabilities(__RPC__out DWORD* pdwCapabilities)
{
	DWORD caps = 0;
	HRESULT ret = S_OK;

	if (callbacks_.Read != NULL)
		caps |= MFBYTESTREAM_IS_READABLE;

	if (callbacks_.Write != NULL)
		caps |= MFBYTESTREAM_IS_WRITABLE;

	*pdwCapabilities = caps;

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::GetLength(__RPC__out QWORD* pqwLength)
{
	HRESULT ret = S_OK;

	*pqwLength = (QWORD)-1;

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::SetLength(QWORD qwLength)
{
	HRESULT ret = S_OK;

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::GetCurrentPosition(__RPC__out QWORD* pqwPosition)
{
	HRESULT ret = S_OK;

	*pqwPosition = 0;

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::SetCurrentPosition(QWORD qwPosition)
{
	HRESULT ret = S_OK;

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::IsEndOfStream(__RPC__out BOOL* pfEndOfStream)
{
	HRESULT ret = S_OK;

	*pfEndOfStream = TRUE;

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::Read(__RPC__out_ecount_full(cb) BYTE* pb, ULONG cb, __RPC__out ULONG* pcbRead)
{
	HRESULT ret = S_OK;
	CMFRWStreamOp* r = NULL;

	ret = CMFRWStreamOp::NewInstance(CMFRWStreamOp::OpType::mrwsRead, (void*)pb, cb, NULL, NULL, &r);
	if (SUCCEEDED(ret)) {
		OpRecordInsert(r);
		r->Wait();
		if (pcbRead != NULL)
			*pcbRead = r->getBytesTransferred();

		ret = r->getResult();
		r->Release();
	}

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::BeginRead(_Out_writes_bytes_(cb)  BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState)
{
	HRESULT ret = S_OK;
	CMFRWStreamOp* r = NULL;

	ret = CMFRWStreamOp::NewInstance(CMFRWStreamOp::OpType::mrwsRead, (void*)pb, cb, NULL, NULL, &r);
	if (SUCCEEDED(ret)) {
		OpRecordInsert(r);
		r->Release();
	}

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::EndRead(IMFAsyncResult* pResult, _Out_  ULONG* pcbRead)
{
	HRESULT ret = S_OK;
	CMFRWStreamOp* r = NULL;

	if (pcbRead != NULL)
		*pcbRead = 0;

	ret = pResult->GetObjectW((IUnknown**)&r);
	if (SUCCEEDED(ret)) {
		r->Wait();
		if (pcbRead != NULL)
			*pcbRead = r->getBytesTransferred();

		r->Release();
	}

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::Write(__RPC__in_ecount_full(cb) const BYTE* pb, ULONG cb, __RPC__out ULONG* pcbWritten)
{
	HRESULT ret = S_OK;
	CMFRWStreamOp* r = NULL;

	ret = CMFRWStreamOp::NewInstance(CMFRWStreamOp::OpType::mrwsWrite, (void*)pb, cb, NULL, NULL, &r);
	if (SUCCEEDED(ret)) {
		OpRecordInsert(r);
		r->Wait();
		if (pcbWritten != NULL)
			*pcbWritten = r->getBytesTransferred();

		ret = r->getResult();
		r->Release();
	}

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::BeginWrite(_In_reads_bytes_(cb)  const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState)
{
	HRESULT ret = S_OK;
	CMFRWStreamOp* r = NULL;

	ret = CMFRWStreamOp::NewInstance(CMFRWStreamOp::OpType::mrwsWrite, (void*)pb, cb, NULL, NULL, &r);
	if (SUCCEEDED(ret)) {
		OpRecordInsert(r);
		r->Release();
	}

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::EndWrite(IMFAsyncResult* pResult, _Out_  ULONG* pcbWritten)
{
	HRESULT ret = S_OK;
	CMFRWStreamOp* r = NULL;

	if (pcbWritten != NULL)
		*pcbWritten = 0;

	ret = pResult->GetObjectW((IUnknown**)&r);
	if (SUCCEEDED(ret)) {
		r->Wait();
		if (pcbWritten != NULL)
			*pcbWritten = r->getBytesTransferred();

		r->Release();
	}

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD dwSeekFlags, __RPC__out QWORD* pqwCurrentPosition)
{
	return E_INVALIDARG;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::Flush(void)
{
	HRESULT ret = S_OK;

	WaitForSingleObject(flushedEvent_, INFINITE);

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::Close(void)
{
	HRESULT ret = S_OK;

	closed_ = true;
	ReleaseSemaphore(opListSemaphore_, 1, NULL);

	return ret;
}


CMFRWStream::CMFRWStream(const MFSTREAM_CALLBACKS& Callbacks)
	: callbacks_(Callbacks), closed_(false), errorCode_(S_OK), opThread_(NULL)
{
	InterlockedExchange(&refCount_, 1);
	InterlockedExchange(&pendingOpCount_, 0);
	InitializeCriticalSection(&opListLock_);
	opListHead_.Blink = &opListHead_;
	opListHead_.Flink = &opListHead_;
	opListSemaphore_ = CreateSemaphoreW(NULL, 0, 0x7FFFFFFF, NULL);
	if (opListSemaphore_ == NULL) {
		errorCode_ = GetLastError();
		return;
	}

	flushedEvent_ = CreateEventW(NULL, TRUE, TRUE, NULL);
	if (flushedEvent_ == NULL) {
		errorCode_ = GetLastError();
		return;
	}

	opThread_ = CreateThread(NULL, 0, _StreamThreadROutine, this, 0, NULL);
	if (opThread_ == NULL) {
		errorCode_ = GetLastError();
		return;
	}

	WaitForSingleObject(opListSemaphore_, INFINITE);
	if (FAILED(errorCode_))
		return;

	return;
}


CMFRWStream::~CMFRWStream(void)
{
	if (opThread_ != NULL) {
		closed_ = true;
		WaitForSingleObject(opThread_, INFINITE);
		CloseHandle(opThread_);
	}

	if (flushedEvent_ != NULL)
		CloseHandle(flushedEvent_);

	if (opListSemaphore_ != NULL)
		CloseHandle(opListSemaphore_);

	DeleteCriticalSection(&opListLock_);

	return;
}


void CMFRWStream::OpRecordInsert(CMFRWStreamOp* Record)
{
	EnterCriticalSection(&opListLock_);
	Record->Insert(&opListHead_);
	if (InterlockedIncrement(&pendingOpCount_) == 1)
		ResetEvent(flushedEvent_);

	LeaveCriticalSection(&opListLock_);
	ReleaseSemaphore(opListSemaphore_, 1, NULL);

	return;
}


DWORD WINAPI CMFRWStream::_StreamThreadROutine(PVOID Context)
{
	HRESULT ret = S_OK;
	CMFRWStream* s = (CMFRWStream*)Context;
	CMFRWStreamOp* r = NULL;
	DWORD bytesTransferred = 0;

	ret = CoInitialize(NULL);
	s->errorCode_ = ret;
	ReleaseSemaphore(s->opListSemaphore_, 1, NULL);
	if (SUCCEEDED(ret)) {
		while (!s->closed_) {
			r = NULL;
			WaitForSingleObject(s->opListSemaphore_, INFINITE);
			EnterCriticalSection(&s->opListLock_);
			r = CMFRWStreamOp::RemoveFromList(&s->opListHead_);
			LeaveCriticalSection(&s->opListLock_);
			if (r != NULL) {
				// TODO: Process the operation
				switch (r->getType()) {
					case CMFRWStreamOp::OpType::mrwsRead:
						ret = s->callbacks_.Read(0, r->getBuffer(), r->getLength(), &bytesTransferred, s->callbacks_.ReadContext);
						break;
					case CMFRWStreamOp::OpType::mrwsWrite:
						ret = s->callbacks_.Write(0, r->getBuffer(), r->getLength(), &bytesTransferred, s->callbacks_.WriteContext);
						break;
				}

				r->Finish(ret, bytesTransferred);
				EnterCriticalSection(&s->opListLock_);
				if (InterlockedDecrement(&s->pendingOpCount_) == 0)
					SetEvent(s->flushedEvent_);

				LeaveCriticalSection(&s->opListLock_);
			}

			MFGen_SafeRelease(r);
		}

		CoUninitialize();
	}

	return ret;
}
