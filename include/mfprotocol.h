
#ifndef __MF_PROTOCOL_H__
#define __MF_PROTOCOL_H__



#include "mfcrypto.h"
#include "mfmessages.h"


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
} EMFMessageType, * PEMFMessageType;


typedef struct _MF_MESSAGE_SERVER_INFO_IN {
	MFCRYPTO_SEED ServerKeyChallenge;
} MF_MESSAGE_SERVER_INFO_IN, * PMF_MESSAGE_SERVER_INFO_IN;

typedef struct _MF_MESSAGE_USER_INFO {
	GUID Id;
	MFCRYPTO_PUBLIC_KEY Key;
	char Name[64];
} MF_MESSAGE_USER_INFO, * PMF_MESSAGE_USER_INFO;

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
} MF_MESSAGE_NEW_USER_IN, * PMF_MESSAGE_NEW_USER_IN;

typedef struct _MF_MESSAGE_NEW_USER_OUT {
	GUID Id;
	MFCRYPTO_SEED UserKeyChallenge;
} MF_MESSAGE_NEW_USER_OUT, * PMF_MESSAGE_NEW_USER_OUT;

typedef struct _MF_MESSAGE_USER_KEY_PROOF_IN {
	GUID Id;
	MFCRYPTO_SIGNATURE UserKeyChallengeSignature;
} MF_MESSAGE_USER_KEY_PROOF_IN, * PMF_MESSAGE_USER_KEY_PROOF_IN;

typedef struct _MF_MESSAGE_NEW_CHANNEL_IN {
	MFCRYPTO_SEED PasswordKeySeed1;
	MFCRYPTO_HASH_DIGEST PasswordHash;
	char Name[256];
	uint32_t Flags;
	uint32_t Padding;
} MF_MESSAGE_NEW_CHANNEL_IN, * PMF_MESSAGE_NEW_CHANNEL_IN;

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
} MF_MESSAGE_ERROR, * PMF_MESSAGE_ERROR;





#endif
