#include "ImgFile.h"
#include "AobList.h"
#include "Tool.h"

#include "Share/Funcs.h"
#include "Maple/CWvsApp.h"

namespace {

	static void** gResMan;		// IWzResMan
	static void** gRoot;		// IWzNameSpace
	static void** pNameSpace;	// _PcSetRootNameSpace

	enum RESMAN_PARAM {
		RC_AUTO_SERIALIZE = 0x1,
		RC_AUTO_SERIALIZE_NO_CACHE = 0x2,
		RC_NO_AUTO_SERIALIZE = 0x4,
		RC_DEFAULT_AUTO_SERIALIZE = 0x0,
		RC_AUTO_SERIALIZE_MASK = 0x7,
		RC_AUTO_REPARSE = 0x10,
		RC_NO_AUTO_REPARSE = 0x20,
		RC_DEFAULT_AUTO_REPARSE = 0x0,
		RC_AUTO_REPARSE_MASK = 0x30,
	};

	// TMS113  int __cdecl PcCreateObject::IWzResMan(int a1, _DWORD *a2, int a3)
	// KMS1029 void __cdecl PcCreateObject::IWzResMa(const wchar_t* sUOL,_com_ptr_t* pObj,IUnknown* pUnkOuter)
	void(__cdecl* _PcCreateObject__IWzResMan)(const wchar_t* sUOL, void* pObj, void* pUnkOuter) = nullptr;

	// TMS113  int (__cdecl **)(int, int, int, int)
	// KMS1029 int (__stdcall *)(IWzResMan *, RESMAN_PARAM, int, int)
	// Fix MKD by convert __cdecl to __stdcall
	int(__stdcall* _IWzResMan__SetResManParam)(void* ecx, RESMAN_PARAM nParam, int nRetainTime, int nNameSpaceCacheTime) = nullptr;

	// TMS113  int __cdecl PcCreateObject::IWzNameSpace(int a1, _DWORD *a2, int a3)
	// KMS1029 void __cdecl PcCreateObject::IWzNameSpace(const wchar_t* sUOL, _com_ptr_t* pObj, IUnknown* pUnkOuter)
	void(__cdecl* _PcCreateObject__IWzNameSpace)(const wchar_t* sUOL, void* pObj, void* pUnkOuter);

	// TMS113  void (__cdecl *)(int *, int)
	// KMS1029 HRESULT __cdecl PcSetRootNameSpace(IUnknown *pNameSpace)
	void(__cdecl* _PcSetRootNameSpace)(void* ecx, int mode) = nullptr;

	// TMS113  int __cdecl PcCreateObject::IWzFileSystem(int a1, _DWORD *a2, int a3)
	// KMS1029 void __cdecl PcCreateObject::IWzFileSystem(const wchar_t* sUOL, _com_ptr_t* pObj, IUnknown* pUnkOuter)
	void(__cdecl* _PcCreateObject__IWzFileSystem)(const wchar_t* sUOL, void* pObj, void* pUnkOuter) = nullptr;

	// TMS113  _bstr_t *__thiscall _bstr_t::_bstr_t(_bstr_t *this, const char *a2)
	// KMS1029 _bstr_t *__thiscall _bstr_t::_bstr_t(_bstr_t *this, const char *a2)
	void* (__thiscall* _bstr_ctor)(void* ecx, const char* str) = nullptr;

	// TMS113  int __thiscall IWzFileSystem::Init(struct IUnknown *this, int *a2)
	// KMS1029 HRESULT __thiscall IWzFileSystem::Init(IWzFileSystem *this, Ztl_bstr_t sPath)
	void* (__thiscall* _IWzFileSystem__Init)(void* ecx, void* sPath) = nullptr;

	// TMS113  int __thiscall IWzNameSpace::Mount(struct IUnknown *this, int *a2, int a3, int a4)
	// KMS1029 HRESULT __thiscall IWzNameSpace::Mount(IWzNameSpace *this, Ztl_bstr_t sPath, IWzNameSpace *pDown, int nPriority)
	int(__thiscall* _IWzNameSpace__Mount)(void* ecx, void* sPath, void* pDown, int nPriority) = nullptr;

