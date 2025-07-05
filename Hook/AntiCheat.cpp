#include "Anticheat.h"

#include "Resources/AOBList.h"
#include "Share/Funcs.h"
#include "Share/Tool.h"

#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

namespace {

	// CRC
	ULONG_PTR uRenderFrameAddress = 0;
	ULONG_PTR uRenderFrameJmpVMAddress = 0;
	// TMS113 int __thiscall IWzGr2D::RenderFrame(void *this)
	int(__thiscall* _IWzGr2D__RenderFrame)(void* ecx);
	int __fastcall IWzGr2D__RenderFrame_Hook(void* ecx, void* edx) {
		// IWzGr2D::RenderFrame ret has multiple callers and set it to jump maybe unstable
		// So there set some conditions to avoid client crash
		auto bRetAddr = (BYTE*)_ReturnAddress();
		if (bRetAddr[-0x05] == 0xE8) {
			ULONG_PTR Call = (ULONG_PTR)&bRetAddr[-0x05] + *(ULONG_PTR*)&bRetAddr[-0x04] + 0x05;

			if (Call == uRenderFrameAddress) {
				// Prevent the client from getting stuck when selecting character
				return _IWzGr2D__RenderFrame(ecx);
			}
			// MSEA v102 jmp vmed, vmed: jmp RenderFrame
			// 00B975B5 - call 004172E2 // RenderFrame
			// 004172E2 - jmp 0083C5F0 // wtf
			auto bCall = (BYTE*)Call;
			if (bCall[0x00] == 0xE9) {
				Call = Call + *(ULONG_PTR*)&bCall[0x01] + 0x05;
				if (Call == uRenderFrameAddress) {
					// Prevent the client crash when entering field
					return _IWzGr2D__RenderFrame(ecx);
				}
			}
		}
		if (bRetAddr[0x00] == 0x68 && bRetAddr[0x05] == 0xE9 && bRetAddr[0x0A] == 0x68) { // 68 ?? ?? ?? ?? E9 ?? ?? ?? ?? 68
			// TMS113 IWzGr2D::RenderFrame ret(0x0077CE88)->VM Protect(0x00D7276E)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRenderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0xE8 && bRetAddr[0x05] == 0xE9) { // E8 ?? ?? ?? ?? E9
			// TMS152 IWzGr2D::RenderFrame ret(0x00733DF8)->VM Protect(0x0120E68A)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRenderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0x50 && bRetAddr[0x01] == 0x9C) { // 50 9C
			// TMS159 IWzGr2D::RenderFrame ret(0x007A3E01)->VM Protect(0x0121FF3D)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRenderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0xE8 && bRetAddr[0x05] == 0x81) { // E8 ?? ?? ?? ?? 81
			// TMS160 IWzGr2D::RenderFrame ret(0x007D046A)->VM Protect(0x01754E16)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRenderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0xE8 && bRetAddr[0x05] == 0xC0) { // E8 ?? ?? ?? ?? C0
			// TMS170 IWzGr2D::RenderFrame ret(0x008EB1A2)->VM Protect(0x017CED4C)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRenderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0x60 && bRetAddr[0x01] == 0xE9) { // 60 E9
			// TMS175 IWzGr2D::RenderFrame ret(0x00976BBF)->VM Protect(0x00FD1D4B)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRenderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0x60 && bRetAddr[0x01] == 0xC7) { // 60 C7
			// TMS177 IWzGr2D::RenderFrame ret(0x009D47A1)->VM Protect(0x01998D10)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRenderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0xE9 && bRetAddr[0x05] == 0x34) { // E9 ?? ?? ?? ?? 34
			// TMS182 IWzGr2D::RenderFrame ret(0x00A56AD7)->VM Protect(0x01DF5EDF)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRenderFrameJmpVMAddress;
		}

		return _IWzGr2D__RenderFrame(ecx);
	}

	bool RemoveHSKey(Rosemary& r) {
		// TMS113-TMS183
		ULONG_PTR uAddress = r.Scan(AOB_Scan_HS_Key_Crypt);
		if (uAddress == 0) {
			DEBUG(L"Unable find HS Key");
			return false;
		}
		ULONG_PTR uStartKeyCrypt = uAddress + 0x05 + *(ULONG_PTR*)(uAddress + 0x01);
		ULONG_PTR uStopKeyCrypt = uAddress + 0x0C + *(ULONG_PTR*)(uAddress + 0x08);
		return r.WriteCode(uStartKeyCrypt, AOB_Code_Ret) &&
			r.WriteCode(uStopKeyCrypt, AOB_Code_Ret);
	}

	bool HSThing(Rosemary& r) {
		// TMS157.2 HS things (when enter character select page)
		ULONG_PTR uAddress = r.Scan(AOB_Scan_HS_Enter_Character_Page_TMS157);
		if (uAddress == 0) {
			DEBUG(L"Unable find HSThing");
			return false;
		}
		return r.JMP(uAddress, uAddress + 0x4E); //r.JMP(0x60ED4F, 0x60ED9D);
	}
}

namespace AntiCheat {

