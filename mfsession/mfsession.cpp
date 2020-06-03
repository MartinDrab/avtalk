
#include <windows.h>
#include "mfgen.h"
#include "mfcap.h"
#include "mfplay.h"
#include "mfsession.h"




extern "C" HRESULT MFSession_NewInstance(PMF_SESSION* Session)
{
	HRESULT ret = S_OK;
	PMF_SESSION tmpSession = NULL;
	IMFMediaSession* s = NULL;
	IMFTopology* t = NULL;

	ret = MFGen_RefMemAlloc(sizeof(MF_SESSION), (void **)&tmpSession);
	if (SUCCEEDED(ret))
		ret = MFCreateMediaSession(NULL, &s);
	
	if (SUCCEEDED(ret))
		ret = MFCreateTopology(&t);

	if (SUCCEEDED(ret)) {
		MFGen_RefMemAddRef(tmpSession);
		s->AddRef();
		tmpSession->Session = s;
		t->AddRef();
		tmpSession->Topology = t;
		*Session = tmpSession;
	}

	MFGen_SafeRelease(t);
	MFGen_SafeRelease(s);
	MFGen_RefMemRelease(tmpSession);

	return ret;
}


extern "C" void MFSession_FreeInstance(PMF_SESSION Session)
{
	MFGen_SafeRelease(Session->Topology);
	Session->Session->Shutdown();
	MFGen_SafeRelease(Session->Session);
	MFGen_RefMemRelease(Session);

	return;
}


HRESULT MFSession_Start(PMF_SESSION Session)
{
	HRESULT ret = S_OK;

	return ret;
}


HRESULT MFSession_Stop(PMF_SESSION Session)
{
	HRESULT ret = S_OK;

	return ret;
}


HRESULT MFSession_ConnectNodes(PMF_SESSION Session, PMFGEN_STREAM_INFO Source, PMFGEN_STREAM_INFO Target)
{
	HRESULT ret = S_OK;

	ret = Session->Topology->AddNode(Source->Node);
	if (SUCCEEDED(ret))
		ret = Session->Topology->AddNode(Target->Node);

	if (SUCCEEDED(ret))
		ret = Source->Node->ConnectOutput(0, Target->Node, 0);

	return ret;
}
