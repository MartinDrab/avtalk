
#ifndef __MF_STREAMOP_H__
#define __MF_STREAMOP_H__


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
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

	void Insert(PLIST_ENTRY Head);
	void Wait(void);
	static CMFRWStreamOp* RemoveFromList(PLIST_ENTRY Head);
	static HRESULT NewInstance(OpType Type, void* Buffer, ULONG Length, IMFAsyncCallback* Callback, IUnknown* State, CMFRWStreamOp** Record);
	~CMFRWStreamOp(void);
	DWORD getBytesTransferred(void) const { return bytesTransferred_; }
	HRESULT getResult(void) const { return result_; }
	void* getBuffer(void) const { return buffer_; }
	DWORD getLength(void) const { return length_; }
	void Finish(HRESULT Result, DWORD BytesTransferred);
private:
	CMFRWStreamOp();
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






#endif
