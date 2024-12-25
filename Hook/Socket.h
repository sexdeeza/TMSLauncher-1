#pragma once
#include "Share/Rosemary.h"

namespace Socket {
	bool InitWSAData(); // Ready for Winsock
	void ClearupWSA(); // Free Winsock
	void SetConnectKey(const std::string& key); // Avoid using cmd start client
	bool Redirect(const std::string& addr, const unsigned short port); // Redirect connect addr and port
	bool RecvXOR(const unsigned char XOR); // XOR all packet from server
}