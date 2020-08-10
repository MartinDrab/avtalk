
#include <windows.h>
#include "mfmemory.h"



int MFGen_RefMemAlloc(size_t NumberOfBytes, void** Buffer)
{
	HRESULT ret = S_OK;
	void* tmpBuffer = NULL;
	volatile LONG* refCount = NULL;

	tmpBuffer = CoTaskMemAlloc(NumberOfBytes + 0x10);
	if (tmpBuffer != NULL) {
		memset(tmpBuffer, 0, NumberOfBytes + 0x10);
		refCount = (LONG*)tmpBuffer;
		InterlockedExchange(refCount, 1);
		*Buffer = (unsigned char*)tmpBuffer + 0x10;
	} else ret = E_OUTOFMEMORY;

	return ret;
}


void MFGen_RefMemAddRef(void* Buffer)
{
	volatile LONG* refCount = NULL;

	refCount = (LONG*)((unsigned char*)Buffer - 0x10);
	InterlockedIncrement(refCount);

	return;
}


void MFGen_RefMemRelease(void* Buffer)
{
	volatile LONG* refCount = NULL;

	if (Buffer != NULL) {
		refCount = (LONG*)((unsigned char*)Buffer - 0x10);
		if (InterlockedDecrement(refCount) == 0)
			CoTaskMemFree((void*)refCount);
	}

	return;
}
