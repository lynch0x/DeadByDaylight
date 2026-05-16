#pragma once
#include "cheat.h"
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

namespace
{


	uintptr_t LocalPawn = 0;
	uintptr_t PlayerController = 0;




	static void ScanActorsWorker()
	{
		
		std::array<EspKnownActor,32> newlyFound;
		unsigned char length = 0;

		// Czytamy stan gry w tle, żeby nie obciążać RenderESP
		uintptr_t GWorld = dbd->Read<uintptr_t>((uintptr_t)dbd->GetProcessBase() + Offsets::GWorld);
		if (GWorld)
		{
			uintptr_t PersistentLevel = dbd->Read<uintptr_t>(GWorld + 0x0050);
			if (PersistentLevel)
			{
				TArray<uintptr_t> actors(dbd, PersistentLevel + 0x00C0);
				for (int i = 0; i < actors.Length; i++)
				{
					uintptr_t actor = actors.GetAtIndex(i);
					if (!actor) continue;

					if (AGeneratorKlass && IsA(actor, AGeneratorKlass)) { newlyFound[length] = { actor, AGeneratorKlass };length++; continue; }
					if (ASurvivorKlass && IsA(actor, ASurvivorKlass)) { newlyFound[length] = { actor, ASurvivorKlass };length++;  continue; }
					if (AKillerKlass && IsA(actor, AKillerKlass)) { newlyFound[length] = { actor, AKillerKlass };length++;  continue; }
					if (ATotemKlass && IsA(actor, ATotemKlass)) { newlyFound[length] = { actor, ATotemKlass };length++;  continue; }
					if (AHatch && IsA(actor, AHatch)) { newlyFound[length] = { actor, AHatch };length++;  continue; }
				}
			}
		}

	
  			for (size_t i = 0; i < length; i++)
			{
				knownActors.push_back(newlyFound[i]);
			}
			
	

	}

	static void ClearEspCaches()
	{
		knownActors.clear();
		boneCacheSize = 0;
	}
}
const char* TranslateCheatState()
{
	switch (CheatState)
	{
	case 1:
		return "Grabbing heavy stuff";
	case 2:
		return "READY";
	default:
		return "Waiting for DBD";
	
	}
}

void SkillcheckThread()
{
	while (!shouldQuit && SkillCheckThreadAlive)
	{
	

			if (LocalPawn)
			{


				uintptr_t interactionhandler = dbd->Read<uintptr_t>(LocalPawn + 0x0B60);
				if (interactionhandler) {
					uintptr_t skillCheck = dbd->Read<uintptr_t>(interactionhandler + 0x0358);
					if (skillCheck) {
						bool isDisplayed = dbd->Read<bool>(skillCheck + 0x1A8);
						if (isDisplayed) {
							//auto someptr = dbd->Read<unsigned char>(LocalPawn + 0x1D8);
							
							bool carried =  false;
							for (size_t i = 0; i < knownActors.size(); i++)
							{
								auto& actor = knownActors[i];
								if (actor.klass != AKillerKlass)continue;
								uintptr_t Killer = actor.actor;
								
									uintptr_t car = dbd->Read<uintptr_t>(Killer + 0x1A30);

									carried = car == LocalPawn;
									if (carried)break;
								
							}
							float currentProgress = dbd->Read<float>(skillCheck + 0x1AC);

							auto skillcheckDefinition = dbd->ReadAsReference<FSkillCheckDefinition>(skillCheck + 0x208);
							bool isNegativeProgressRate = skillcheckDefinition->ProgressRate < 0.f;

							if (carried) {
								float wiggleProgress = isNegativeProgressRate ? skillcheckDefinition->StartingTickerPosition + (1 - currentProgress) : currentProgress + skillcheckDefinition->StartingTickerPosition;

								if (wiggleProgress > 1.f)
									wiggleProgress -= 1.f;

								float bonusZoneStart = skillcheckDefinition->BonusZoneStart;
								float bonusZoneEnd = bonusZoneStart + skillcheckDefinition->BonusZoneLength;

								if (wiggleProgress > bonusZoneStart && wiggleProgress < bonusZoneEnd) {
									sendSpaceCommand();
								}
							}
							else {
								float skillCheckStartZone = skillcheckDefinition->SuccessZoneStart - skillcheckDefinition->StartingTickerPosition;
								float skillCheckEndZone = skillcheckDefinition->SuccessZoneEnd - skillcheckDefinition->StartingTickerPosition;
								if (skillcheckDefinition->BonusZoneLength > 0)
									skillCheckStartZone = skillcheckDefinition->BonusZoneStart - skillcheckDefinition->StartingTickerPosition;

								float startRange = isNegativeProgressRate ? 1 - skillCheckEndZone : skillCheckStartZone;
								float endRange = isNegativeProgressRate ? 1 - skillCheckStartZone : skillCheckEndZone;
								if (currentProgress > startRange) {
									sendSpaceCommand();
								}
							}
						}
					}
				}
			
			
		}

			Sleep(10);
	}
}

