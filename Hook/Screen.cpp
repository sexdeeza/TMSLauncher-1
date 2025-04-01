#include "Screen.h"
#include "AobList.h"
#include "Tool.h"

namespace Screen {
	bool FixDomain(Rosemary& r) {
		// TMS old default DNS (Unable show ad window if not set)
		return r.StringPatch("tw.login.maplestory.gamania.com", "127.0.0.1");
		// Not need this because HS has been removed
		//r.StringPatch("tw.hackshield.gamania.com", "202.80.106.36");
	}

	bool FixWindowMode(Rosemary& r) {
		// Window mode is needed to run old client with recent displays
		// or change display settings 59.9 fps to 60.1 fps, 60.1 fps may allow to use full screen mode
		ULONG_PTR uAddress = r.Scan(AOB_Scan_Window_Mode);
		if (uAddress == 0) {
			return false;
		}
		ULONG_PTR Window_Mode = uAddress + 0x03;
		return r.WriteByte(Window_Mode, 0x00);
	}

	void FixMinimizeButton(DWORD& dwStyle) {
		// TMS113-TMS118 don't show the minimize button when using window mode
		dwStyle |= WS_MINIMIZEBOX;
	}
}