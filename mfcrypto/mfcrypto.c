
#include <stdint.h>
#include <assert.h>
#include "sodium.h"
#include "mfcrypto.h"





/************************************************************************/
/*                    PUBLIC FUNCTIONS                            */
/************************************************************************/


void MFCrypto_KeyGen(PMFCRYPTO_PUBLIC_KEY PK, PMFCRYPTO_SECRET_KEY SK)
{
	crypto_box_keypair(PK->Enc, SK->Enc);
	crypto_sign_keypair(PK->Sign, SK->Sign);

	return;
}


void MFCrypto_KeyGenSeed(PMFCRYPTO_PUBLIC_KEY PK, PMFCRYPTO_SECRET_KEY SK, const MFCRYPTO_SEED *Seed)
{
	crypto_box_seed_keypair(PK->Enc, SK->Enc, Seed->Seed);
	crypto_sign_seed_keypair(PK->Sign, SK->Sign, Seed->Seed);

	return;
}


void MFCrypto_Encrypt(const MFCRYPTO_PUBLIC_KEY *PK, PMFCRYPTO_ENC_HEADER Ct, const void *Pt, size_t Len)
{
	int ret = 0;

	ret = crypto_box_seal(Ct->Data, (const unsigned char *)Pt, Len, PK->Enc);
	assert(ret == 0);

	return;
}


int MFCrypto_Decrypt(const MFCRYPTO_PUBLIC_KEY *PK, const MFCRYPTO_SECRET_KEY *SK, void *Pt, const MFCRYPTO_ENC_HEADER *Ct, size_t Len)
{

	return crypto_box_seal_open((unsigned char *)Pt, Ct->Data, Len, PK->Enc, SK->Enc);

}


void MFCrypto_Sign(const MFCRYPTO_SECRET_KEY *SK, PMFCRYPTO_SIGNATURE Signature, const void *Buffer, size_t Len)
{
	int ret = 0;
	unsigned long long signLen = crypto_sign_BYTES;

	ret = crypto_sign_detached(Signature->Signature, &signLen, (const unsigned char *)Buffer, Len, SK->Sign);
	assert(ret == 0);

	return;
}


int MFCrypto_Verify(const MFCRYPTO_PUBLIC_KEY *PK, const MFCRYPTO_SIGNATURE *Signature, const void *Buffer, size_t Len)
{
	int ret = 0;

	ret = crypto_sign_verify_detached(Signature->Signature, (const unsigned char *)Buffer, Len, PK->Sign);

	return ret;
}


void MFCrypto_SymKeyGen(PMFCRYPTO_SYMKEY Key)
{
	crypto_secretbox_keygen(Key->Key);

	return;
}


void MFCrypto_SymKeyGenSeed(const MFCRYPTO_SEED* Seed, PMFCRYPTO_SYMKEY Key)
{
	randombytes_buf_deterministic(Key->Key, sizeof(Key->Key), Seed->Seed);

	return;
}


void MFCrypto_SymEncrypt(const MFCRYPTO_SYMKEY* Key, const void* Buffer, size_t Length, PMFCRYPTO_SYMENC_HEADER Ct)
{
	randombytes_buf(Ct->Nonce, sizeof(Ct->Nonce));
	crypto_secretbox_easy(Ct->MAC, Buffer, Length, Ct->Nonce, Key->Key);

	return;
}


int MFCrypto_SymDecrypt(const MFCRYPTO_SYMKEY* Key, const MFCRYPTO_SYMENC_HEADER* Buffer, size_t Length, void* Pt)
{
	return crypto_secretbox_open_easy(Pt, Buffer->MAC, Length - sizeof(Buffer->Nonce), Buffer->Nonce, Key->Key);
}
