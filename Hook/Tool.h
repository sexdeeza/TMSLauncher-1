#pragma once
#include <windows.h>
#include <string>
#include "Config.h"

std::wstring BYTE2String(BYTE b);
std::wstring DWORD2String(DWORD dw);
std::wstring Str2WStr(const std::string& str);
ULONG_PTR GetFuncAddress(ULONG_PTR uCall);
ULONG_PTR GetDWORDAddress(ULONG_PTR uPush);

#define DEBUG(msg) \
{\
	if (Config::IsDebugMode) {\
		std::wstring wmsg = L"[Debug] ";\
		wmsg += msg;\
		OutputDebugStringW(wmsg.c_str());\
	}\
}\

#define SCANRES(msg) { DEBUG(L""#msg" = " + DWORD2String(msg));}