	// TMS113  void __fastcall CWvsApp::InitializeResMan(int a1, int a2)
	// KMS1029 void __thiscall CWvsApp::InitializeResMan(CWvsApp *this)
	void(__thiscall* _CWvsApp__InitializeResMan)(void* ecx) = nullptr;
	void __fastcall CWvsApp__InitializeResMan_Hook(void* ecx, void* edx) {
		// Generic
		void* pUnkOuter = nullptr;
		void* pGameFileSystem = nullptr;
		void* pDataFileSystem = nullptr;
		void* sPath = nullptr;
		int nPriority = 0;

		// PcCreateObject::IWzResMan
		_PcCreateObject__IWzResMan(L"ResMan", gResMan, pUnkOuter);
		// IWzResMan::SetResManParam
		ULONG_PTR uSetResManParamCall = *(ULONG_PTR*)*gResMan + 20; // This is a function ptr
		SADDR(&_IWzResMan__SetResManParam, *(ULONG_PTR*)uSetResManParamCall);
		_IWzResMan__SetResManParam(*gResMan, (RESMAN_PARAM)(RC_AUTO_REPARSE | RC_AUTO_SERIALIZE), -1, -1);

		// PcCreateObject::IWzNameSpace
		_PcCreateObject__IWzNameSpace(L"NameSpace", gRoot, pUnkOuter);
		ULONG_PTR uSetRootNameSpaceCall = *(ULONG_PTR*)pNameSpace; // This is a function ptr

		SADDR(&_PcSetRootNameSpace, uSetRootNameSpaceCall);
		_PcSetRootNameSpace(gRoot, 1);

		// Get the current dir of exe
		static char sStartPath[MAX_PATH];
		GetModuleFileNameA(NULL, sStartPath, MAX_PATH);
		CWvsApp::Dir_SlashToBackSlash(sStartPath);
		CWvsApp::Dir_upDir(sStartPath);

		// Game FileSystem
		_PcCreateObject__IWzFileSystem(L"NameSpace#FileSystem", &pGameFileSystem, pUnkOuter);

		_bstr_ctor(&sPath, sStartPath);

		auto iGameFS = _IWzFileSystem__Init(pGameFileSystem, sPath);

		_bstr_ctor(&sPath, "/");

		auto mGameFS = _IWzNameSpace__Mount(*gRoot, sPath, pGameFileSystem, nPriority);

		// Data FileSystem
		_PcCreateObject__IWzFileSystem(L"NameSpace#FileSystem", &pDataFileSystem, pUnkOuter);

		_bstr_ctor(&sPath, "./Data");

		auto iDataFS = _IWzFileSystem__Init(pDataFileSystem, sPath);

		_bstr_ctor(&sPath, "/");

		auto mDataFS = _IWzNameSpace__Mount(*gRoot, sPath, pDataFileSystem, nPriority);
	}
}