	bool RemoveLocaleCheck(Rosemary& r) {
		ULONG_PTR uAddress = r.Scan(AOB_Scan_WinMain_Big5_Check);
		if (uAddress == 0) {
			DEBUG(L"Unable find WinMain Big5");
			return false;
		}
		ULONG_PTR uCheckAddress = uAddress + 5;
		auto bCheck = (BYTE*)(uCheckAddress);
		if (bCheck[0] == 0x74) {
			// short jump 0x74
			return r.WriteByte(uCheckAddress, 0xEB);
		}
		else if (bCheck[0] == 0x0F) {
			// near jump 0x0F 0x84
			return r.WriteByte(uCheckAddress, 0x90) && r.WriteByte(uCheckAddress + 1, 0xE9);
		}
		else {
			DEBUG(L"Unknown jmp");
			return false;
		}
	}

	bool RemoveSecurityClient(Rosemary& r) {
		ULONG_PTR uAddress = 0;
		size_t index;
		size_t length = AOB_Scan_CSecurityClient__IsInstantiated_Calls.size();
		for (index = 0; index < length; index++)
		{
			uAddress = r.Scan(AOB_Scan_CSecurityClient__IsInstantiated_Calls[index]);
			if (uAddress > 0) {
				break;
			}
		}
		if (uAddress == 0) {
			DEBUG(L"Unable find CSecurityClient::IsInstantiated");
			return false;
		}
		if (index < 4) {
			// TMS113-TMS183 is HS Version
			if (!RemoveHSKey(r)) {
				return false;
			}
		}
		if (index == 0) {
			// TMS113-TMS121 easy HS
			return r.WriteByte(uAddress, 0xEB);
		}
		else if (index == 1) {
			// TMS145 can't find TSingleton<CSecurityClient>::IsInstantiated in CWvsApp::ZtlExceptionHandler
			ULONG_PTR CSecurityClientIsInstantiated = uAddress + 0x08;
			return r.WriteCode(CSecurityClientIsInstantiated, AOB_Code_Ret);
		}
		else if (index < 5) {
			// TMS122-TMS192
			// Call TSingleton<CSecurityClient>::IsInstantiated in CWvsApp::ZtlExceptionHandler(68 00 00 00 80 6A 02)
			ULONG_PTR CSecurityClientIsInstantiated = GetFuncAddress(uAddress);
			return r.WriteCode(CSecurityClientIsInstantiated, AOB_Code_Ret);
		}
		else if (index == 5) {
			// TMS194 can't find TSingleton<CSecurityClient>::IsInstantiated in CWvsApp::ZtlExceptionHandler
			// Call TSingleton<CSecurityClient>::GetInstance in CSecurityClient::EncodeMemoryCheckResult
			ULONG_PTR CSecurityClientIsInstantiated = GetFuncAddress(uAddress) + 0x06;
			return r.WriteCode(CSecurityClientIsInstantiated, AOB_Code_Ret);
		}
		else {
			DEBUG(L"Unknown index in RemoveAntiCheat");
			return false;
		}
		return true;
	}

	bool RemoveTerminationError(Rosemary& r)
	{
		ULONG_PTR uAddress = 0;
		size_t length = AOB_Scan_CSecurityClient__ClearModule_Addrs.size();
		for (size_t index = 0; index < length; index++)
		{
			uAddress = r.Scan(AOB_Scan_CSecurityClient__ClearModule_Addrs[index]);
			if (uAddress > 0) {
				break;
			}
		}
		if (uAddress == 0) {
			DEBUG(L"Unable find CSecurityClient::ClearModule");
			return false;
		}
		return r.WriteCode(uAddress, AOB_Code_Ret);
	}

	bool RemoveRenderFrameCheck(Rosemary& r) {
		uRenderFrameAddress = r.Scan(AOB_Scan_IWzGr2D__RenderFrame); // IWzGr2D::RenderFrame
		if (uRenderFrameAddress == 0) {
			return false;
		}
		uRenderFrameJmpVMAddress = r.Scan(AOB_Scan_IWzGr2D__RenderFrame_JMP_CWvsApp__Run); // CWvsApp::Run
		if (uRenderFrameAddress == 0) {
			return false;
		}
		SADDR(&_IWzGr2D__RenderFrame, uRenderFrameAddress);
		return SHOOK(true, &_IWzGr2D__RenderFrame, IWzGr2D__RenderFrame_Hook);
	}

	bool RemoveEnterFieldCheck(Rosemary& r) {
		ULONG_PTR uAddress = r.Scan(AOB_Scan_CWvsContext__OnEnterField);
		if (uAddress == 0) {
			return false;
		}
		// Enter_VM AOB start at E9 8C 00 00 00
		ULONG_PTR uOnEnterField_Enter_VM = uAddress + 0x10;
		// Leave_VM AOB start at 8B 8D ?? FE FF FF 
		ULONG_PTR uOnEnterField_Leave_VM = uOnEnterField_Enter_VM + 0x91; // <-Enter_VM+0x91

		if (!uOnEnterField_Enter_VM || !uOnEnterField_Leave_VM) {
			return false;
		}

		return r.JMP(uOnEnterField_Enter_VM, uOnEnterField_Leave_VM);
	}
}