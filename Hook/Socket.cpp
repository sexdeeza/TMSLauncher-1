#include <winsock2.h>
#include <ws2tcpip.h>
#include "Socket.h"
#include "AobList.h"
#include "Tool.h"

#include "Share/Funcs.h"

#pragma comment(lib, "ws2_32.lib")

namespace {
	const std::string defaultIP = "127.0.0.1";
	const USHORT defaultPort = 8484;
	const int timeoutSec = 5;

	static WSADATA wsaData{};
	static struct sockaddr* gSockAddr = nullptr;
	static std::string connectKey = "";

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
		sockaddr_in* addr_in = (sockaddr_in*)name;
		int res = SOCKET_ERROR;
		if (addr_in->sin_port == htons(defaultPort)) {
			DEBUG(L"Connect to " + Str2WStr(IP2Str(gSockAddr)));
			res = _connect(s, gSockAddr, sizeof(*gSockAddr));
		}
		else {
			res = _connect(s, name, namelen);
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
}

namespace Socket {
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

	void SetConnectKey(const std::string& key) {
		connectKey = key;
	}

	bool Redirect(const std::string& addr, const unsigned short port) {
		InitSockAddr(&gSockAddr, addr, port);
		return SHOOK(true, &_connect, connect_Hook);
	}
}