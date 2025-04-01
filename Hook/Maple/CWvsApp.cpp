#include <cstring>
#include "CWvsApp.h"

// Convert '/' to '\\'
void CWvsApp::Dir_SlashToBackSlash(char* sDir) {
	if (sDir == nullptr) {
		return;
	}
	size_t length = strlen(sDir);
	for (size_t i = 0; i < length; ++i) {
		if (sDir[i] == '/') {
			sDir[i] = '\\';
		}
	}
}

void CWvsApp::Dir_upDir(char* sDir) {
	if (sDir == nullptr) {
		return;
	}
	size_t length = strlen(sDir);
	if (length > 0 && sDir[length - 1] == '/') {
		sDir[length - 1] = '\0';
	}
	size_t i = length - 1;
	if (i > 0) {
		while (sDir[i] != '/') { 
			if (--i <= 0) { 
				return; 
			} 
		}
		sDir[i] = '\0';
	}
}