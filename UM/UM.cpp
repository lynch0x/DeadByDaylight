
#include <iostream>
#include "dbd.h"
#include "offsets.h"
#include "structs.h"
void sendSpaceCommand()
{
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_SPACE;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SPACE;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}
bool IsProbablyValidAddress(uintptr_t addr) {
	return (addr >= 0x10000 && addr <= 0x00007FFFFFFFFFFF);
}
bool IsA(DeadByDaylight& dbd,uintptr_t actor, uintptr_t klass) {
	uintptr_t temp = dbd.Read<uintptr_t>(actor + 0x10);
//	while (IsProbablyValidAddress(temp)) {
		if (temp == klass) {
			return true;
		}
		temp = dbd.Read<uintptr_t>(temp + 0x60);
		if (temp == klass) {
			return true;
		}
		temp = dbd.Read<uintptr_t>(temp + 0x60);
		if (temp == klass) {
			return true;
		}
//	}
	return false;
}
uintptr_t FindSingleActorOfType(DeadByDaylight& dbd,uintptr_t level,uintptr_t klass) {

	unsigned int actorsCount =dbd.Read<unsigned int>(level + 0x00C0 + 0x8);
	if (actorsCount > 100 && actorsCount < 3000) {
		uintptr_t actorsArray = dbd.Read<uintptr_t>(level + 0x00C0);
		for (unsigned int i = 0; i < actorsCount; i++)
		{

			uintptr_t actor = dbd.Read<uintptr_t>(actorsArray + i * 0x8);
		
			if (IsA(dbd,actor, klass)) {
				return actor;
			}
		}
	}
	return 0;
}
uintptr_t GetUObjectFromPool(DeadByDaylight& dbd,uintptr_t base,int index) {
	uintptr_t gObjectsPtr = (uintptr_t)base + Offsets::GObjects;
	uintptr_t gObjects =dbd.Read<uintptr_t>(gObjectsPtr);
	uintptr_t objArray = dbd.Read<uintptr_t>(gObjects + 0x0);
	uintptr_t objectItem = objArray + (index * 0x18);
	uintptr_t objectPtr = dbd.Read<uintptr_t>(objectItem);
	return objectPtr;
}
static uintptr_t killer = 0;
static  uintptr_t killerKlass = 0;
bool IsKillerCarryingMe(DeadByDaylight& dbd,uintptr_t base,uintptr_t persistent, uintptr_t mypawn) {
	
	if (killer) {
		uintptr_t carried = dbd.Read<uintptr_t>(killer + 0x1A68);
		return mypawn == carried;
	}
	return false;
}
static uintptr_t lastPersistent = 0;
int main()
{
	DeadByDaylight dbd;
	if (dbd.handle == INVALID_HANDLE_VALUE) {
		std::cout << "Could not open ioctl\n";
		return 0;
	}
	HANDLE base = dbd.GetProcessBase();
	while (true) {
		uintptr_t GWorld = dbd.Read<uintptr_t>((uintptr_t)base + Offsets::GWorld);
		if (GWorld) {
			uintptr_t persistent = dbd.Read<uintptr_t>(GWorld + 0x0050);
			if (IsProbablyValidAddress(persistent) && persistent != lastPersistent) {
				lastPersistent = persistent;
				if (killerKlass == 0) {
					killerKlass = GetUObjectFromPool(dbd, (uintptr_t)base, 0x0000113C);
				}
			    killer = FindSingleActorOfType(dbd, persistent, killerKlass);
			}
			uintptr_t gameInstance = dbd.Read<uintptr_t>(GWorld + 0x0200);
			if (gameInstance) {
				uintptr_t localPlayersArray = dbd.Read<uintptr_t>(gameInstance + 0x0058);
				if (localPlayersArray) {
					uintptr_t localPlayer = dbd.Read<uintptr_t>(localPlayersArray);
					if (localPlayer) {
						uintptr_t playerController = dbd.Read<uintptr_t>(localPlayer + 0x0050);
						if (playerController) {
							uintptr_t pawn = dbd.Read<uintptr_t>(playerController + 0x0388);
							if (pawn) {

								uintptr_t interactionhandler = dbd.Read<uintptr_t>(pawn + 0x0B98);
								uintptr_t skillCheck = dbd.Read<uintptr_t>(interactionhandler + 0x0338);
								if (skillCheck) {
									bool isDisplayed = dbd.Read<bool>(skillCheck + 0x191);
									if (isDisplayed) {
										float currentProgress = dbd.Read<float>(skillCheck + 0x194);
										auto skillcheckDefinition = dbd.Read<FSkillCheckDefinition>(skillCheck + 0x200);
										bool isNegativeProgressRate = skillcheckDefinition.ProgressRate < 0.f;

										
										if (IsKillerCarryingMe(dbd, (uintptr_t)base, persistent, pawn)) {
											float wiggleProgress = isNegativeProgressRate ? skillcheckDefinition.StartingTickerPosition + (1 - currentProgress) : currentProgress + skillcheckDefinition.StartingTickerPosition;

											if (wiggleProgress > 1.f)
												wiggleProgress -= 1.f;

											float bonusZoneStart = skillcheckDefinition.BonusZoneStart;
											float bonusZoneEnd = bonusZoneStart + skillcheckDefinition.BonusZoneLength;

											if (wiggleProgress > bonusZoneStart && wiggleProgress < bonusZoneEnd) {
												sendSpaceCommand();
											}
										}
										else {
											float skillCheckStartZone = skillcheckDefinition.SuccessZoneStart - skillcheckDefinition.StartingTickerPosition;
											float skillCheckEndZone = skillcheckDefinition.SuccessZoneEnd - skillcheckDefinition.StartingTickerPosition;
											if (skillcheckDefinition.BonusZoneLength > 0)
												skillCheckStartZone = skillcheckDefinition.BonusZoneStart - skillcheckDefinition.StartingTickerPosition;

											float startRange = isNegativeProgressRate ? 1 - skillCheckEndZone : skillCheckStartZone;
											float endRange = isNegativeProgressRate ? 1 - skillCheckStartZone : skillCheckEndZone;
											if (currentProgress > startRange) {
												std::cout << "Skillcheck!\n";
												sendSpaceCommand();
											}
										}

									}
								}
							}
						}
					}
				}
			}
		}
		Sleep(100);
	}
}