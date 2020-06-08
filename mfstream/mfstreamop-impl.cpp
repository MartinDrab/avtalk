

#include <windows.h>
#include <winerror.h>
#include <Mfidl.h>
#include <mfapi.h>
#include "mfgen.h"
#include "mfstreamop-impl.h"



HRESULT STDMETHODCALLTYPE CMFRWStreamOp::QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
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


ULONG STDMETHODCALLTYPE CMFRWStreamOp::AddRef(void)
{
	return (ULONG)InterlockedIncrement(&refCount_);
}


ULONG STDMETHODCALLTYPE CMFRWStreamOp::Release(void)
{
	LONG tmp = InterlockedDecrement(&refCount_);

	if (tmp == 0)
		delete this;

	return tmp;
}


void CMFRWStreamOp::Insert(PLIST_ENTRY Head)
{
	entry_.Flink = Head;
	entry_.Blink = Head->Blink;
	Head->Blink->Flink = &entry_;
	Head->Blink = &entry_;
	AddRef();

	return;
}


void CMFRWStreamOp::Wait(void)
{
	WaitForSingleObject(event_, INFINITE);

	return;
}


CMFRWStreamOp* CMFRWStreamOp::RemoveFromList(PLIST_ENTRY Head)
{
	CMFRWStreamOp* ret = NULL;

	if (Head->Flink != Head) {
		ret = CONTAINING_RECORD(Head->Flink, CMFRWStreamOp, entry_);
		ret->entry_.Flink->Blink = ret->entry_.Blink;
		ret->entry_.Blink->Flink = ret->entry_.Flink;
	}

	return ret;
}


HRESULT CMFRWStreamOp::NewInstance(OpType Type, void* Buffer, ULONG Length, IMFAsyncCallback* Callback, IUnknown* State, CMFRWStreamOp** Record)
{
	HRESULT ret = S_OK;
	CMFRWStreamOp* tmpRecord = NULL;

	tmpRecord = new CMFRWStreamOp;
	if (tmpRecord != NULL) {
		tmpRecord->type_ = Type;
		tmpRecord->buffer_ = Buffer;
		tmpRecord->length_ = Length;
		tmpRecord->event_ = CreateEventW(NULL, TRUE, FALSE, NULL);
		if (tmpRecord->event_ != NULL) {
			tmpRecord->callback_ = Callback;
			tmpRecord->state_ = State;
			if (tmpRecord->callback_ != NULL) {
				ret = MFCreateAsyncResult(tmpRecord, tmpRecord->callback_, tmpRecord->state_, &tmpRecord->asyncResult_);
				if (SUCCEEDED(ret))
					tmpRecord->callback_->AddRef();
			}

			if (SUCCEEDED(ret)) {
				if (tmpRecord->state_ != NULL)
					tmpRecord->state_->AddRef();

				tmpRecord->AddRef();
				*Record = tmpRecord;
			}
		} else ret = GetLastError() | 0x80070000;

		tmpRecord->Release();
	} else ret = E_OUTOFMEMORY;

	return ret;
}


CMFRWStreamOp::~CMFRWStreamOp(void)
{
	MFGen_SafeRelease(asyncResult_);
	MFGen_SafeRelease(state_);
	MFGen_SafeRelease(callback_);
	if (event_ != NULL)
		CloseHandle(event_);

	return;
}


void CMFRWStreamOp::Finish(HRESULT Result, DWORD BytesTransferred)
{
	bytesTransferred_ = BytesTransferred;
	result_ = Result;
	SetEvent(event_);
	if (callback_ != NULL) {
		asyncResult_->SetStatus(result_);
		callback_->Invoke(asyncResult_);
	}

	return;
}


CMFRWStreamOp::CMFRWStreamOp()
	: asyncResult_(NULL), callback_(NULL), state_(NULL), event_(NULL)
{
	entry_.Flink = &entry_;
	entry_.Blink = &entry_;
	InterlockedExchange(&refCount_, 1);

	return;
}
