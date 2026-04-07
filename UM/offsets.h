#pragma once
namespace Offsets
{
#if _STEAM || _DEBUG
	constexpr uintptr_t GObjects = 0x0BD9FE00;
	constexpr uintptr_t GNames = 0x0BCCC780;
	constexpr uintptr_t GWorld = 0x0BF58D60;
#endif
#if _EPIC
	constexpr uintptr_t GObjects = 0xb793cc0;
	constexpr uintptr_t GNames = 0xB6C05C0;
	constexpr uintptr_t GWorld = 0xB94B8D0;
#endif
}