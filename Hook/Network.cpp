#include "pch.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Network.h"

#include "Resources/AOBList.h"

#pragma comment(lib, "ws2_32.lib")

namespace {
	const std::string defaultIP = "127.0.0.1";
	const USHORT defaultPort = 8484;
	const int timeoutSec = 5;

	static WSADATA wsaData{};
	static struct sockaddr* gChannelSockAddr = nullptr;
	static struct sockaddr* gLoginSockAddr = nullptr;
	static std::string connectKey = "";
	static unsigned char recvXOR = 0x00;
	static unsigned char sendXOR = 0x00;
	static BYTE gMapleVersion;

	std::string IP2Str(const struct sockaddr* name) {
		std::string ipStr = "";
		char ipBuf[INET6_ADDRSTRLEN];
		if (name->sa_family == AF_INET) {
			sockaddr_in* addr_in = (sockaddr_in*)name;
			inet_ntop(AF_INET, &addr_in->sin_addr, ipBuf, sizeof(ipBuf));
			ipStr = std::string(ipBuf);
		}
		else if (name->sa_family == AF_INET6) {
			sockaddr_in6* addr_in6 = (sockaddr_in6*)name;
			inet_ntop(AF_INET6, &addr_in6->sin6_addr, ipBuf, sizeof(ipBuf));
			ipStr = std::string(ipBuf);
		}
		else {
			DEBUG(L"Unknown family");
		}
		return ipStr;
	}

	bool IsIPv4(const std::string& addr) {
		struct sockaddr_in sa {};
		return inet_pton(AF_INET, addr.c_str(), &(sa.sin_addr)) == 1;
	}

	bool IsIPv6(const std::string& addr) {
		struct sockaddr_in6 sa6 {};
		return inet_pton(AF_INET6, addr.c_str(), &(sa6.sin6_addr)) == 1;
	}

	bool ResolveDomain(struct sockaddr** ppName, const std::string& addr, const USHORT port) {
		struct addrinfo hints {};
		struct addrinfo* pResult = nullptr;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		int err = getaddrinfo(addr.c_str(), NULL, &hints, &pResult);
		if (err != 0) {
			SCANRES(err);
			if (err == WSANOTINITIALISED) {
				DEBUG(L"WSAStartup hasn't initialized.");
			}
			DEBUG(L"Failed to get addr info from domain");
			return false;
		}

		if (pResult->ai_family == AF_INET) {
			struct sockaddr_in* addr_in = new struct sockaddr_in();
			memcpy(addr_in, pResult->ai_addr, sizeof(struct sockaddr_in));
			addr_in->sin_port = htons(port);
			*ppName = (struct sockaddr*)addr_in;
		}
		else if (pResult->ai_family == AF_INET6) {
			struct sockaddr_in6* addr_in6 = new struct sockaddr_in6();
			memcpy(addr_in6, pResult->ai_addr, sizeof(struct sockaddr_in6));
			addr_in6->sin6_port = htons(port);
			*ppName = (struct sockaddr*)addr_in6;
		}
		else {
			DEBUG(L"Unknown family");
		}

		freeaddrinfo(pResult);

		return *ppName != nullptr;
	}

	void InitSockAddr(struct sockaddr** ppName, const std::string& addr, const USHORT port) {
		if (*ppName != nullptr) {
			DEBUG(L"SockAddr has already been initialized");
			return;
		}
		if (IsIPv4(addr)) {
			// Try to use IPv4
			struct sockaddr_in* pAddrIn = new struct sockaddr_in();
			pAddrIn->sin_family = AF_INET;
			inet_pton(AF_INET, addr.c_str(), &pAddrIn->sin_addr);
			pAddrIn->sin_port = htons(port);
			*ppName = (struct sockaddr*)pAddrIn;
			return;
		}
		if (IsIPv6(addr)) {
			// Try to use IPv6
			struct sockaddr_in6* pAddrIn6 = new struct sockaddr_in6();
			pAddrIn6->sin6_family = AF_INET6;
			inet_pton(AF_INET6, addr.c_str(), &pAddrIn6->sin6_addr);
			pAddrIn6->sin6_port = htons(port);
			*ppName = (struct sockaddr*)pAddrIn6;
			return;
		}
		if (ResolveDomain(ppName, addr, port)) {
			// Try to resolve domain
			DEBUG(L"Resolve domain ok");
			return;
		}
		// Use default addr
		struct sockaddr_in* pAddrIn = new struct sockaddr_in();
		pAddrIn->sin_family = AF_INET;
		inet_pton(AF_INET, defaultIP.c_str(), &(pAddrIn->sin_addr));
		pAddrIn->sin_port = htons(defaultPort);
		*ppName = (struct sockaddr*)pAddrIn;
	}

	bool IsWritable(SOCKET s, int timeoutSec) {
		fd_set writefds{};
		FD_ZERO(&writefds);
		FD_SET(s, &writefds);
		struct timeval timeout {};
		timeout.tv_sec = timeoutSec;
		timeout.tv_usec = 0;
		int status = select(s + 1, NULL, &writefds, NULL, &timeout);
		if (status > 0) {
			if (FD_ISSET(s, &writefds)) {
				return true;
			}
			else {
				DEBUG(L"socket is not ready for writing");
			}
		}
		else if (status == 0) {
			DEBUG(L"select timeout");
		}
		else {
			DEBUG(L"select error: " + WSAGetLastError());
		}
		return false;
	}

