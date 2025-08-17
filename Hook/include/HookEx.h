#pragma once

namespace HookEx {

#pragma region ResManEx
	bool Mount(Rosemary& r, const BYTE* mapleVersion, const unsigned int* mapleWZKey);
	void* GetResMan();
	void* GetUnknown(void* tagVar, bool fAddRef, bool fTryChangeType);
#pragma endregion

}