
#include <windows.h>
#include <winerror.h>
#include <mfidl.h>
#include <mfapi.h>
#include "mfstream-impl.h"



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

	return ret;
}


HRESULT STDMETHODCALLTYPE CMFRWStream::Close(void)
{
	HRESULT ret = S_OK;

	closed_ = true;
	ReleaseSemaphore(opListSemaphore_, 1, NULL);

	return ret;
}


CMFRWStream::CMFRWStream(bool ReadSupport, bool WriteSupport)
	: supportsRead_(ReadSupport), supportsWrite_(WriteSupport), closed_(false), errorCode_(S_OK), opThread_(NULL)
{
	InterlockedExchange(&refCount_, 1);
	InitializeCriticalSection(&opListLock_);
	opListHead_.Blink = &opListHead_;
	opListHead_.Flink = &opListHead_;
	opListSemaphore_ = CreateSemaphoreW(NULL, 0, 0x7FFFFFFF, NULL);
	if (opListSemaphore_ == NULL) {
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

	if (opListSemaphore_ != NULL)
		CloseHandle(opListSemaphore_);

	DeleteCriticalSection(&opListLock_);

	return;
}


void CMFRWStream::OpRecordInsert(CMFRWStreamOp* Record)
{
	EnterCriticalSection(&opListLock_);
	Record->Insert(&opListHead_);
	LeaveCriticalSection(&opListLock_);
	ReleaseSemaphore(opListSemaphore_, 1, NULL);

	return;
}


DWORD WINAPI CMFRWStream::_StreamThreadROutine(PVOID Context)
{
	HRESULT ret = S_OK;
	CMFRWStream* s = (CMFRWStream*)Context;
	CMFRWStreamOp* r = NULL;

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
				r->Finish(S_OK, r->getLength());
			}

			MFGen_SafeRelease(r);
		}

		CoUninitialize();
	}

	return ret;
}
