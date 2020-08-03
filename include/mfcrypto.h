
#ifndef __MFCRYPTO_H__
#define __MFCRYPTO_H__



#include <stdint.h>
#include "sodium.h"


typedef struct _MFCRYPTO_PUBLIC_KEY {
	unsigned char Enc[crypto_box_PUBLICKEYBYTES];
	unsigned char Sign[crypto_sign_PUBLICKEYBYTES];
} MFCRYPTO_PUBLIC_KEY, *PMFCRYPTO_PUBLIC_KEY;

typedef struct _MFCRYPTO_SECRET_KEY {
	unsigned char Enc[crypto_box_SECRETKEYBYTES];
	unsigned char Sign[crypto_sign_SECRETKEYBYTES];
} MFCRYPTO_SECRET_KEY, * PMFCRYPTO_SECRET_KEY;

typedef struct _MFCRYPTO_SIGNATURE {
	unsigned char Signature[crypto_sign_BYTES];
} MFCRYPTO_SIGNATURE, *PMFCRYPTO_SIGNATURE;

typedef struct _MFCRYPTO_ENC_HEADER {
	unsigned char Data[crypto_box_SEALBYTES];
} MFCRYPTO_ENC_HEADER, *PMFCRYPTO_ENC_HEADER;

typedef struct _MFCRYPTO_SEED {
	unsigned char Seed[crypto_box_SEEDBYTES];
} MFCRYPTO_SEED, *PMFCRYPTO_SEED;

typedef struct _MFCRYPTO_HASH_DIGEST {
	unsigned char Hash[crypto_hash_sha256_BYTES];
} MFCRYPTO_HASH_DIGEST, *PMFCRYPTO_HASH_DIGEST;

typedef struct _MFCRYPTO_SYMKEY {
	unsigned char Key[crypto_secretbox_KEYBYTES];
} MFCRYPTO_SYMKEY, *PMFCRYPTO_SYMKEY;

typedef struct _MFCRYPTO_SYMENC_HEADER {
	unsigned char Nonce[crypto_secretbox_NONCEBYTES];
	unsigned char MAC[crypto_secretbox_MACBYTES];
} MFCRYPTO_SYMENC_HEADER, *PMFCRYPTO_SYMENC_HEADER;

#ifdef __cplusplus
extern "C" {
#endif

void MFCrypto_KeyGen(PMFCRYPTO_PUBLIC_KEY PK, PMFCRYPTO_SECRET_KEY SK);
void MFCrypto_KeyGenSeed(PMFCRYPTO_PUBLIC_KEY PK, PMFCRYPTO_SECRET_KEY SK, const MFCRYPTO_SEED* Seed);
void MFCrypto_Encrypt(const MFCRYPTO_PUBLIC_KEY* PK, PMFCRYPTO_ENC_HEADER Ct, const void* Pt, size_t Len);
int MFCrypto_Decrypt(const MFCRYPTO_PUBLIC_KEY* PK, const MFCRYPTO_SECRET_KEY* SK, void* Pt, const MFCRYPTO_ENC_HEADER* Ct, size_t Len);
void MFCrypto_Sign(const MFCRYPTO_SECRET_KEY* SK, PMFCRYPTO_SIGNATURE Signature, const void* Buffer, size_t Len);
int MFCrypto_Verify(const MFCRYPTO_PUBLIC_KEY* PK, const MFCRYPTO_SIGNATURE* Signature, const void* Buffer, size_t Len);

void MFCrypto_SymKeyGen(PMFCRYPTO_SYMKEY Key);
void MFCrypto_SymKeyGenSeed(const MFCRYPTO_SEED* Seed, PMFCRYPTO_SYMKEY Key);
void MFCrypto_SymEncrypt(const MFCRYPTO_SYMKEY* Key, const void* Buffer, size_t Length, PMFCRYPTO_SYMENC_HEADER Ct);
int MFCrypto_SymDecrypt(const MFCRYPTO_SYMKEY* Key, const MFCRYPTO_SYMENC_HEADER *Buffer, size_t Length, void *Pt);

void MFCrypto_Hash(const void* Buffer, size_t Length, PMFCRYPTO_HASH_DIGEST Digest);


#ifdef __cplusplus
}
#endif



#endif
