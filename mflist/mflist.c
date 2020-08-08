
#include "mflist.h"



void MFList_Init(PMF_LIST_ENTRY ListHead)
{
	ListHead->Next = ListHead;
	ListHead->Prev = ListHead;

	return;
}


int MFList_Empty(const MF_LIST_ENTRY * ListHead)
{
	return (ListHead->Next == ListHead && ListHead->Prev == ListHead->Next);
}


void MFList_InsertTail(PMF_LIST_ENTRY ListHead, PMF_LIST_ENTRY Entry)
{
	Entry->Next = ListHead;
	Entry->Prev = ListHead->Prev;
	ListHead->Prev->Next = Entry;
	ListHead->Prev = Entry;

	return;
}


void MFList_InsertHead(PMF_LIST_ENTRY ListHead, PMF_LIST_ENTRY Entry)
{
	Entry->Prev = ListHead;
	Entry->Next = ListHead->Next;
	ListHead->Next->Prev = Entry;
	ListHead->Next = Entry;

	return;
}


void MFList_Remove(PMF_LIST_ENTRY Entry)
{
	Entry->Next->Prev = Entry->Prev;
	Entry->Prev->Next = Entry->Next;
	MFList_Init(Entry);

	return;
}
