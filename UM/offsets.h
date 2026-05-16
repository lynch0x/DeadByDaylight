#pragma once
namespace Offsets
{
#if _STEAM || _DEBUG
	constexpr uintptr_t GObjects = 0x0D0F5BE0;
	constexpr uintptr_t GNames = 0x0CFED7C0;
	constexpr uintptr_t GWorld = 0x0CD8EEA0;
	constexpr uintptr_t SSL = 0xB9d3fd0;
#endif
#if _EPIC
	constexpr uintptr_t GObjects = 0xc75b4e0;
	constexpr uintptr_t GNames = 0xc61a058;
	constexpr uintptr_t GWorld = 0xc435e80;
#endif
}