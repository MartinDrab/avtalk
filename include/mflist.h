
#ifndef __MF_LIST_H__
#define __MF_LIST_H__



#include <stdint.h>


typedef struct _MF_LIST_ENTRY {
	struct _MF_LIST_ENTRY *Next;
	struct _MF_LIST_ENTRY *Prev;
} MF_LIST_ENTRY, * PMF_LIST_ENTRY;


#ifdef __cplusplus
extern "C" {
#endif

void MFList_Init(PMF_LIST_ENTRY ListHead);
int MFList_Empty(const MF_LIST_ENTRY* ListHead);
void MFList_InsertTail(PMF_LIST_ENTRY ListHead, PMF_LIST_ENTRY Entry);
void MFList_InsertHead(PMF_LIST_ENTRY ListHead, PMF_LIST_ENTRY Entry);
void MFList_Remove(PMF_LIST_ENTRY Entry);

#ifdef __cplusplus
}
#endif



#endif
