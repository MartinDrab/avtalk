
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "mflist.h"
#include "mflock.h"
#include "mfmemory.h"
#include "mfcrypto.h"
#include "mfmessages.h"
#include "mfthread.h"
#include "mfbroker.h"




static int _maxConnections = 64;
static int _workerThreadCount = 8;



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
	ret = MFLock_Init(&Conn->SendLock);
	if (ret == 0) {
		MFList_Init(&Conn->MessagesToSend);
		Conn->Socket = Socket;
		Conn->Context = Context;
		Conn->Broker = Broker;
		Conn->ReferenceCount = 1;
		Conn->State = mcsHandshake;
	} else ret = GetLastError();

	return ret;
}


static void _MFConn_AddRef(PMF_CONNECTION Conn)
{
	InterlockedIncrement(&Conn->ReferenceCount);

	return;
}


static void _MFConn_Disconnect(PMF_CONNECTION Conn)
{
	if (Conn->Socket != INVALID_SOCKET) {
		shutdown(Conn->Socket, SD_BOTH);
		closesocket(Conn->Socket);
	}

	Conn->Socket = INVALID_SOCKET;
	Conn->Disconnected = 1;

	return;
}

static void _MFConn_Release(PMF_CONNECTION Conn)
{
	PMF_LISTED_MESSAGE msg = NULL;
	PMF_LISTED_MESSAGE old = NULL;

	if (InterlockedDecrement(&Conn->ReferenceCount) == 0) {
		_MFConn_Disconnect(Conn);
		msg = CONTAINING_RECORD(Conn->MessagesToSend.Next, MF_LISTED_MESSAGE, Entry);
		while (&msg->Entry != &Conn->MessagesToSend) {
			old = msg;
			msg = CONTAINING_RECORD(msg->Entry.Next, MF_LISTED_MESSAGE, Entry);
			MFGen_RefMemRelease(msg);
		}

		MFLock_Finit(&Conn->SendLock);
		Conn->State = mcsFree;
	}

	return;
}


PMF_CONNECTION _MFConn_Next(PMF_CONNECTION Start, PMF_CONNECTION Current, int Max, size_t Step)
{
	PMF_CONNECTION ret = NULL;

	ret = Current;
	while (ret - Start < Max && ret->State == mcsFree)
		ret += Step;

	if (ret - Start >= Max)
		ret = NULL;

	return ret;
}


