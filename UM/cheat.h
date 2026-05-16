#pragma once
#include "external/imgui.h"
#include "shared.h"
#include "engine.h"
#include "offsets.h"
#include "methods.h"
#include "structs.h"

inline bool SkillCheckThreadAlive = false;

inline uintptr_t ASurvivorKlass=0;
inline uintptr_t AGeneratorKlass=0;
inline uintptr_t AHatch=0;
inline uintptr_t ATotemKlass=0;
inline uintptr_t AKillerKlass=0;

const char* TranslateCheatState();
void RenderMenu();
void RenderESP();
void CheatThread();
void SkillcheckThread();
void RenderBlindable();
