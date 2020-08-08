
#ifndef __MF_THREAD_H__
#define __MF_THREAD_H__


#include <windows.h>




struct _MF_THREAD;

typedef int (MF_THREAD_ROUTINE)(struct _MF_THREAD *Thread);

typedef struct _MF_THREAD {
	MF_THREAD_ROUTINE *Routine;
	void *Context;
	int ExitCode;
	volatile int Terminated;
	volatile int Finished;
	HANDLE ThreadHandle;
	DWORD ThreadId;
} MF_THREAD, *PMF_THREAD;


#ifdef __cplusplus
extern "C" {
#endif

int MFThread_Create(MF_THREAD_ROUTINE *Routine, void *Context, PMF_THREAD *Thread);
void MFThread_Terminate(PMF_THREAD Thread);
void MFThread_WaitFor(PMF_THREAD Thread);
void MFThread_Close(PMF_THREAD Thread);
int MFThread_Terminated(const MF_THREAD *Thread);
int MFThread_ExitCode(const MF_THREAD *Thread, int* ExitCode);
void* MFThread_Context(const MF_THREAD *Thread);


#ifdef __cplusplus
}
#endif



#endif
