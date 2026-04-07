#pragma once
#include "dbd.h"
#include <vector>
inline DeadByDaylight* dbd = 0;
inline int screenWidth = 0;
inline int screenHeight = 0;
inline bool showMenu = true;
inline bool shouldQuit = false;
inline unsigned char CheatState = 0;
struct BoneCacheEntry {
	uintptr_t actor;
	int bones[16];
};
inline std::vector<BoneCacheEntry> boneCache{};