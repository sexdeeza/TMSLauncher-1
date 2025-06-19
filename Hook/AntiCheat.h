#pragma once
#include "Share/Rosemary.h"

namespace AntiCheat {
	bool RemoveLocaleCheck(Rosemary& r);
	bool RemoveSecurityClient(Rosemary& r);
	bool RemoveRenderFrameCheck(Rosemary& r);
	bool RemoveEnterFieldCheck(Rosemary& r);
}