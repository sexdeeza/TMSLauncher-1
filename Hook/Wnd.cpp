#include "Wnd.h"

#include "Resources/AOBList.h"
#include "Share/Tool.h"

namespace Wnd {

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