namespace ImgFile {
	bool Mount(Rosemary& r) {
		ULONG_PTR uAddress = 0;
		size_t index;
		size_t length = AOB_Scan_CWvsApp__InitializeResMan_Calls.size();
		for (index = 0; index < length; index++) {
			// CWvsApp::InitializeResMan base address
			uAddress = r.Scan(AOB_Scan_CWvsApp__InitializeResMan_Calls[index]); // TMS113(0x00A13020)
			if (uAddress > 0) {
				break;
			}
		}
		if (uAddress == 0) {
			DEBUG(L"Unable find CWvsApp::InitializeResMan");
			return false;
		}
		auto offsets = AOB_Scan_CWvsApp__InitializeResMan_Offsets[index];
		// PcCreateObject::IWzResMan
		ULONG_PTR uIWzResManCall = uAddress + offsets[0]; // TMS113(0x00A13086)
		ULONG_PTR uIWzResManAddress = GetFuncAddress(uIWzResManCall); // TMS113(0x00A170BD)
		SADDR(&_PcCreateObject__IWzResMan, uIWzResManAddress);
		// PcCreateObject::IWzNameSpace
		ULONG_PTR uIWzNameSpaceCall = uAddress + offsets[1]; // TMS113(0x00A13151)
		ULONG_PTR uIWzNameSpaceAddress = GetFuncAddress(uIWzNameSpaceCall); // TMS113(0x00A17122)
		SADDR(&_PcCreateObject__IWzNameSpace, uIWzNameSpaceAddress);
		// PcCreateObject::IWzFileSystem
		ULONG_PTR uIWzFileSystemCall = uAddress + offsets[2]; // TMS113(0x00A131F7)
		ULONG_PTR uIWzFileSystemAddress = GetFuncAddress(uIWzFileSystemCall); // TMS113(0x00A17187)
		SADDR(&_PcCreateObject__IWzFileSystem, uIWzFileSystemAddress);
		// _bstr_t::_bstr_t
		ULONG_PTR bstr_ctorCall = uAddress + offsets[3]; // TMS113(0x00A1326E)
		ULONG_PTR bstr_ctorAddress = GetFuncAddress(bstr_ctorCall);// TMS113(0x004063D7)
		SADDR(&_bstr_ctor, bstr_ctorAddress);
		// IWzFileSystem::Init
		ULONG_PTR uIWzFileSystemInitCall = uAddress + offsets[4]; // TMS113(0x00A1328F)
		ULONG_PTR uIWzFileSystemInitAddress = GetFuncAddress(uIWzFileSystemInitCall); // TMS113(0x00A13DBA)
		SADDR(&_IWzFileSystem__Init, uIWzFileSystemInitAddress);
		// IWzNameSpace::Mount
		ULONG_PTR uMountCall = uAddress + offsets[5]; // TMS113(0x00A134CB)
		ULONG_PTR uMountAddress = GetFuncAddress(uMountCall); // TMS113(0x00A13D60)
		if (uMountAddress == 0) {
			if (index == 1) {
				uMountAddress = GetFuncAddress(uMountCall + 3); // Work for TMS130-TMS136
			}
			else if (index == 3) {
				uMountAddress = GetFuncAddress(uMountCall + 6); // Work for TMS182-TMS192
			}
			else {
				DEBUG(L"uMountAddress is empty");
			}
		}
		SADDR(&_IWzNameSpace__Mount, uMountAddress);
		// CWvsApp::InitializeResMan Ready
		if (index == 0) {
			// TMS113-TMS122
			gResMan = (void**)(GetDWORDAddress(uIWzResManCall - 0xB)); // TMS113(0x00BF66F8)
			gRoot = (void**)(GetDWORDAddress(uIWzNameSpaceCall - 0xB)); // TMS113(0x00BF66F0)
			pNameSpace = (void**)(GetDWORDAddress(uIWzNameSpaceCall + 0x37)) + 4; // TMS113(0x00BF5ED0)
		}
		else if (index == 1 || index == 3) {
			// TMS123-TMS136
			// TMS152-TMS192
			gResMan = (void**)(GetDWORDAddress(GetFuncAddress(uIWzResManCall - 0x12)));
			gRoot = (void**)(GetDWORDAddress(GetFuncAddress(uIWzNameSpaceCall - 0x12)));
			pNameSpace = (void**)GetDWORDAddress(GetFuncAddress(uIWzNameSpaceCall + 0x24));
		}
		else if (index == 2) {
			// TMS139-TMS152
			gResMan = (void**)(GetDWORDAddress(uIWzResManCall - 0x9));
			gRoot = (void**)(GetDWORDAddress(uIWzNameSpaceCall - 0xA));
			pNameSpace = (void**)(GetFuncAddress(uIWzNameSpaceCall + 0x29));
		}
		else if (index == 4) {
			// TMS194
			gResMan = (void**)(GetDWORDAddress(uIWzResManCall - 0x9));
			gRoot = (void**)(GetDWORDAddress(uIWzNameSpaceCall - 0xD));
			pNameSpace = (void**)(GetFuncAddress(uIWzNameSpaceCall + 0x2A));
		}
		else {
			DEBUG(L"Unknown index in ImgFile Mount");
			return false;
		}
		SADDR(&_CWvsApp__InitializeResMan, uAddress);
		return SHOOK(true, &_CWvsApp__InitializeResMan, CWvsApp__InitializeResMan_Hook);
	}
}