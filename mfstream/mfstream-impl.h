
#ifndef __MF_STREAM_IMPL_H__
#define __MF_STREAM_IMPL_H__


#include <windows.h>
#include <winerror.h>
#include <Mfidl.h>
#include <mfapi.h>
#include "mfgen.h"



class CMFRWStreamOp : public IUnknown
{
public:
	enum OpType {
		mrwsUnknown,
		mrwsRead,
		mrwsWrite,
		mrwsMax,
	};

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

	void Insert(PLIST_ENTRY Head)
	{
		entry_.Flink = Head;
		entry_.Blink = Head->Blink;
		Head->Blink->Flink = &entry_;
		Head->Blink = &entry_;
		AddRef();

		return;
	}
	void Wait(void)
	{
		WaitForSingleObject(event_, INFINITE);

		return;
	}
	static CMFRWStreamOp* RemoveFromList(PLIST_ENTRY Head)
	{
		CMFRWStreamOp* ret = NULL;

		if (Head->Flink != Head) {
			ret = CONTAINING_RECORD(Head->Flink, CMFRWStreamOp, entry_);
			ret->entry_.Flink->Blink = ret->entry_.Blink;
			ret->entry_.Blink->Flink = ret->entry_.Flink;
		}

		return ret;
	}
	static HRESULT NewInstance(OpType Type, void* Buffer, ULONG Length, IMFAsyncCallback* Callback, IUnknown* State, CMFRWStreamOp** Record)
	{
		HRESULT ret = S_OK;
		CMFRWStreamOp *tmpRecord = NULL;

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
	~CMFRWStreamOp(void)
	{
		MFGen_SafeRelease(asyncResult_);
		MFGen_SafeRelease(state_);
		MFGen_SafeRelease(callback_);
		if (event_ != NULL)
			CloseHandle(event_);

		return;
	}
	DWORD getBytesTransferred(void) const { return bytesTransferred_; }
	HRESULT getResult(void) const { return result_; }
	void* getBuffer(void) const { return buffer_; }
	DWORD getLength(void) const { return length_; }
	void Finish(HRESULT Result, DWORD BytesTransferred)
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
private:
	CMFRWStreamOp()
		: asyncResult_(NULL), callback_(NULL), state_(NULL), event_(NULL)
	{
		entry_.Flink = &entry_;
		entry_.Blink = &entry_;
		InterlockedExchange(&refCount_, 1);

		return;
	}
	CMFRWStreamOp(const CMFRWStreamOp&) = delete;
	CMFRWStreamOp(CMFRWStreamOp&&) = delete;
	CMFRWStreamOp& operator = (const CMFRWStreamOp&) = delete;
	CMFRWStreamOp& operator = (CMFRWStreamOp&&) = delete;
	volatile LONG refCount_;
	LIST_ENTRY entry_;
	OpType type_;
	DWORD length_;
	void* buffer_;
	IMFAsyncCallback* callback_;
	IUnknown* state_;
	IMFAsyncResult* asyncResult_;
	HRESULT result_;
	DWORD bytesTransferred_;
	HANDLE event_;
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

	virtual HRESULT STDMETHODCALLTYPE Read(__RPC__out_ecount_full(cb) BYTE* pb, ULONG cb, __RPC__out ULONG* pcbRead);
	virtual HRESULT STDMETHODCALLTYPE BeginRead(_Out_writes_bytes_(cb)  BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState);
	virtual HRESULT STDMETHODCALLTYPE EndRead(IMFAsyncResult* pResult, _Out_  ULONG* pcbRead);
	virtual HRESULT STDMETHODCALLTYPE Write(__RPC__in_ecount_full(cb) const BYTE* pb, ULONG cb, __RPC__out ULONG* pcbWritten);
	virtual HRESULT STDMETHODCALLTYPE BeginWrite(_In_reads_bytes_(cb)  const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState);
	virtual HRESULT STDMETHODCALLTYPE EndWrite(IMFAsyncResult* pResult, _Out_  ULONG* pcbWritten);
	virtual HRESULT STDMETHODCALLTYPE Seek(MFBYTESTREAM_SEEK_ORIGIN SeekOrigin, LONGLONG llSeekOffset, DWORD dwSeekFlags, __RPC__out QWORD* pqwCurrentPosition);
	virtual HRESULT STDMETHODCALLTYPE Flush(void);
	virtual HRESULT STDMETHODCALLTYPE Close(void);

	CMFRWStream(bool ReadSupport, bool WriteSupport);
	~CMFRWStream(void);
private:
	bool supportsRead_;
	bool supportsWrite_;
	volatile bool closed_;
	volatile HRESULT errorCode_;
	volatile LONG refCount_;
	LIST_ENTRY opListHead_;
	CRITICAL_SECTION opListLock_;
	HANDLE opListSemaphore_;
	HANDLE opThread_;
	void OpRecordInsert(CMFRWStreamOp* Record);
	static DWORD WINAPI _StreamThreadROutine(PVOID Context);
};




#endif
