
#include <windows.h>
#include "mfgen.h"
#include "mfstream-impl.h"
#include "mfstream.h"








extern "C" HRESULT MFStream_NewInstance(const MFSTREAM_CALLBACKS* Callbacks, IMFByteStream** Stream)
{
	HRESULT ret = S_OK;
	CMFRWStream* s = NULL;

	s = new CMFRWStream(*Callbacks);
	if (s != NULL) {
		ret = s->getError();
		if (SUCCEEDED(ret)) {
			s->AddRef();
			*Stream = s;
		}

		s->Release();
	} else ret = E_OUTOFMEMORY;

	return ret;
}


extern "C" void MFStream_FreeInstance(IMFByteStream* Stream)
{
	MFGen_SafeRelease(Stream);

	return;
}
