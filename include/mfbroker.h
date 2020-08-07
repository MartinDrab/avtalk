
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
	MF_MESSAGE_HEADER Header;
	// Data
} MF_LISTED_MESSAGE, *PMF_LISTED_MESSAGE;


struct _MF_BROKER;

typedef struct _MF_CONNECTION {
	MFCRYPTO_PUBLIC_KEY Key;
	SOCKET Socket;
	void *Context;
	struct _MF_BROKER* Broker;
	MF_LIST_ENTRY MessagesToSend;
	MF_LIST_ENTRY MessagesReceived;
} MF_CONNECTION, *PMF_CONNECTION;


typedef struct _MF_BROKER {
	MFCRYPTO_PUBLIC_KEY PublicKey;
	MFCRYPTO_SECRET_KEY SecretKey;
	SOCKET ListenSocket;
	PMF_CONNECTION Connections;
	int ConnectionCount;
	volatile BOOL Terminated;
} MF_BROKER, *PMF_BROKER;






#endif
