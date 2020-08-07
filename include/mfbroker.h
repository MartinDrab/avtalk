
#ifndef __MF_BROKER_H__
#define __MF_BROKER_H__


#include <list>
#include <winsock2.h>
#include <windows.h>
#include "mfgen.h"
#include "mfcrypto.h"
#include "mfmessages.h"



typedef struct _MF_LISTED_MESSAGE {
	MF_LIST_ENTRY Entry;
	uint32_t OriginalFlags;
	uint32_t Padding;
	MF_MESSAGE_HEADER Header;
	// Data
} MF_LISTED_MESSAGE, *PMF_LISTED_MESSAGE;


typedef enum _EMFConnectionState {
	mcsFree,
	mcsHandshake,
	mcfEstablished,
} EMFConnectionState, *PEMFConnectionState;

struct _MF_BROKER;

typedef struct _MF_CONNECTION {
	MFCRYPTO_PUBLIC_KEY Key;
	SOCKET Socket;
	EMFConnectionState State;
	void *Context;
	struct _MF_BROKER* Broker;
	MF_LIST_ENTRY MessagesToSend;
	CRITICAL_SECTION SendLock;
} MF_CONNECTION, *PMF_CONNECTION;

typedef enum _EBrokerErrorType {
	betMessageFailedReceive,
	betMessageFailedSend,
	betMessageInvalidSignature,
	betMessageFailedDecryption,
	betFailedPoll,
	betSocketError,
	betMemoryAllocation,
	betAcceptFailed,
	betTooManyConnections,
} EBrokerErrorType, *PEBrokerErrorType;

typedef enum _EBrokerMessageEventType {
	bmetReady,
	bmetAboutToEncrypt,
	bmetAboutToSign,
	bmetSent,
} EBrokerMessageEventType, *PEBrokerMessageEventType;

typedef int (MF_BROKER_ERROR_CALLBACK)(EBrokerErrorType Type, int Code, void *Data, void *Context);
typedef int (MF_BROKER_MESSAGE_CALLBACK)(EBrokerMessageEventType Type, const MF_MESSAGE_HEADER* Header, const void* Data, size_t DataLength, uint32_t OriginalFlags, void* Context);;

typedef struct _MF_BROKER {
	MFCRYPTO_PUBLIC_KEY PublicKey;
	MFCRYPTO_SECRET_KEY SecretKey;
	SOCKET ListenSocket;
	PMF_CONNECTION Connections;
	int ConnectionCount;
	volatile BOOL Terminated;
	HANDLE ThreadHandle;
	DWORD ThreadId;
	MF_BROKER_ERROR_CALLBACK *ErrorCallback;
	void* ErrorCallbackContext;
	MF_BROKER_MESSAGE_CALLBACK *MessageCallback;
	void* MessageCallbackContext;
} MF_BROKER, *PMF_BROKER;






#endif
