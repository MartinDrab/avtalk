
#include <windows.h>
#include "mflock.h"



int MFLock_Init(PMF_LOCK Lock)
{
	int ret = 0;

	if (!InitializeCriticalSectionAndSpinCount(&Lock->Lock, 0x1000))
		ret = GetLastError();

	return ret;
}


void MFLock_Enter(PMF_LOCK Lock)
{
	EnterCriticalSection(&Lock->Lock);

	return;
}


void MFLock_Leave(PMF_LOCK Lock)
{
	LeaveCriticalSection(&Lock->Lock);

	return;
}


void MFLock_Finit(PMF_LOCK Lock)
{
	DeleteCriticalSection(&Lock->Lock);

	return;
}
