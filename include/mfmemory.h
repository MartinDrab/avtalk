
#ifndef __MF_MEMORY_H__
#define __MF_MEMORY_H__



#ifdef __cplusplus
extern "C" {
#endif

HRESULT MFGen_RefMemAlloc(size_t NumberOfBytes, void** Buffer);
void MFGen_RefMemAddRef(void* Buffer);
void MFGen_RefMemRelease(void* Buffer);

#ifdef __cplusplus
}
#endif



#endif
