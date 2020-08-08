
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "mfgen.h"
#include "mfcrypto.h"
#include "mfmessages.h"
#include "mfbroker.h"




static int _maxConnections = 64;




static int _sendData(SOCKET Socket, const void* Data, size_t Length)
{
	int ret = 0;
	int bytesSent = 0;

	bytesSent = send(Socket, (char*)Data, (int)Length, 0);
	if (bytesSent != Length)
		ret = WSAGetLastError();

	return ret;
}


static int _recvData(SOCKET Socket, void* Data, size_t Length)
{
	int ret = 0;
	int bytesReceived = 0;
	char* tmp = NULL;

	tmp = (char *)Data;
	while (Length > 0) {
		bytesReceived = recv(Socket, tmp, (int)Length, 0);
		if (bytesReceived < 0) {
			ret = WSAGetLastError();
			break;
		}

		tmp += bytesReceived;
		Length -= bytesReceived;
	}

	return ret;
}


static int _MFMessage_Recv(SOCKET Socket, PMF_LISTED_MESSAGE *Message)
{
	int ret = 0;
	int bytesReceived = 0;
	MF_MESSAGE_HEADER hdr;
	PMF_LISTED_MESSAGE tmpMsg = NULL;

	ret = _recvData(Socket, &hdr, sizeof(hdr));
	if (ret == 0 && hdr.MessageSize < sizeof(hdr))
		ret = ERROR_INVALID_PARAMETER;

	if (ret == 0)
		ret = MFGen_RefMemAlloc(hdr.MessageSize + sizeof(MF_LISTED_MESSAGE) - sizeof(tmpMsg->Header), (void **)&tmpMsg);

	if (ret == 0) {
		MFList_Init(&tmpMsg->Entry);
		tmpMsg->Header = hdr;
		ret = _recvData(Socket, &tmpMsg->Header + 1, hdr.MessageSize - sizeof(hdr));
		if (ret == 0) {
			*Message = tmpMsg;
			MFGen_RefMemAddRef(tmpMsg);
		}

		MFGen_RefMemRelease(tmpMsg);
	}

	return ret;
}


static int _MFMessage_Send(SOCKET Socket, const MF_LISTED_MESSAGE *Message)
{
	int ret = 0;

	ret = _sendData(Socket, &Message->Header, Message->Header.MessageSize);

	return ret;
}


static int _MFConn_Init(PMF_CONNECTION Conn, PMF_BROKER Broker, SOCKET Socket, void *Context)
{
	int ret = 0;

	memset(Conn, 0, sizeof(MF_CONNECTION));
	if (InitializeCriticalSectionAndSpinCount(&Conn->SendLock, 0x1000)) {
		MFList_Init(&Conn->MessagesToSend);
		Conn->Socket = Socket;
		Conn->Context = Context;
		Conn->Broker = Broker;
		Conn->State = mcsHandshake;
	} else ret = GetLastError();

	return ret;
}


static void _MFConn_Finit(PMF_CONNECTION Conn)
{
	PMF_LISTED_MESSAGE msg = NULL;
	PMF_LISTED_MESSAGE old = NULL;

	closesocket(Conn->Socket);
	msg = CONTAINING_RECORD(Conn->MessagesToSend.Next, MF_LISTED_MESSAGE, Entry);
	while (&msg->Entry != &Conn->MessagesToSend) {
		old = msg;
		msg = CONTAINING_RECORD(msg->Entry.Next, MF_LISTED_MESSAGE, Entry);
		MFGen_RefMemRelease(msg);
	}

	DeleteCriticalSection(&Conn->SendLock);
	Conn->State = mcsFree;

	return;
}


PMF_CONNECTION _MFConn_Next(PMF_CONNECTION Start, PMF_CONNECTION Current, int Max)
{
	PMF_CONNECTION ret = NULL;

	ret = Current;
	while (ret - Start < Max && ret->State == mcsFree)
		++ret;

	if (ret - Start == Max)
		ret = NULL;

	return ret;
}


PMF_CONNECTION _MFConn_First(PMF_CONNECTION Start, int Max)
{
	return _MFConn_Next(Start, Start, Max);
}