PMF_CONNECTION _MFConn_First(PMF_CONNECTION Start, int Max, size_t Step)
{
	return _MFConn_Next(Start, Start, Max, Step);
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


static int _MFBrokerThreadRoutine(PMF_THREAD Thread)
{
	int ret = 0;
	int waitRes = 0;
	int fdCount = 0;
	MF_LIST_ENTRY recvList;
	PMF_BROKER broker = NULL;
	uint32_t threadIndex = 0;
	struct pollfd *fds = NULL;
	struct pollfd *tmp = NULL;
	PMF_CONNECTION conn = NULL;
	PMF_LISTED_MESSAGE msg = NULL;
	PMF_LISTED_MESSAGE old = NULL;

	broker = (PMF_BROKER)MFThread_Context(Thread);
	threadIndex = MFTHread_Index(Thread) % broker->ThreadCount;
	MFList_Init(&recvList);
	while (!MFThread_Terminated(Thread)) {
		if (fdCount != broker->ConnectionCount) {
			if (fds != NULL)
				MFGen_RefMemRelease(fds);

			fds = NULL;
			ret = MFGen_RefMemAlloc(broker->ConnectionCount*sizeof(fds), (void **)&fds);
			if (ret != 0) {
				ret = broker->ErrorCallback(betMemoryAllocation, ret, NULL, broker->ErrorCallbackContext);
				if (ret == 0)
					continue;
				
				break;
			}

			fdCount = broker->ConnectionCount;
		}
		
		tmp = fds;
		conn = _MFConn_First(broker->Connections + threadIndex, _maxConnections - threadIndex, broker->ThreadCount);
		for (int i = 0; i < fdCount; ++i) {			
			tmp->fd = conn->Socket;
			tmp->revents = 0;
			tmp->events = POLLIN;
			if (!MFList_Empty(&conn->MessagesToSend))
				tmp->events |= POLLOUT;
			
			++tmp;
			conn = _MFConn_Next(broker->Connections, conn + 1, _maxConnections, broker->ThreadCount);
		}

		waitRes = WSAPoll(fds, fdCount, 1000);
		if (waitRes > 0) {
			tmp = fds;
			conn = _MFConn_First(broker->Connections + threadIndex, _maxConnections - threadIndex, broker->ThreadCount);
			while (tmp - fds < fdCount) {
				if (tmp->revents & POLLERR) {
					broker->ErrorCallback(betSocketError, ret, conn, broker->ErrorCallbackContext);
					_MFConn_Disconnect(conn);
					_MFConn_Release(conn);
					--broker->ConnectionCount;
					++tmp;
					conn = _MFConn_Next(broker->Connections, conn + 1, _maxConnections, broker->ThreadCount);
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
							MFGen_RefMemAddRef(msg);
							MFList_InsertTail(&recvList, &msg->Entry);
							_MFConn_AddRef(conn);
							msg->Connection = conn;
						}

						MFGen_RefMemRelease(msg);
					} else broker->ErrorCallback(betMessageFailedReceive, ret, conn, broker->ErrorCallbackContext);
				}

				if (tmp->revents & POLLHUP) {
					_MFConn_Disconnect(conn);
					_MFConn_Release(conn);
					--broker->ConnectionCount;
					++tmp;
					conn = _MFConn_Next(broker->Connections, conn + 1, _maxConnections, broker->ThreadCount);
					continue;
				}

				if (tmp->revents & POLLOUT) {
					MFLock_Enter(&conn->SendLock);
					while (ret == 0 && !MFList_Empty(&conn->MessagesToSend)) {
						msg = CONTAINING_RECORD(conn->MessagesToSend.Next, MF_LISTED_MESSAGE, Entry);
						MFList_Remove(&msg->Entry);
						MFLock_Leave(&conn->SendLock);
						if ((msg->Header.Flags & MF_MESSAGE_HEADER_ENCRYPTED) == 0 && broker->MessageCallback(bmetAboutToEncrypt, conn, &msg->Header, &msg->Header + 1, msg->Header.DataSize, msg->OriginalFlags, broker->MessageCallbackContext))
							MFMessage_HeaderEncrypt(&msg->Header, &conn->Key);

						if ((msg->Header.Flags & MF_MESSAGE_SIGNED) == 0 && broker->MessageCallback(bmetAboutToSign, conn, &msg->Header, &msg->Header + 1, msg->Header.MessageSize - sizeof(msg->Header), msg->OriginalFlags, broker->MessageCallbackContext))
							MFMessage_Sign(&msg->Header, &broker->SecretKey);
						
						ret = _MFMessage_Send(conn->Socket, msg);
						if (ret == 0)
							broker->MessageCallback(bmetSent, conn, &msg->Header, &msg->Header + 1, msg->Header.MessageSize - sizeof(msg->Header), msg->OriginalFlags, broker->MessageCallbackContext);
						
						if (ret != 0)
							broker->ErrorCallback(betMessageFailedSend, ret, conn, broker->ErrorCallbackContext);

						MFGen_RefMemRelease(msg);
						MFLock_Enter(&conn->SendLock);
					}

					MFLock_Leave(&conn->SendLock);
				}

				++tmp;
				conn = _MFConn_Next(broker->Connections, conn + 1, _maxConnections, broker->ThreadCount);
			}
		} else if (waitRes == SOCKET_ERROR) {
			ret = WSAGetLastError();
			MFThread_Terminate(Thread);
			broker->ErrorCallback(betFailedPoll, ret, NULL, broker->ErrorCallbackContext);
		}

		msg = CONTAINING_RECORD(recvList.Next, MF_LISTED_MESSAGE, Entry);
		while (&msg->Entry != &recvList) {
			old = msg;
			msg = CONTAINING_RECORD(msg->Entry.Next, MF_LISTED_MESSAGE, Entry);
			broker->MessageCallback(bmetReady, old->Connection, &old->Header, &old->Header + 1, old->Header.DataSize, old->OriginalFlags, broker->MessageCallbackContext);
			_MFConn_Release(old->Connection);
			MFGen_RefMemRelease(old);
		}
	}

	if (fds != NULL)
		MFGen_RefMemRelease(fds);

	if (broker->Mode == bmServer)
		closesocket(broker->ListenSocket);

	return ret;
}


