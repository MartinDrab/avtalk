
#include <winsock2.h>
#include <windows.h>
#include "mfgen.h"
#include "mfcrypto.h"
#include "mfmessages.h"
#include "mfbroker.h"




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


static DWORD WINAPI _MFBrokerThread(PVOID Context)
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

	MFList_Init(&recvList);
	while (!broker->Terminated) {
		if (fdCount != broker->ConnectionCount) {
			if (fds != NULL)
				MFGen_RefMemRelease(fds);

			fds = NULL;
			ret = MFGen_RefMemAlloc(broker->ConnectionCount*sizeof(fds), (void **)&fds);
			if (ret != 0)
				break;

			fdCount = broker->ConnectionCount;
		}
		
		tmp = fds;
		conn = broker->Connections;
		for (int i = 0; i < fdCount; ++i) {
			tmp->fd = conn->Socket;
			tmp->revents = 0;
			tmp->events = POLLIN;
			if (!MFList_Empty(&conn->MessagesToSend))
				tmp->events |= POLLOUT;
			
			++tmp;
			++conn;
		}

		waitRes = WSAPoll(fds, fdCount, 1000);
		if (waitRes > 0) {
			tmp = fds;
			conn = broker->Connections;
			while (conn - broker->Connections < fdCount) {
				if (tmp->revents & POLLERR) {
					broker->ErrorCallback(betSocketError, ret, conn, broker->ErrorCallbackContext);
					++tmp;
					++conn;
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

					++tmp;
					++conn;
					continue;
				}

				if (tmp->revents & POLLOUT) {
					while (ret == 0 && !MFList_Empty(&conn->MessagesToSend)) {
						msg = CONTAINING_RECORD(conn->MessagesToSend.Next, MF_LISTED_MESSAGE, Entry);
						if (broker->MessageCallback(bmetAboutToEncrypt, &msg->Header, &msg->Header + 1, msg->Header.DataSize, msg->OriginalFlags, broker->MessageCallbackContext))
							MFMessage_HeaderEncrypt(&msg->Header, &conn->Key);

						if (broker->MessageCallback(bmetAboutToSign, &msg->Header, &msg->Header + 1, msg->Header.MessageSize - sizeof(msg->Header), msg->OriginalFlags, broker->MessageCallbackContext))
							MFMessage_Sign(&msg->Header, &broker->SecretKey);
						
						ret = _MFMessage_Send(conn->Socket, msg);
						if (ret == 0) {
							MFList_Remove(&msg->Entry);
							MFGen_RefMemRelease(msg);
						} else broker->ErrorCallback(betMessageFailedSend, ret, conn, broker->ErrorCallbackContext);
					}

					++tmp;
					++conn;
					continue;
				}

				++tmp;
				++conn;
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

	return ret;
}
