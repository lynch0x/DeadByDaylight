#pragma once
#include "dbd.h"
#include <array>
#include <vector>
#include <string>
#include <mutex>
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
template<typename T, int N>
struct StaticVector {
    std::array<T, N> data;
    int size = 0;

    void push_back(const T& v) {
        if (size < N)
            data[size++] = v;
    }

    void clear() {
        size = 0;
    }
};
struct EspKnownActor
{
    uintptr_t actor = 0;
    uintptr_t klass = 0;
};
inline std::array<BoneCacheEntry, 12> boneCache;
inline size_t boneCacheSize = 0;
inline std::vector<EspKnownActor> knownActors;
inline std::mutex mtx;