void CheatThread()
{
	
	knownActors.reserve(32);
	dbd->GetProcessBase();
	while (!shouldQuit)
	{
		if (!dbd->CheckGameAlive())
		{
			shouldQuit = true;
			break;
		}
		else {
			if (CheatState == 0) {

				uintptr_t GWorld = 0;
				dbd->drv.ReadProcessMemory(dbd->dtb, (uintptr_t)dbd->GetProcessBase() + Offsets::GWorld,&GWorld,8 );
				if (GWorld) {

					CheatState = 1;
				}
			}
			if (CheatState == 1) {
				GObjectsPool pool;
				uintptr_t a = pool.GetByIndex(dbd, 0x00007FFB);
				void* ptr = dbd->Read<void*>(a + 0xf8);
				//	const unsigned char patch[] = { 0xE9, 0x11, 0x02, 0x00, 0x00, 0x90 };
				//	dbd->WriteBytes((uintptr_t)dbd->GetProcessBase() + 0x5ce0c52,(PVOID)&patch[0], sizeof(patch));
				
				const char ToSearchCount = 5;
				

				SearchKey ar[ToSearchCount] =
				{
					{"Survivor",&ASurvivorKlass},
					{"Killer",&AKillerKlass},
					{"Generator",&AGeneratorKlass},
					{"Totem",&ATotemKlass},
					{"Hatch",&AHatch}
				};
				pool.GetManyByNames(dbd, ar, ToSearchCount);
				bool IsAllowedToChangeCheatState = true;
				for (unsigned char i = 0; i < ToSearchCount; i++)
				{
					SearchKey loc = ar[i];
					if (*loc.Return == 0)
					{
						IsAllowedToChangeCheatState = false;
					}
				}
				if (IsAllowedToChangeCheatState) {

					CheatState = 2;
				}
			}
			if (CheatState == 2) {
				uintptr_t GWorld = dbd->Read<uintptr_t>((uintptr_t)dbd->GetProcessBase() + Offsets::GWorld);
				if (GWorld) {
					static uintptr_t lastWorld = 0;
					static uintptr_t lastLevel = 0;
					uintptr_t PersistentLevel = dbd->Read<uintptr_t>(GWorld + 0x0050);
					if (PersistentLevel && (GWorld != lastWorld || PersistentLevel != lastLevel))
					{
						LocalPawn = 0;
						PlayerController = 0;
						{
							std::lock_guard<std::mutex> lock(mtx);
							ClearEspCaches();
						}
						dbd->ClearMappings();
					}
					if (PersistentLevel && (GWorld != lastWorld || PersistentLevel != lastLevel) && IsWorldLoaded(GWorld))
					{
						lastWorld = GWorld;
						lastLevel = PersistentLevel;
						{
							std::lock_guard<std::mutex> lock(mtx);
							ScanActorsWorker();
						}
						uintptr_t GameInstance = dbd->Read<uintptr_t>(GWorld + 0x250);
						TArray<uintptr_t> localPlayers(dbd, GameInstance + 0x58);
						if (localPlayers.Length > 0)
						{
							uintptr_t myPlayer = localPlayers.GetAtIndex(0);
							if (myPlayer) {
								PlayerController = dbd->Read<uintptr_t>(myPlayer + 0x50);
								if (PlayerController)
								{
									LocalPawn = dbd->Read<uintptr_t>(PlayerController + 0x03A0);
									if (LocalPawn)
									{
										uintptr_t playerStateCache = dbd->Read<uintptr_t>(LocalPawn + 0x06A0);
										if (playerStateCache) {

											unsigned char role = dbd->Read<unsigned char>(playerStateCache + 0x03DA);

											if (!SkillCheckThreadAlive)
											{
												if (role == 2) {
													SkillCheckThreadAlive = true;
													std::thread l(SkillcheckThread);
													l.detach();
												}
											}
											else {
												if (role != 2) {
													SkillCheckThreadAlive = false;
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


		}
			
		
		Sleep(1500);
	}
}
void RenderMenu() {
	if (showMenu) {
		ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
		if (!ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove)) {
			
			ImGui::End();
			return;
		}
		ImGui::Text(TranslateCheatState());
		
		ImGui::End();
	}
}
void RenderBlindable()
{
	if (!LocalPawn)
		return;

	constexpr float width = 200.0f;
	constexpr float height = 25.0f;

	ImGui::SetNextWindowPos(ImVec2(
		(screenWidth - width) * 0.5f,
		40.0f
	));

	ImGui::Begin(
		"BlindOverlay",
		nullptr,
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_AlwaysAutoResize
	);
	for (size_t i = 0; i < knownActors.size(); i++)
	{
		auto& actor = knownActors[i];
		if (actor.klass != AKillerKlass)continue;
		uintptr_t killer = actor.actor;

		if (killer == LocalPawn)
			continue;

		uintptr_t blindChargable =
			dbd->Read<uintptr_t>(killer + 0x0D30);

		if (!blindChargable)
			continue;

		float current_charge =
			dbd->Read<float>(blindChargable + 0x1C8 + 0x1C);



		ImGui::Text("Killer blind progress");
		ImGui::ProgressBar(
			current_charge,
			ImVec2(width, height)
		);

	}
	

	ImGui::End();
}
void RenderESP() {
	if (CheatState == 2) {


		if (LocalPawn )
		{




			
			FMinimalViewInfo POV = GetMyPov(PlayerController);
	

		//	std::lock_guard<std::mutex> lock(mtx);

			FVector LocalPlayerLocation = GetActorLocation(LocalPawn);
			for (size_t k = 0; k < knownActors.size(); k++)
			{
				const auto& entry = knownActors[k];
		
				if (entry.actor == 0)continue;
				if (!dbd->Read<uintptr_t>(entry.actor))
				{
					knownActors[k].actor = 0;
					continue;
				}
					

				//if (!entry.klass || !IsA(actor, entry.klass)) continue;

				FVector vec = GetActorLocation(entry.actor);
				FVector2D screenPosition;
				
				if (LocalPawn && WorldToScreenPoint(vec, POV, screenPosition) )
				{
					if (entry.klass == AGeneratorKlass)
					{


						auto chargable = dbd->Read<uintptr_t>(entry.actor + 0x0508);
						auto max = dbd->Read<float>(chargable + 0x20);
						auto current_charge = dbd->Read<float>(chargable + 0x1C8 + 0x1C);
						/*auto generator_charge = DBD::read<uintptr_t>(actor + 0x04F0);
						auto current_charge = DBD::read<float>(generator_charge + 0x01B0 + 0x18);*/
						int generator_percent = (current_charge / 90) * 100;
						auto lol = "Generator [" + std::to_string(generator_percent) + "%]";
						DistanceText(lol, vec, LocalPlayerLocation);
						DrawESPText(lol.c_str(), ImVec2(screenPosition.X, screenPosition.Y), ImColor(105, 202, 255));

					}
					if (entry.klass == ASurvivorKlass)
					{
						if (entry.actor != LocalPawn)
						{
							uintptr_t plrstate = GetPlayerState(entry.actor);
							if (!plrstate)continue;

							const std::wstring_view nickname = GetUsername(plrstate);
							if (!nickname.empty()) {

								
								//DistanceText(tmp, vec, LocalPlayerLocation);
								const std::string utf8 = Utf8FromView(nickname);
								DrawESPText(utf8.c_str(), ImVec2(screenPosition.X, screenPosition.Y), IM_COL32(0, 255, 0, 255));


							}
							else {

							
								DrawESPText("BOT", ImVec2(screenPosition.X, screenPosition.Y), IM_COL32(0, 255, 0, 255));


							}
							DrawSkeleton(entry.actor, POV, IM_COL32(0, 255, 0, 255));
						}

					}
					if (entry.klass == AKillerKlass)
					{
						if (entry.actor != LocalPawn)
						{
							
							std::string tmp("Killer");
							DistanceText(tmp, vec, LocalPlayerLocation);
							DrawESPText(tmp.c_str(), ImVec2(screenPosition.X, screenPosition.Y), IM_COL32(255, 0, 0, 255));

							DrawSkeleton(entry.actor, POV, IM_COL32(255, 0, 0, 255));

						}
					}
					if (entry.klass == ATotemKlass)
					{

						ETotemState state = (ETotemState)dbd->Read<unsigned char>(entry.actor + 0x0450);
						if (state == ETotemState::Hex) {

							std::string tmp("Totem[HEX]");
							DistanceText(tmp, vec, LocalPlayerLocation);
							DrawESPText(tmp.c_str(), ImVec2(screenPosition.X, screenPosition.Y + 10), ImColor(159, 61, 179));



						}
					}
					if (entry.klass == AHatch)
					{
						std::string tmp("!!!HATCH!!!");
						DistanceText(tmp, vec, LocalPlayerLocation);

						DrawESPText(tmp.c_str(), ImVec2(screenPosition.X, screenPosition.Y), ImColor(235, 52, 131));
					}
				}
				//	}



			}


		}
	}


		
}