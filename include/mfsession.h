
#ifndef __MF_SESSION_H__
#define __MF_SESSION_H__


#include <windows.h>
#include "mfgen.h"
#include "mfcap.h"
#include "mfplay.h"



typedef struct _MF_SESSION {
	IMFMediaSession* Session;
	IMFTopology* Topology;
} MF_SESSION, *PMF_SESSION;



#ifdef __cplusplus
extern "C" {
#endif

HRESULT MFSession_NewInstance(PMF_SESSION* Session);
void MFSession_FreeInstance(PMF_SESSION Session);
HRESULT MFSession_Start(PMF_SESSION Session);
HRESULT MFSession_Stop(PMF_SESSION Session);
HRESULT MFSession_ConnectNodes(PMF_SESSION Session, PMFGEN_STREAM_INFO Source, PMFGEN_STREAM_INFO Target);

#ifdef __cplusplus
}
#endif


#endif
