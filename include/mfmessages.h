
#ifndef __MF_MESSAGES_H__
#define __MF_MESSAGES_H__


#include <windows.h>
#include "sodium.h"
#include "mfcrypto.h"


#define MF_PROTOCOL_VERSION					1

#define MF_MESSAGE_SIGNED					1
#define MF_MESSAGE_HEADER_ENCRYPTED			2
#define MF_MESSAGE_DATA_ENCRYPTED			4
#define MF_MESSAGE_DATA_COMPRESSED			8

typedef struct _MF_MESSAGE_HEADER {
	uint32_t MessageSize;
	uint16_t Version;
	uint16_t Flags;
	MFCRYPTO_SIGNATURE MessageSignature;
	MFCRYPTO_ENC_HEADER HeaderEncryption;
	uint32_t MessageType;
	uint32_t DataSize;
	uint64_t PacketCounter;
	uint64_t Context;
	GUID Source;
	GUID Dest;
	MFCRYPTO_ENC_HEADER DataEncryption;
} MF_MESSAGE_HEADER, *PMF_MESSAGE_HEADER;


#ifdef __cplusplus
extern "C" {
#endif

int MFMessage_Alloc(uint32_t Type, const GUID* Source, const GUID* Dest, const void* Data, size_t Length, PMF_MESSAGE_HEADER *Message);
void MFMessage_Free(PMF_MESSAGE_HEADER Msg);
void MFMessage_DataEncrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* Key);
int MFMessage_DataDecrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* PK, const MFCRYPTO_SECRET_KEY* SK);
void MFMessage_HeaderEncrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* Key);
int MFMessage_HeaderDecrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* PK, const MFCRYPTO_SECRET_KEY* SK);
void MFMessage_Sign(PMF_MESSAGE_HEADER Header, const MFCRYPTO_SECRET_KEY* SecretKey);
int MFMessage_Verify(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* Key);
void MFMessage_SetContext(PMF_MESSAGE_HEADER Header, uint64_t Context);
void MFMessage_SetCounter(PMF_MESSAGE_HEADER Header, uint64_t Counter);
uint64_t MFMessage_GetContext(const MF_MESSAGE_HEADER* Header);
uint64_t MFMessage_GetCounter(const MF_MESSAGE_HEADER* Header);


#ifdef __cplusplus
}
#endif


#endif
