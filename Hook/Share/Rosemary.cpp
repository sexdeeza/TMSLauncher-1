#include "Rosemary.h"
#include "AobScan.h"

Rosemary::Rosemary() {
	init = false;
}

Rosemary::~Rosemary() {
	section_list.clear();
	data_section_list.clear();
}

void Rosemary::Init(const std::vector<MODULEENTRY32W>* const gModuleEntryList, std::wstring wModuleName) {
	if (init) {
		return;
	}
	section_list.clear();
	data_section_list.clear();
	for (size_t i = 0; i < gModuleEntryList->size(); i++) {
		const MODULEENTRY32W& me32 = (*gModuleEntryList)[i];
		if (_wcsicmp(me32.szModule, wModuleName.c_str()) == 0) {
			MEMORY_BASIC_INFORMATION mbi;
			memset(&mbi, 0, sizeof(mbi));

			ULONG_PTR baseAddr = (ULONG_PTR)me32.modBaseAddr;
			ULONG_PTR endAddr = baseAddr + me32.modBaseSize;
			while (baseAddr < endAddr)
			{
				if (VirtualQuery((void*)baseAddr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
					if (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) {
						section_list.push_back(mbi);
					}
					// data section for string search
					if (section_list.size() && mbi.Protect & (PAGE_READWRITE | PAGE_WRITECOPY)) {
						// PAGE_READONLY and EXECUTE are currently ignored
						data_section_list.push_back(mbi);
					}
					baseAddr += mbi.RegionSize;
				}
			}

			if (!section_list.empty()) {
				init = true;
				return;
			}
		}
	}
}

bool Rosemary::IsInitialized() const {
	return init;
}

bool Rosemary::GetSectionList(std::vector<MEMORY_BASIC_INFORMATION>& vSection) {
	vSection = section_list;
	return true;
}

ULONG_PTR Rosemary::Scan(std::wstring wAob, int res) {
	if (!init) {
		return 0;
	}

	AobScan a(wAob);

	int count = 0;
	for (size_t i = 0; i < section_list.size(); i++) {
		for (ULONG_PTR uAddress = (ULONG_PTR)section_list[i].BaseAddress; uAddress < ((ULONG_PTR)section_list[i].BaseAddress + section_list[i].RegionSize); uAddress++) {
			if (a.Compare(uAddress)) {
				count++;
				if (res) {
					if (res != count) {
						continue;
					}
				}
				return uAddress;
			}
		}
	}

	return 0;
}

// ListScan
ULONG_PTR Rosemary::Scan(std::wstring wAobList[], size_t size, size_t& index, bool(*Scanner)(ULONG_PTR)) {
	ULONG_PTR uAddress = 0;

	index = -1;

	if (!init) {
		return 0;
	}

	for (size_t i = 0; i < size; i++) {

		if (!Scanner) {
			uAddress = Scan(wAobList[i]);
		}
		else {
			uAddress = Scan(wAobList[i], Scanner);
		}

		if (uAddress) {
			index = i;
			return uAddress;
		}
	}

	return 0;
}


ULONG_PTR Rosemary::Scan(std::wstring wAob, bool(*Scanner)(ULONG_PTR)) {
	if (!init) {
		return 0;
	}

	if (!Scanner) {
		return 0;
	}

	AobScan a(wAob);

	for (size_t i = 0; i < section_list.size(); i++) {
		for (ULONG_PTR uAddress = (ULONG_PTR)section_list[i].BaseAddress; uAddress < ((ULONG_PTR)section_list[i].BaseAddress + section_list[i].RegionSize); uAddress++) {
			if (a.Compare(uAddress)) {
				if (Scanner(uAddress)) {
					return uAddress;
				}
			}
		}
	}

	return 0;
}

bool Rosemary::WriteByte(ULONG_PTR uAddress, const unsigned char ucValue) {
	DWORD oldProtect = 0;
	size_t nSize = sizeof(ucValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(unsigned char*)uAddress = ucValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteBytes(ULONG_PTR uAddress, const unsigned char* ucValue) {
	if (ucValue == nullptr) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = sizeof(ucValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}
	memcpy((void*)uAddress, ucValue, nSize);

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::FillBytes(ULONG_PTR uAddress, const unsigned char ucValue, const int nCount) {
	DWORD oldProtect = 0;
	size_t nSize = sizeof(ucValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	memset((void*)uAddress, ucValue, nCount);

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteShort(ULONG_PTR uAddress, const unsigned short usValue) {
	DWORD oldProtect = 0;
	size_t nSize = sizeof(usValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(unsigned short*)uAddress = usValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteInt(ULONG_PTR uAddress, const unsigned int uiValue) {
	DWORD oldProtect = 0;
	size_t nSize = sizeof(uiValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(unsigned int*)uAddress = uiValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteLong(ULONG_PTR uAddress, const unsigned long ulValue) {
	DWORD oldProtect = 0;
	size_t nSize = sizeof(ulValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(unsigned long*)uAddress = ulValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteString(ULONG_PTR uAddress, const char* cValue) {
	if (cValue == nullptr) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = strlen(cValue) + 1;
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	memcpy((void*)uAddress, cValue, nSize);

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);
	return true;
}

bool Rosemary::WriteFloat(ULONG_PTR uAddress, const float fValue) {
	DWORD oldProtect = 0;
	size_t nSize = sizeof(fValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(float*)uAddress = fValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteDouble(ULONG_PTR uAddress, const double dValue) {
	DWORD oldProtect = 0;
	size_t nSize = sizeof(dValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(double*)uAddress = dValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::Backup(std::vector<MEMORY_BASIC_INFORMATION>& vSection, std::vector<void*>& vBackup) {
	vSection.clear();
	vBackup.clear();

	if (!init) {
		return false;
	}

	for (size_t i = 0; i < section_list.size(); i++) {
		void* memory = VirtualAlloc(NULL, section_list[i].RegionSize, MEM_COMMIT, PAGE_READWRITE);
		if (!memory) {
			vBackup.clear();
			return false;
		}
		for (size_t j = 0; j < section_list[i].RegionSize; j++) {
			// Ensure j does not exceed buffer size
			if (j >= section_list.size()) {
				break;  // Prevent buffer overrun
			}
			((BYTE*)memory)[j] = *(BYTE*)((ULONG_PTR)section_list[i].BaseAddress + j);
		}
		vBackup.push_back(memory);
	}

	vSection = section_list;
	return true;
}

bool Rosemary::Hook(ULONG_PTR uAddress, void* HookFunction, ULONG_PTR uNop) {
	DWORD oldProtect = 0;
	if (!VirtualProtect((void*)uAddress, 5 + uNop, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(BYTE*)uAddress = 0xE9;
	*(DWORD*)(uAddress + 0x01) = (DWORD)((ULONG_PTR)HookFunction - uAddress) - 0x05;

	for (size_t i = 0; i < uNop; i++) {
		((BYTE*)uAddress)[5 + i] = 0x90;
	}

	VirtualProtect((void*)uAddress, 5 + uNop, oldProtect, &oldProtect);
	return true;
}

bool Rosemary::JMP(ULONG_PTR uPrev, ULONG_PTR uNext, ULONG_PTR uNop) {
	DWORD oldProtect = 0;
	if (!VirtualProtect((void*)uPrev, 5 + uNop, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(BYTE*)uPrev = 0xE9;
	*(DWORD*)(uPrev + 0x01) = (DWORD)(uNext - uPrev) - 0x05;

	for (size_t i = 0; i < uNop; i++) {
		((BYTE*)uPrev)[5 + i] = 0x90;
	}

	VirtualProtect((void*)uPrev, 5 + uNop, oldProtect, &oldProtect);
	return true;
}

ULONG_PTR Rosemary::StringPatch(std::string search_string, std::string replace_string) {
	// including null
	size_t search_string_size = search_string.length() + 1;
	size_t replace_string_size = replace_string.length() + 1;

	if (search_string_size < replace_string_size) {
		// unsafe
		return 0;
	}
	for (size_t i = 0; i < data_section_list.size(); i++) {
		for (ULONG_PTR uAddress = (ULONG_PTR)data_section_list[i].BaseAddress; uAddress < ((ULONG_PTR)data_section_list[i].BaseAddress + data_section_list[i].RegionSize - search_string_size); uAddress++) {
			if (memcmp((void*)uAddress, search_string.c_str(), search_string_size) == 0) {
				memset((void*)uAddress, 0, search_string_size);
				memcpy_s((void*)uAddress, replace_string_size, replace_string.c_str(), replace_string_size);
				return uAddress;
			}
		}
	}
	// some packer's has execute flags
	for (size_t i = 0; i < section_list.size(); i++) {
		for (ULONG_PTR uAddress = (ULONG_PTR)section_list[i].BaseAddress; uAddress < ((ULONG_PTR)section_list[i].BaseAddress + section_list[i].RegionSize - search_string_size); uAddress++) {
			if (memcmp((void*)uAddress, search_string.c_str(), search_string_size) == 0) {
				memset((void*)uAddress, 0, search_string_size);
				memcpy_s((void*)uAddress, replace_string_size, replace_string.c_str(), replace_string_size);
				return uAddress;
			}
		}
	}
	return 0;
}