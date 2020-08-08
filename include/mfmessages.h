
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

typedef enum _EMFMessageType {
	mfmtPing,
	mfmtQueryServerInfo,
	mfmtNewUser,
	mfmtNewUserKeyProof,
	mfmtDeleteUser,
	mfmtNewChannel,
	mfmtEnterChannel,
	mfmtEnterChannelResponse,
	mfmtLeaveChannel,
	mfmtError,
	mfmtMax,
} EMFMessageType, *PEMFMessageType;

typedef struct _MF_MESSAGE_HEADER {
	uint32_t MessageSize;
	uint16_t Version;
	uint16_t Flags;
	MFCRYPTO_SIGNATURE MessageSignature;
	MFCRYPTO_ENC_HEADER HeaderEncryption;
	EMFMessageType MessageType;
	uint32_t DataSize;
	uint64_t PacketCounter;
	uint64_t Context;
	GUID Source;
	GUID Dest;
	MFCRYPTO_ENC_HEADER DataEncryption;
} MF_MESSAGE_HEADER, *PMF_MESSAGE_HEADER;

typedef struct _MF_MESSAGE_SERVER_INFO_IN {
	MFCRYPTO_SEED ServerKeyChallenge;
} MF_MESSAGE_SERVER_INFO_IN, *PMF_MESSAGE_SERVER_INFO_IN;

typedef struct _MF_MESSAGE_USER_INFO {
	GUID Id;
	MFCRYPTO_PUBLIC_KEY Key;
	char Name[64];
} MF_MESSAGE_USER_INFO, *PMF_MESSAGE_USER_INFO;

typedef struct _MF_MESSAGE_CHANNEL_INFO {
	GUID Id;
	char Name[64];
	uint32_t SeedCount;
	uint32_t Flags;
	MFCRYPTO_HASH_DIGEST PasswordHash;
	MFCRYPTO_SEED PasswordKeySeed2;
	// User randoms
} MF_MESSAGE_CHANNEL_INFO, * PMF_MESSAGE_CHANNEL_INFO;

typedef struct _MF_MESSAGE_SERVER_INFO_OUT {
	MFCRYPTO_PUBLIC_KEY ServerPublicKey;
	MFCRYPTO_SIGNATURE ServerKeyChallengeSignature;
	char ServerName[256];
	uint32_t Flags;
	uint32_t SupportedVersions;
	uint32_t UserCount;
	uint32_t ChannelCount;
	// Users
	// Channels
} MF_MESSAGE_SERVER_INFO_OUT, * PMF_MESSAGE_SERVER_INFO_OUT;

typedef struct _MF_MESSAGE_NEW_USER_IN {
	MFCRYPTO_PUBLIC_KEY PublicKey;
	uint64_t ExpirationDate;
	uint32_t Flags; // Visible server-wide, ...
} MF_MESSAGE_NEW_USER_IN, *PMF_MESSAGE_NEW_USER_IN;

typedef struct _MF_MESSAGE_NEW_USER_OUT {
	GUID Id;
	MFCRYPTO_SEED UserKeyChallenge;
} MF_MESSAGE_NEW_USER_OUT, * PMF_MESSAGE_NEW_USER_OUT;

typedef struct _MF_MESSAGE_USER_KEY_PROOF_IN {
	GUID Id;
	MFCRYPTO_SIGNATURE UserKeyChallengeSignature;
} MF_MESSAGE_USER_KEY_PROOF_IN, *PMF_MESSAGE_USER_KEY_PROOF_IN;

typedef struct _MF_MESSAGE_NEW_CHANNEL_IN {
	MFCRYPTO_SEED PasswordKeySeed1;
	MFCRYPTO_HASH_DIGEST PasswordHash;
	char Name[256];
	uint32_t Flags;
	uint32_t Padding;
} MF_MESSAGE_NEW_CHANNEL_IN, *PMF_MESSAGE_NEW_CHANNEL_IN;

typedef struct _MF_MESSAGE_NEW_CHANNEL_OUT {
	GUID Id;
	MFCRYPTO_SEED PasswordKeySeed2;
} MF_MESSAGE_NEW_CHANNEL_OUT, * PMF_MESSAGE_NEW_CHANNEL_OUT;

typedef struct _MF_MESSAGE_ENTER_CHANNEL_IN {
	GUID Id;
	MFCRYPTO_SEED PasswordKeySeed1;
} MF_MESSAGE_ENTER_CHANNEL_IN, * PMF_MESSAGE_ENTER_CHANNEL_IN;

typedef struct _MF_MESSAGE_ENTER_CHANNEL_OUT {
	GUID Id;
	MFCRYPTO_SEED Challenge; // Needs to be signed by channel's key
	MFCRYPTO_SEED PasswordKeySeed2;
	uint32_t SeedCount;
	uint32_t Padding;
	// MFCRYPTO_SEED seeds
} MF_MESSAGE_ENTER_CHANNEL_OUT, * PMF_MESSAGE_ENTER_CHANNEL_OUT;

typedef struct _MF_MESSAGE_ENTER_CHANNEL_RESPONSE_IN {
	GUID Id;
	MFCRYPTO_SYMENC_HEADER EncHeader;
	MFCRYPTO_SEED EncryptedChallenge;
} MF_MESSAGE_ENTER_CHANNEL_RESPONSEL_IN, * PMF_MESSAGE_ENTER_CHANNEL_RESPONSE_IN;

typedef struct _MF_MESSAGE_ERROR {
	uint32_t ErrorCode;
	uint32_t StringLen;
	// ANSI String
} MF_MESSAGE_ERROR, *PMF_MESSAGE_ERROR;


#ifdef __cplusplus
extern "C" {
#endif

int MFMessage_Alloc(EMFMessageType Type, const GUID* Source, const GUID* Dest, const void* Data, size_t Length, PMF_MESSAGE_HEADER *Message);
void MFMessage_Free(PMF_MESSAGE_HEADER Msg);
void MFMessage_DataEncrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* Key);
int MFMessage_DataDecrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* PK, const MFCRYPTO_SECRET_KEY* SK);
void MFMessage_HeaderEncrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* Key);
int MFMessage_HeaderDecrypt(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* PK, const MFCRYPTO_SECRET_KEY* SK);
void MFMessage_Sign(PMF_MESSAGE_HEADER Header, const MFCRYPTO_SECRET_KEY* SecretKey);
int MFMessage_Verify(PMF_MESSAGE_HEADER Header, const MFCRYPTO_PUBLIC_KEY* Key);
void MFMessage_SetContext(PMF_MESSAGE_HEADER Header, uint64_t Context);
void MFMessage_SetCounter(PMF_MESSAGE_HEADER Header, uint64_t Counter);


#ifdef __cplusplus
}
#endif


#endif