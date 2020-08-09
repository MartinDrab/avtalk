
#include <windows.h>
#include "mfmemory.h"
#include "mfthread.h"





static long _lastThreadIndex = -1;


static DWORD WINAPI _MFThreadWrapperRoutine(void* Context)
{
	DWORD ret = 0;
	PMF_THREAD t = (PMF_THREAD)Context;

	t->ExitCode = t->Routine(t);
	t->Finished = 1;
	ret = t->ExitCode;
	MFGen_RefMemRelease(t);

	return ret;
}



int MFThread_Create(MF_THREAD_ROUTINE* Routine, void* Context, PMF_THREAD* Thread)
{
	int ret = 0;
	PMF_THREAD tmpThread = NULL;

	ret = MFGen_RefMemAlloc(sizeof(MF_THREAD), (void **)&tmpThread);
	if (ret == 0) {
		memset(tmpThread, 0, sizeof(MF_THREAD));
		tmpThread->Routine = Routine;
		tmpThread->Context = Context;
		MFGen_RefMemAddRef(tmpThread);
		tmpThread->Index = (uint32_t)InterlockedIncrement(&_lastThreadIndex);
		tmpThread->ThreadHandle = CreateThread(NULL, 0, _MFThreadWrapperRoutine, tmpThread, 0, &tmpThread->ThreadId);
		if (tmpThread->ThreadHandle == NULL) {
			ret = GetLastError();
			MFGen_RefMemRelease(tmpThread);
		}
		
		if (ret == 0) {
			MFGen_RefMemAddRef(tmpThread);
			*Thread = tmpThread;
		}

		MFGen_RefMemRelease(tmpThread);
	}

	return ret;
}


void MFThread_Terminate(PMF_THREAD Thread)
{
	Thread->Terminated = 1;

	return;
}


void MFThread_WaitFor(PMF_THREAD Thread)
{
	WaitForSingleObject(Thread->ThreadHandle, INFINITE);

	return;
}


void MFThread_Close(PMF_THREAD Thread)
{
	CloseHandle(Thread->ThreadHandle);
	MFGen_RefMemRelease(Thread);

	return;
}


int MFThread_Terminated(const MF_THREAD *Thread)
{
	return (Thread->Terminated);
}


int MFThread_ExitCode(const MF_THREAD *Thread, int* ExitCode)
{
	int ret = 0;

	if (Thread->Finished) {
		if (ExitCode != NULL)
			*ExitCode = Thread->ExitCode;
	} else ret = -1;

	return ret;
}


void* MFThread_Context(const MF_THREAD *Thread)
{
	return Thread->Context;
}


uint32_t MFTHread_Index(const MF_THREAD *Thread)
{
	return Thread->Index;
}
