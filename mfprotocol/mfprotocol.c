
#include "mfcrypto.h"
#include "mfmemory.h"
#include "mfmessages.h"
#include "mfprotocol.h"



int MFProtocol_ServerInfoIn(const MFCRYPTO_SEED *ServerKeyChallenge, PMF_MESSAGE_HEADER* Message)
{
	int ret = 0;
	PMF_MESSAGE_HEADER tmpMessage = NULL;
	MF_MESSAGE_SERVER_INFO_IN data;

	memset(&data, 0, sizeof(data));
	if (ServerKeyChallenge != NULL)
		data.ServerKeyChallenge = *ServerKeyChallenge;
	else MFCrypto_RandomBuffer(data.ServerKeyChallenge.Seed, sizeof(data.ServerKeyChallenge.Seed));

	ret = MFMessage_Alloc(mfmtQueryServerInfo, NULL, NULL, &data, sizeof(MF_MESSAGE_SERVER_INFO_IN), Message);

	return ret;
}


int MFProtocol_ServerInfoOut(const char *ServerName, const MFCRYPTO_PUBLIC_KEY *ServerPublicKey, const MFCRYPTO_SIGNATURE *ServerKeyResponse, size_t UserCount, const MF_MESSAGE_USER_INFO *Users, size_t ChannelCount, const MF_MESSAGE_CHANNEL_INFO *Channels, PMF_MESSAGE_HEADER *Message)
{
	int ret = 0;
	size_t nameLen = 0;
	size_t dataSize = 0;
	PMF_MESSAGE_SERVER_INFO_OUT si = NULL;
	PMF_MESSAGE_USER_INFO ui = NULL;
	PMF_MESSAGE_CHANNEL_INFO chi = NULL;

	dataSize = sizeof(MF_MESSAGE_SERVER_INFO_OUT) + UserCount * sizeof(MF_MESSAGE_USER_INFO) + ChannelCount * sizeof(MF_MESSAGE_CHANNEL_INFO);
	ret = MFGen_RefMemAlloc(dataSize, (void **)&si);
	if (ret == 0) {
		memset(si, 0, dataSize);
		si->ServerKeyChallengeSignature = *ServerKeyResponse;
		si->ServerPublicKey = *ServerPublicKey;
		if (ServerName != NULL)
			nameLen = strlen(ServerName);

		if (nameLen >= sizeof(si->ServerName))
			nameLen = sizeof(si->ServerName) - sizeof(char);

		memcpy(si->ServerName, ServerName, nameLen);
		si->UserCount = (uint32_t)UserCount;
		ui = (PMF_MESSAGE_USER_INFO)(si + 1);
		memcpy(ui, Users, sizeof(MF_MESSAGE_USER_INFO)*si->UserCount);
		si->ChannelCount = (uint32_t)ChannelCount;
		chi = (PMF_MESSAGE_CHANNEL_INFO)(ui + si->UserCount);
		memcpy(chi, Channels, sizeof(MF_MESSAGE_CHANNEL_INFO)*si->ChannelCount);
		ret = MFMessage_Alloc(mfmtQueryServerInfo, NULL, NULL, si, dataSize, Message);
		MFGen_RefMemRelease(si);
	}

	return ret;
}


int MFProtocol_ErrorMessage(uint32_t ErrorCode, const char* ErrorMessage, PMF_MESSAGE_HEADER* Message)
{
	int ret = 0;
	size_t dataSize = 0;
	size_t nameLen = 0;
	PMF_MESSAGE_ERROR me = NULL;

	if (ErrorMessage != NULL)
		nameLen = (strlen(ErrorMessage) + 1) * sizeof(char);

	dataSize = sizeof(MF_MESSAGE_ERROR) + nameLen;
	ret = MFGen_RefMemAlloc(dataSize, (void **)&me);
	if (ret == 0) {
		memset(me, 0, dataSize);
		me->ErrorCode = ErrorCode;
		me->StringLen = (uint32_t)nameLen;
		memcpy(me + 1, ErrorMessage, nameLen);
		ret = MFMessage_Alloc(mfmtError, NULL, NULL, me, dataSize, Message);
		MFGen_RefMemRelease(me);
	}

	return ret;
}