static PMF_CONNECTION _MFConn_FindFree(PMF_CONNECTION Start, int Max)
{
	PMF_CONNECTION ret = NULL;

	ret = Start;
	while (ret - Start < Max && ret->State != mcsFree)
		++ret;

	if (ret - Start == Max)
		ret = NULL;

	return ret;
}


static DWORD WINAPI _MFBrokerThreadRoutine(PVOID Context)
{
	DWORD ret = 0;
	int waitRes = 0;
	PMF_BROKER broker = (PMF_BROKER)Context;
	struct pollfd *fds = NULL;
	int fdCount = 0;
	struct pollfd *tmp = NULL;
	PMF_LISTED_MESSAGE msg = NULL;
	PMF_LISTED_MESSAGE old = NULL;
	PMF_CONNECTION conn = NULL;
	MF_LIST_ENTRY recvList;
	SOCKET newSocket = INVALID_SOCKET;

	MFList_Init(&recvList);
	while (!broker->Terminated) {
		if (fdCount != broker->ConnectionCount) {
			if (fds != NULL)
				MFGen_RefMemRelease(fds);

			fds = NULL;
			ret = MFGen_RefMemAlloc((broker->ConnectionCount + 1)*sizeof(fds), (void **)&fds);
			if (ret != 0) {
				ret = broker->ErrorCallback(betMemoryAllocation, ret, NULL, broker->ErrorCallbackContext);
				if (ret == 0)
					continue;
				
				break;
			}

			fdCount = broker->ConnectionCount;
		}
		
		tmp = fds;
		conn = _MFConn_First(broker->Connections, _maxConnections);
		for (int i = 0; i < fdCount; ++i) {			
			tmp->fd = conn->Socket;
			tmp->revents = 0;
			tmp->events = POLLIN;
			if (!MFList_Empty(&conn->MessagesToSend))
				tmp->events |= POLLOUT;
			
			++tmp;
			conn = _MFConn_Next(broker->Connections, conn + 1, _maxConnections);
		}

		if (broker->Mode == bmServer) {
			tmp->fd = broker->ListenSocket;
			tmp->events = POLLIN;
			tmp->revents = 0;
		}

		waitRes = WSAPoll(fds, fdCount + (broker->ListenSocket != INVALID_SOCKET ? 1 : 0), 1000);
		if (waitRes > 0) {
			tmp = fds;
			conn = _MFConn_First(broker->Connections, _maxConnections);
			while (tmp - fds < fdCount) {
				if (tmp->revents & POLLERR) {
					broker->ErrorCallback(betSocketError, ret, conn, broker->ErrorCallbackContext);
					_MFConn_Finit(conn);
					--broker->ConnectionCount;
					++tmp;
					conn = _MFConn_Next(broker->Connections, conn + 1, _maxConnections);
					continue;
				}
				
				if (tmp->revents & POLLIN) {
					ret = _MFMessage_Recv(conn->Socket, &msg);
					if (ret == 0) {
						msg->OriginalFlags = msg->Header.Flags;
						if (msg->Header.Flags & MF_MESSAGE_SIGNED) {
							ret = MFMessage_Verify(&msg->Header, &conn->Key);
							if (ret != 0)
								broker->ErrorCallback(betMessageInvalidSignature, ret, msg, broker->ErrorCallbackContext);
						}

						if (ret == 0 && msg->Header.Flags & MF_MESSAGE_HEADER_ENCRYPTED) {
							ret = MFMessage_HeaderDecrypt(&msg->Header, &broker->PublicKey, &broker->SecretKey);
							if (ret != 0)
								broker->ErrorCallback(betMessageFailedDecryption, ret, msg, broker->ErrorCallbackContext);
						}

						if (ret == 0) {
							MFList_InsertTail(&recvList, &msg->Entry);
							MFGen_RefMemAddRef(msg);
						}

						MFGen_RefMemRelease(msg);
					} else broker->ErrorCallback(betMessageFailedReceive, ret, conn, broker->ErrorCallbackContext);
				}

				if (tmp->revents & POLLHUP) {
					_MFConn_Finit(conn);
					--broker->ConnectionCount;
					++tmp;
					conn = _MFConn_Next(broker->Connections, conn + 1, _maxConnections);
					continue;
				}

				if (tmp->revents & POLLOUT) {
					EnterCriticalSection(&conn->SendLock);
					while (ret == 0 && !MFList_Empty(&conn->MessagesToSend)) {
						msg = CONTAINING_RECORD(conn->MessagesToSend.Next, MF_LISTED_MESSAGE, Entry);
						MFList_Remove(&msg->Entry);
						LeaveCriticalSection(&conn->SendLock);
						if ((msg->Header.Flags & MF_MESSAGE_HEADER_ENCRYPTED) == 0 && broker->MessageCallback(bmetAboutToEncrypt, &msg->Header, &msg->Header + 1, msg->Header.DataSize, msg->OriginalFlags, broker->MessageCallbackContext))
							MFMessage_HeaderEncrypt(&msg->Header, &conn->Key);

						if ((msg->Header.Flags & MF_MESSAGE_SIGNED) == 0 && broker->MessageCallback(bmetAboutToSign, &msg->Header, &msg->Header + 1, msg->Header.MessageSize - sizeof(msg->Header), msg->OriginalFlags, broker->MessageCallbackContext))
							MFMessage_Sign(&msg->Header, &broker->SecretKey);
						
						ret = _MFMessage_Send(conn->Socket, msg);
						if (ret == 0)
							broker->MessageCallback(bmetSent, &msg->Header, &msg->Header + 1, msg->Header.MessageSize - sizeof(msg->Header), msg->OriginalFlags, broker->MessageCallbackContext);
						
						if (ret != 0)
							broker->ErrorCallback(betMessageFailedSend, ret, conn, broker->ErrorCallbackContext);

						MFGen_RefMemRelease(msg);
						EnterCriticalSection(&conn->SendLock);
					}

					LeaveCriticalSection(&conn->SendLock);
				}

				++tmp;
				conn = _MFConn_Next(broker->Connections, conn + 1, _maxConnections);
			}

			if (ret == 0 && broker->Mode == bmServer) {
				if (tmp->revents & POLLERR) {
					broker->ErrorCallback(betSocketError, ret, NULL, broker->ErrorCallbackContext);
					ret = -1;
				}

				if (ret == 0 && tmp->revents & POLLIN) {
					newSocket = accept(broker->ListenSocket, NULL, NULL);
					if (newSocket == INVALID_SOCKET) {
						ret = WSAGetLastError();
						broker->ErrorCallback(betAcceptFailed, ret, NULL, broker->ErrorCallbackContext);
					}

					if (ret == 0) {
						conn = _MFConn_FindFree(broker->Connections, _maxConnections);
						if (conn == NULL) {
							ret = -1;
							broker->ErrorCallback(betTooManyConnections, ret, NULL, broker->ErrorCallbackContext);
						}

						if (ret == 0) {
							ret = _MFConn_Init(conn, broker, newSocket, NULL);
							if (ret != 0)
								broker->ErrorCallback(betMemoryAllocation, ret, NULL, broker->ErrorCallbackContext);
						}

						if (ret == 0)
							++broker->ConnectionCount;

						if (ret != 0) {
							// TODO: Send error message
							closesocket(newSocket);
						}
					}
				}
			}
		} else if (waitRes == SOCKET_ERROR) {
			ret = WSAGetLastError();
			broker->Terminated = TRUE;
			broker->ErrorCallback(betFailedPoll, ret, NULL, broker->ErrorCallbackContext);
		}

		msg = CONTAINING_RECORD(recvList.Next, MF_LISTED_MESSAGE, Entry);
		while (&msg->Entry != &recvList) {
			old = msg;
			msg = CONTAINING_RECORD(msg->Entry.Next, MF_LISTED_MESSAGE, Entry);
			broker->MessageCallback(bmetReady, &old->Header, &old->Header + 1, old->Header.DataSize, old->OriginalFlags, broker->MessageCallbackContext);
			MFGen_RefMemRelease(old);
		}
	}

	if (fds != NULL)
		MFGen_RefMemRelease(fds);

	if (broker->Mode == bmServer)
		closesocket(broker->ListenSocket);

	return ret;
}


