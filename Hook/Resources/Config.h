#pragma once
#include <string>

namespace Config {
	// Tool
	const bool IsDebugMode = true;
	// Socket
	const std::string ServerAddr = "127.0.0.1";
	const unsigned short LoginServerPort = 8484;
	const bool IsSendConnectKey = false;
	const std::string ConnectKey = "secret";
	const unsigned char RecvXOR = 0x00; // 0xC
	const unsigned char SendXOR = 0x00; // 0xC0
	// ResMan
	// If set MapleWZKey 0 will use maple version as wz key
	// To prevent WZ file unpacking, it's recommended to save MapleWZKey greater than 2000
	const unsigned int MapleWZKey = 0;
	// DamageSkin
	const unsigned int DamageSkinID = 0;
	// Wnd
	const std::string WindowTitle = "goms";
	const bool IsMultipleClient = true;
} 