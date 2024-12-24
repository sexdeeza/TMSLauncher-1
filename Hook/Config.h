#pragma once
#include <string>

namespace Config {
	// Tool
	const bool IsDebugMode = true;
	// Socket
	const std::string LoginServerAddr = "127.0.0.1";
	const unsigned short LoginServerPort = 8484;
	const bool IsSendConnectKey = false;
	const std::string ConnectKey = "secret";
	// Screen
	const std::string WindowTitle = "goms";
	const bool IsMultipleClient = true;
	// ImgFile
	const bool IsImgFile = false;
}