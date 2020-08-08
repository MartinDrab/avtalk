
#ifndef __MF_LOCK_H__
#define __MF_LOCK_H__



#include <windows.h>


typedef struct _MF_LOCK {
	CRITICAL_SECTION Lock;
} MF_LOCK, *PMF_LOCK;


#ifdef __cplusplus
extern "C" {
#endif

int MFLock_Init(PMF_LOCK Lock);
void MFLock_Enter(PMF_LOCK Lock);
void MFLock_Leave(PMF_LOCK Lock);
void MFLock_Finit(PMF_LOCK Lock);

#ifdef __cplusplus
}
#endif



#endif
