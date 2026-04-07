#pragma once

struct FSkillCheckDefinition final
{
public:
	float                                         SuccessZoneStart;                                  // 0x0000(0x0004)(ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         SuccessZoneEnd;                                    // 0x0004(0x0004)(ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         BonusZoneLength;                                   // 0x0008(0x0004)(ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         BonusZoneStart;                                    // 0x000C(0x0004)(ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         ProgressRate;                                      // 0x0010(0x0004)(ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         StartingTickerPosition;                            // 0x0014(0x0004)(ZeroConstructor, Transient, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};
enum class ESkillCheckCustomType : uint8_t
{
	VE_None = 0,
	VE_OnPickedUp = 1,
	VE_OnAttacked = 2,
	VE_DecisiveStrikeWhileWiggling = 3,
	VE_GeneratorOvercharge1 = 4,
	VE_GeneratorOvercharge2 = 5,
	VE_GeneratorOvercharge3 = 6,
	VE_BrandNewPart = 7,
	VE_Struggle = 8,
	VE_OppressionPerkGeneratorKicked = 9,
	VE_SoulChemical = 10,
	VE_Wiggle = 11,
	VE_YellowGlyph = 12,
	VE_K27P03Continuous = 13,
	VE_Continuous = 14,
	VE_S42P02 = 15,
	VE_K38P03Continuous = 16,
	VE_SnapOutOfIt = 17,
	VE_MAX = 18,
};
enum class ETotemState : uint8_t
{
	Cleansed = 0,
	Dull = 1,
	Hex = 2,
	Boon = 3,
	ETotemState_MAX = 4,
};

enum class EPalletState : uint8_t
{
	Up = 0,
	Falling = 1,
	Fallen = 2,
	Destroyed = 3,
	EPalletState_MAX = 4,
};
