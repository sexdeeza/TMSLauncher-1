#pragma once
#include "Share/Rosemary.h"

namespace Screen {
	bool FixDomain(Rosemary& r); // Fix domain resolve error
	bool FixWindowMode(Rosemary& r); // Fix window mode error
	void FixMinimizeButton(DWORD& dwStyle); // Fix the minimize button isn't show
}