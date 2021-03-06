
#include <assert.h>
#include <windows.h>
#include "mfcrypto.h"
#include "mfmemory.h"
#include "mfmessages.h"





int MFMessage_Alloc(uint32_t Type, const GUID *Source, const GUID *Dest, const void* Data, size_t Length, PMF_MESSAGE_HEADER* Message)
{
	int ret = 0;
	uint32_t totalSize = 0;
	PMF_MESSAGE_HEADER tmpMsg = NULL;

	totalSize = sizeof(MF_MESSAGE_HEADER) + (uint32_t)Length;
	ret = MFGen_RefMemAlloc(totalSize, (void **)&tmpMsg);
	if (SUCCEEDED(ret)) {
		memset(tmpMsg, 0, totalSize);
		tmpMsg->Version = MF_PROTOCOL_VERSION;
		tmpMsg->MessageSize = totalSize;
		tmpMsg->MessageType = Type;
		tmpMsg->DataSize = (uint32_t)Length;
		memcpy(tmpMsg + 1, Data, Length);
		if (Source != NULL)
			tmpMsg->Source = *Source;

		if (Dest != NULL)
			tmpMsg->Dest = *Dest;
		
		*Message = tmpMsg;
	}

	return ret;
}


void MFMessage_Free(PMF_MESSAGE_HEADER Msg)
{
	MFGen_RefMemRelease(Msg);

	return;
}


void MFMessage_DataEncrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* Key)
{
	size_t remaining = 0;

	assert((Header->Flags & MF_MESSAGE_DATA_ENCRYPTED) == 0);
	assert((Header->Flags & MF_MESSAGE_HEADER_ENCRYPTED) == 0);
	MFCrypto_Encrypt(Key, &Header->DataEncryption, Header + 1, Header->DataSize);
	remaining = (Header->MessageSize - sizeof(MF_MESSAGE_HEADER) - Header->DataSize);
	MFCrypto_RandomBuffer((unsigned char *)(Header + 1) + Header->DataSize - remaining, remaining);
	Header->Flags |= MF_MESSAGE_DATA_ENCRYPTED;
	Header->Flags &= ~MF_MESSAGE_SIGNED;

	return;
}


int MFMessage_DataDecrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY *PK, const MFCRYPTO_SECRET_KEY *SK)
{
	int ret = 0;

	assert((Header->Flags & MF_MESSAGE_HEADER_ENCRYPTED) == 0);
	assert((Header->Flags & MF_MESSAGE_DATA_ENCRYPTED) != 0);
	ret = MFCrypto_Decrypt(PK, SK, Header + 1, &Header->DataEncryption, Header->DataSize + sizeof(Header->DataEncryption));
	if (ret == 0) {
		Header->Flags &= ~MF_MESSAGE_DATA_ENCRYPTED;
		Header->Flags &= ~MF_MESSAGE_SIGNED;
	}

	return ret;
}


void MFMessage_Sign(PMF_MESSAGE_HEADER Header, const MFCRYPTO_SECRET_KEY* SecretKey)
{
	MFCRYPTO_SIGNATURE s;

	Header->Flags |= MF_MESSAGE_SIGNED;
	memset(&Header->MessageSignature, 0, sizeof(Header->MessageSignature));
	MFCrypto_Sign(SecretKey, &s, Header, Header->MessageSize);
	Header->MessageSignature = s;

	return;
}


int MFMessage_Verify(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* Key)
{
	int ret = 0;
	MFCRYPTO_SIGNATURE s;

	assert((Header->Flags & MF_MESSAGE_SIGNED) != 0);
	s = Header->MessageSignature;
	memset(&Header->MessageSignature, 0, sizeof(Header->MessageSignature));
	ret = MFCrypto_Verify(Key, &s, Header, Header->MessageSize);
	Header->MessageSignature = s;

	return ret;
}


void MFMessage_HeaderEncrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* Key)
{
	assert((Header->Flags & MF_MESSAGE_HEADER_ENCRYPTED) == 0);
	MFCrypto_Encrypt(Key, &Header->HeaderEncryption, &Header->HeaderEncryption + 1, (unsigned char *)(Header + 1) - (unsigned char *)(&Header->HeaderEncryption + 1));
	Header->Flags |= MF_MESSAGE_HEADER_ENCRYPTED;
	Header->Flags &= ~MF_MESSAGE_SIGNED;

	return;
}


int MFMessage_HeaderDecrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* PK, const MFCRYPTO_SECRET_KEY* SK)
{
	int ret = 0;

	assert((Header->Flags & MF_MESSAGE_HEADER_ENCRYPTED) != 0);
	ret = MFCrypto_Decrypt(PK, SK, &Header->HeaderEncryption + 1, &Header->HeaderEncryption, (unsigned char*)(Header + 1) - (unsigned char*)&Header->HeaderEncryption);
	if (ret == 0) {
		Header->Flags &= ~MF_MESSAGE_HEADER_ENCRYPTED;
		Header->Flags &= ~MF_MESSAGE_SIGNED;
	}

	return ret;
}


void MFMessage_SetContext(PMF_MESSAGE_HEADER Header, uint64_t Context)
{
	Header->Context = Context;

	return;
}


void MFMessage_SetCounter(PMF_MESSAGE_HEADER Header, uint64_t Counter)
{
	Header->PacketCounter = Counter;

	return;
}


uint64_t MFMessage_GetContext(const MF_MESSAGE_HEADER* Header)
{
	return Header->Context;
}


uint64_t MFMessage_GetCounter(const MF_MESSAGE_HEADER* Header)
{
	return Header->PacketCounter;
}