int MFBroker_Alloc(EBrokerMode Mode, const char* HostPort, const MFCRYPTO_PUBLIC_KEY* PublicKey, const MFCRYPTO_SECRET_KEY* SecretKey, MF_BROKER_MESSAGE_CALLBACK* MessageCallback, void* MessageCallbackContext, MF_BROKER_ERROR_CALLBACK* ErrorCallback, void* ErrorCallbackContext, PMF_BROKER* Broker)
{
	int ret = 0;
	SOCKET sock = NULL;
	struct addrinfo hints;
	struct addrinfo *addrs = NULL;
	struct addrinfo *tmp = NULL;
	PMF_BROKER tmpBroker = NULL;

	ret = MFGen_RefMemAlloc(sizeof(MF_BROKER) + _maxConnections*sizeof(MF_CONNECTION), (void **)&tmpBroker);
	if (ret == 0) {
		memset(tmpBroker, 0, sizeof(MF_BROKER));
		tmpBroker->Mode = Mode;
		tmpBroker->MessageCallback = MessageCallback;
		tmpBroker->MessageCallbackContext = ErrorCallbackContext;
		tmpBroker->ErrorCallback = ErrorCallback;
		tmpBroker->ErrorCallbackContext = ErrorCallbackContext;
		tmpBroker->Connections = (PMF_CONNECTION)(tmpBroker + 1);
		tmpBroker->ListenSocket = INVALID_SOCKET;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		ret = getaddrinfo(HostPort, NULL, &hints, &addrs);
		if (ret == 0) {
			tmp = addrs;
			while (tmp != NULL) {
				sock = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
				if (sock != INVALID_SOCKET) {
					switch (tmpBroker->Mode) {
						case bmClient:
							ret = connect(sock, tmp->ai_addr, (int)tmp->ai_addrlen);
							if (ret == SOCKET_ERROR)
								ret = WSAGetLastError();

							if (ret == 0)
								ret = _MFConn_Init(tmpBroker->Connections, tmpBroker, sock, NULL);
							
							if (ret == 0)
								++tmpBroker->ConnectionCount;
							break;
						case bmServer:
							ret = bind(sock, tmp->ai_addr, (int)tmp->ai_addrlen);
							if (ret == SOCKET_ERROR)
								ret = WSAGetLastError();

							if (ret == 0) {
								ret = listen(sock, SOMAXCONN);
								if (ret == SOCKET_ERROR)
									ret = WSAGetLastError();
							}

							if (ret == 0)
								tmpBroker->ListenSocket = sock;
							break;
					}

					if (ret != 0)
						closesocket(sock);
				}

				if (ret == 0)
					break;

				tmp = tmp->ai_next;
			}

			freeaddrinfo(addrs);
		}

		if (ret == 0) {
			tmpBroker->ThreadHandle = CreateThread(NULL, 0, _MFBrokerThreadRoutine, tmpBroker, 0, &tmpBroker->ThreadId);
			if (tmpBroker->ThreadHandle == NULL)
				ret = GetLastError();

			if (ret != 0) {
				switch (tmpBroker->Mode) {
					case bmClient:
						_MFConn_Finit(tmpBroker->Connections);
						--tmpBroker->ConnectionCount;
						break;
					case bmServer:
						closesocket(sock);
						break;
				}
			}
		}

		if (ret == 0) {
			MFGen_RefMemAddRef(tmpBroker);
			*Broker = tmpBroker;
		}

		MFGen_RefMemRelease(tmpBroker);
	}

	return ret;
}


void MFBroker_Free(PMF_BROKER Broker)
{
	PMF_CONNECTION conn = NULL;

	Broker->Terminated;
	WaitForSingleObject(Broker->ThreadHandle, INFINITE);
	CloseHandle(Broker->ThreadHandle);
	conn = _MFConn_First(Broker->Connections, _maxConnections);
	while (conn != NULL) {
		_MFConn_Finit(conn);
		conn = _MFConn_Next(Broker->Connections, conn + 1, _maxConnections);
	}
	
	MFGen_RefMemRelease(Broker);

	return;
}