static int _MFBrokerServerThread(PMF_THREAD Thread)
{
	int ret = 0;
	int waitRes = 0;
	struct pollfd fds[1];
	SOCKET newSocket = NULL;
	PMF_CONNECTION conn = NULL;
	PMF_BROKER broker = (PMF_BROKER)MFThread_Context(Thread);

	memset(fds, 0, sizeof(fds));
	while (!MFThread_Terminated(Thread)) {
		fds[0].fd = broker->ListenSocket;
		fds[0].events = POLLIN;
		fds[0].revents = 0;
		waitRes = WSAPoll(fds, sizeof(fds) / sizeof(fds[0]), 2000);
		if (waitRes > 0) {
			if (fds[0].revents & POLLERR) {
				broker->ErrorCallback(betSocketError, ret, NULL, broker->ErrorCallbackContext);
				ret = -1;
			}

			if (ret == 0 && fds[0].revents & POLLIN) {
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
		} else if (waitRes == SOCKET_ERROR) {
			ret = WSAGetLastError();
			MFThread_Terminate(Thread);
			broker->ErrorCallback(betFailedPoll, ret, NULL, broker->ErrorCallbackContext);
		}
	}

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

	ret = MFGen_RefMemAlloc(sizeof(MF_BROKER) + _maxConnections*sizeof(MF_CONNECTION) + _workerThreadCount*sizeof(PMF_THREAD), (void **)&tmpBroker);
	if (ret == 0) {
		memset(tmpBroker, 0, sizeof(MF_BROKER));
		tmpBroker->Mode = Mode;
		tmpBroker->MessageCallback = MessageCallback;
		tmpBroker->MessageCallbackContext = ErrorCallbackContext;
		tmpBroker->ErrorCallback = ErrorCallback;
		tmpBroker->ErrorCallbackContext = ErrorCallbackContext;
		tmpBroker->Connections = (PMF_CONNECTION)(tmpBroker + 1);
		tmpBroker->Threads = (PMF_THREAD*)(tmpBroker->Connections + _maxConnections);
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
			switch (Mode) {
				case bmClient:
					tmpBroker->ThreadCount = 1;
					ret = MFThread_Create(_MFBrokerThreadRoutine, tmpBroker, tmpBroker->Threads);
					break;
				case bmServer:
					ret = MFThread_Create(_MFBrokerServerThread, tmpBroker, &tmpBroker->ListenThread);
					if (ret == 0) {
						tmpBroker->ThreadCount = _workerThreadCount;
						for (size_t i = 0; i < tmpBroker->ThreadCount; ++i) {
							ret = MFThread_Create(_MFBrokerThreadRoutine, tmpBroker, tmpBroker->Threads + i);
							if (ret != 0) {
								for (size_t j = 0; j < i; ++j)
									MFThread_Terminate(tmpBroker->Threads[j]);

								for (size_t j = 0; j < i; ++j) {
									MFThread_WaitFor(tmpBroker->Threads[j]);
									MFThread_Close(tmpBroker->Threads[j]);
								}

								break;
							}
						}

						if (ret != 0) {
							MFThread_Terminate(tmpBroker->ListenThread);
							MFThread_WaitFor(tmpBroker->ListenThread);
							MFThread_Close(tmpBroker->ListenThread);
						}
					}
					break;
			}

			if (ret != 0) {
				switch (tmpBroker->Mode) {
					case bmClient:
						_MFConn_Release(tmpBroker->Connections);
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

	for (size_t i = 0; i < Broker->ThreadCount; ++i)
		MFThread_Terminate(Broker->Threads[i]);

	for (size_t i = 0; i < Broker->ThreadCount; ++i) {
		MFThread_WaitFor(Broker->Threads[i]);
		MFThread_Close(Broker->Threads[i]);
	}
	
	if (Broker->Mode == bmServer) {
		MFThread_Terminate(Broker->ListenThread);
		MFThread_WaitFor(Broker->ListenThread);
		MFThread_Close(Broker->ListenThread);
		closesocket(Broker->ListenSocket);
	}

	conn = _MFConn_First(Broker->Connections, _maxConnections, 1);
	while (conn != NULL) {
		_MFConn_Release(conn);
		conn = _MFConn_Next(Broker->Connections, conn + 1, _maxConnections, 1);
	}
	
	MFGen_RefMemRelease(Broker);

	return;
}