	// 1.CWvsApp::ConnectLogin(this,bFirstConnect)
	// 2.CClientSocket::ConnectLogin(this)
	// 3.CClientSocket::Connect(this,ctx)
	// 4.CClientSocket::Connect(this,pAddr) call connect
	// 5.Send connect key before CClientSocket::OnConnect(this,bSuccess)
	static auto _connect = decltype(&connect)(GetProcAddress(GetModuleHandleW(L"Ws2_32.dll"), "connect"));
	int WINAPI connect_Hook(SOCKET s, const struct sockaddr* name, int namelen) {
		int res = SOCKET_ERROR;
		sockaddr_in* addr_in = (sockaddr_in*)name;
		std::string serverReturnedIP = IP2Str(name);
		USHORT serverReturnedPort = ntohs(addr_in->sin_port);
		if (serverReturnedPort == defaultPort) {
			DEBUG(L"Connect to login server");
			res = _connect(s, gLoginSockAddr, sizeof(*gLoginSockAddr));
		}
		else {
			DEBUG(L"Connect to channel server with port:" + std::to_wstring(serverReturnedPort));
			if (serverReturnedIP == defaultIP) {
				// Server returned localhost ip and channel port
				if (gChannelSockAddr->sa_family == AF_INET6) {
					sockaddr_in6* addr_ipv6 = (sockaddr_in6*)gChannelSockAddr;
					addr_ipv6->sin6_port = addr_in->sin_port;
				}
				else {
					sockaddr_in* addr_ipv4 = (sockaddr_in*)gChannelSockAddr;
					addr_ipv4->sin_port = addr_in->sin_port;
				}
				// Using launcher-side addr to connect channel server
				res = _connect(s, gChannelSockAddr, sizeof(*gChannelSockAddr));
			}
			else {
				// Server returned public ip and channel port
				// Using server-provided addr to connect channel server
				res = _connect(s, name, namelen);
			}
		}
		if (connectKey.empty()) {
			return res;
		}
		int err = WSAGetLastError();
		if (res == SOCKET_ERROR && err == WSAEWOULDBLOCK) {
			if (IsWritable(s, timeoutSec)) {
				// Send connect key
				if (send(s, connectKey.c_str(), connectKey.length(), 0) == SOCKET_ERROR) {
					DEBUG(L"Failed to send connect key");
				}
				else {
					DEBUG(L"Send connect key ok");
				}
			}
			else {
				DEBUG(L"Failed to check writable");
			}
		}
		// Set the last error avoid call ClientSocket::Close()
		SetLastError(WSAEWOULDBLOCK);
		return res;
	}

	// 1.Server XOR all OutPacket length and payload (also include OnConnect packet)
	// 2.Client need to recover original packet through XOR during recv
	// 3.Get maple version from connect packet
	static auto _recv = decltype(&recv)(GetProcAddress(GetModuleHandleW(L"Ws2_32.dll"), "recv"));
	int WINAPI recv_Hook(SOCKET s, char* buf, int len, int flags) {
		int res = _recv(s, buf, len, flags);
		if (res == SOCKET_ERROR) {
			return res;
		}
		if (res == 0) {
			DEBUG(L"Connection was closed by peer");
			return res;
		}
		if (recvXOR != 0x00) {
			for (int i = 0; i < res; i++) {
				buf[i] ^= recvXOR;
			}
		}
		if (res > 2 && gMapleVersion == 0) {
			gMapleVersion = buf[0];
		}
		return res;
	}

	// Server need to recover original packet through XOR during recv
	static auto _send = decltype(&send)(GetProcAddress(GetModuleHandleW(L"Ws2_32.dll"), "send"));
	int WINAPI send_Hook(SOCKET s, char* buf, int len, int flags) {
		if (sendXOR != 0x00) {
			for (int i = 0; i < len; i++) {
				buf[i] ^= sendXOR;
			}
		}
		int res = _send(s, buf, len, flags);
		if (res == SOCKET_ERROR) {
			return res;
		}
		if (res == 0) {
			DEBUG(L"Connection was closed by peer");
			return res;
		}
		return res;
	}
}

namespace Network {
	bool InitWSAData() {
		if (wsaData.wVersion != 0) {
			DEBUG(L"wsaData has already been initialized");
			return false;
		}
		int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (res != 0) {
			DEBUG(L"Failed to init WSAData");
			SCANRES(res);
			return false;
		}
		return true;
	}

	void ClearupWSA() {
		int res = WSACleanup();
		if (res != 0) {
			DEBUG(L"Failed to clear up WSA");
			return;
		}
		DEBUG(L"WSACleanup ok");
	}

	bool FixDomain(Rosemary& r) {
		// TMS old default DNS (Unable show ad window if not set)
		return r.StringPatch("tw.login.maplestory.gamania.com", "127.0.0.1");
		// Not need this because HS has been removed
		//r.StringPatch("tw.hackshield.gamania.com", "202.80.106.36");
	}

	void SetConnectKey(const std::string& key) {
		connectKey = key;
	}

	bool Redirect(const std::string& serverAddr, const unsigned short loginServerPort) {
		InitSockAddr(&gLoginSockAddr, serverAddr, loginServerPort);
		InitSockAddr(&gChannelSockAddr, serverAddr, 0);
		return SHOOK(true, &_connect, connect_Hook);
	}

	bool RecvXOR(const unsigned char XOR) {
		recvXOR = XOR;
		return SHOOK(true, &_recv, recv_Hook);
	}

	bool SendXOR(const unsigned char XOR) {
		sendXOR = XOR;
		return SHOOK(true, &_send, send_Hook);
	}

	const BYTE* GetMapleVersion() {
		return &gMapleVersion;
	}
}