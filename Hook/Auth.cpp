#include "Auth.h"
#include "AobList.h"
#include "Tool.h"

#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

namespace {
	bool RemoveHSKey(Rosemary& r) {
		// TMS113-TMS183
		ULONG_PTR uAddress = r.Scan(AOB_Scan_HS_Key_Crypt);
		if (uAddress == 0) {
			DEBUG(L"Unable find HS Key");
			return false;
		}
		ULONG_PTR uStartKeyCrypt = uAddress + 0x05 + *(ULONG_PTR*)(uAddress + 0x01);
		ULONG_PTR uStopKeyCrypt = uAddress + 0x0C + *(ULONG_PTR*)(uAddress + 0x08);
		return r.WriteBytes(uStartKeyCrypt, AOB_Patch_Ret) &&
			r.WriteBytes(uStopKeyCrypt, AOB_Patch_Ret);
	}

	bool HSThing(Rosemary& r) {
		// TMS157.2 HS things (when enter character select page)
		ULONG_PTR uAddress = r.Scan(AOB_Scan_HS_Enter_Charcter_Page_TMS157);
		if (uAddress == 0) {
			DEBUG(L"Unable find HSThing");
			return false;
		}
		return r.JMP(uAddress, uAddress + 0x4E); //r.JMP(0x60ED4F, 0x60ED9D);
	}
}

namespace Auth {
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

	bool RemoveAntiCheat(Rosemary& r) {
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
			return r.WriteBytes(CSecurityClientIsInstantiated, AOB_Patch_Ret);
		}
		else if (index < 5) {
			// TMS122-TMS192
			// Call TSingleton<CSecurityClient>::IsInstantiated in CWvsApp::ZtlExceptionHandler(68 00 00 00 80 6A 02)
			ULONG_PTR CSecurityClientIsInstantiated = GetFuncAddress(uAddress);
			return r.WriteBytes(CSecurityClientIsInstantiated, AOB_Patch_Ret);
		}
		else if (index == 5) {
			// TMS194 can't find TSingleton<CSecurityClient>::IsInstantiated in CWvsApp::ZtlExceptionHandler
			// Call TSingleton<CSecurityClient>::GetInstance in CSecurityClient::EncodeMemoryCheckResult
			ULONG_PTR CSecurityClientIsInstantiated = GetFuncAddress(uAddress) + 0x06;
			return r.WriteBytes(CSecurityClientIsInstantiated, AOB_Patch_Ret);
		}
		else {
			DEBUG(L"Unknown index in RemoveAntiCheat");
			return false;
		}
		return true;
	}
}