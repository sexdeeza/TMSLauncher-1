#pragma once

namespace Wnd {
	bool FixWindowMode(Rosemary& r); // Allow older clients(BeforeBB) to run at the recent displays
	void FixMinimizeButton(DWORD& dwStyle); // Fix the minimize button isn't show
}