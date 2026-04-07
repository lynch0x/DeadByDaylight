#pragma once
#include "external/imgui.h"
#include "shared.h"
#include "engine.h"
#include "offsets.h"
#include "methods.h"
inline uintptr_t LocalPawn=0;
inline uintptr_t PlayerController=0;
inline uintptr_t ASurvivorKlass=0;
inline uintptr_t AGeneratorKlass=0;
inline uintptr_t ATotemKlass=0;
inline uintptr_t LastPersistentLevel = 0;
inline uintptr_t AKillerKlass=0;
inline std::vector<uintptr_t> generators{};
inline std::vector<uintptr_t> survivors{};
inline std::vector<uintptr_t> killers{};
inline std::vector<uintptr_t> totems{};
const char* TranslateCheatState();
void RenderMenu();
void RenderESP();
void CheatThread();