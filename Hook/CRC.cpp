#include "CRC.h"
#include "AobList.h"
#include "Tool.h"

#include "Share/Funcs.h"

#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

namespace {
	ULONG_PTR uRenderFrameAddress = 0;
	ULONG_PTR uRunderFrameJmpVMAddress = 0;
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
			// 00B975B5 - call 004172E2 // RederFrame
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
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRunderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0xE8 && bRetAddr[0x05] == 0xE9) { // E8 ?? ?? ?? ?? E9
			// TMS152 IWzGr2D::RenderFrame ret(0x00733DF8)->VM Protect(0x0120E68A)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRunderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0x50 && bRetAddr[0x01] == 0x9C) { // 50 9C
			// TMS159 IWzGr2D::RenderFrame ret(0x007A3E01)->VM Protect(0x0121FF3D)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRunderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0xE8 && bRetAddr[0x05] == 0x81) { // E8 ?? ?? ?? ?? 81
			// TMS160 IWzGr2D::RenderFrame ret(0x007D046A)->VM Protect(0x01754E16)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRunderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0xE8 && bRetAddr[0x05] == 0xC0) { // E8 ?? ?? ?? ?? C0
			// TMS170 IWzGr2D::RenderFrame ret(0x008EB1A2)->VM Protect(0x017CED4C)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRunderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0x60 && bRetAddr[0x01] == 0xE9) { // 60 E9
			// TMS175 IWzGr2D::RenderFrame ret(0x00976BBF)->VM Protect(0x00FD1D4B)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRunderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0x60 && bRetAddr[0x01] == 0xC7) { // 60 C7
			// TMS177 IWzGr2D::RenderFrame ret(0x009D47A1)->VM Protect(0x01998D10)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRunderFrameJmpVMAddress;
		}
		else if (bRetAddr[0x00] == 0xE9 && bRetAddr[0x05] == 0x34) { // E9 ?? ?? ?? ?? 34
			// TMS182 IWzGr2D::RenderFrame ret(0x00A56AD7)->VM Protect(0x01DF5EDF)
			*(ULONG_PTR*)_AddressOfReturnAddress() = uRunderFrameJmpVMAddress;
		}

		return _IWzGr2D__RenderFrame(ecx);
	}
}

namespace CRC {
	bool RemoveRenderFrameCheck(Rosemary& r) {
		uRenderFrameAddress = r.Scan(AOB_Scan_IWzGr2D__RenderFrame); // IWzGr2D::RenderFrame
		if (uRenderFrameAddress == 0) {
			return false;
		}
		uRunderFrameJmpVMAddress = r.Scan(AOB_Scan_IWzGr2D__RenderFrame_JMP_CWvsApp__Run); // CWvsApp::Run
		if (uRenderFrameAddress == 0) {
			return false;
		}
		SADDR(&_IWzGr2D__RenderFrame, uRenderFrameAddress);
		return SHOOK(true, &_IWzGr2D__RenderFrame, IWzGr2D__RenderFrame_Hook);
	}

	bool RemoveCWvsContextOnEnterFieldCheck(Rosemary& r) {
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