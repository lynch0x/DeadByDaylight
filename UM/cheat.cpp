#pragma once
#include "cheat.h"
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
void CheatThread()
{
	while (!shouldQuit)
	{
		if (!dbd->CheckGameAlive())
		{
			shouldQuit = true;
			break;
		}
		else {
			if (CheatState == 0) {
				
				uintptr_t GWorld = dbd->Read<uintptr_t>((uintptr_t)dbd->GetProcessBase() + Offsets::GWorld);
				if (GWorld) {
					
					CheatState = 1;
				}
			}
			if (CheatState==1){
				const char ToSearchCount = 4;
				GObjectsPool pool;
				SearchKey ar[ToSearchCount] =
				{
					{"Survivor",&ASurvivorKlass},
					{"Killer",&AKillerKlass},
					{"Generator",&AGeneratorKlass},
					{"Totem",&ATotemKlass}
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
				if(IsAllowedToChangeCheatState)
				CheatState = 2;
			}
			if (CheatState == 2) {
				uintptr_t GWorld = dbd->Read<uintptr_t>((uintptr_t)dbd->GetProcessBase() + Offsets::GWorld);
				if (GWorld) {
					uintptr_t PersistentLevel = dbd->Read<uintptr_t>(GWorld + 0x0050);
					if (PersistentLevel != 0 && PersistentLevel != LastPersistentLevel) {
						LocalPawn = 0;
						LastPersistentLevel = PersistentLevel;
						Sleep(10000);
							auto actors = TArray<uintptr_t>(dbd, PersistentLevel + 0x00C0);
							//TODO: Make array refreshing because it gets not fully loaded array (instead of sleep)
							if (actors.Length > 100) {
								generators.clear();
								survivors.clear();
								killers.clear();
								totems.clear();
								for (int i = 0; i < actors.Length; i++)
								{
									uintptr_t actor = actors.GetAtIndex(i);
									if (!actor)continue;
									if (IsA(actor, AGeneratorKlass))
									{
										generators.push_back(actor);
									}
									if (IsA(actor, ASurvivorKlass))
									{
										survivors.push_back(actor);
									}
									if (IsA(actor, AKillerKlass))
									{
										killers.push_back(actor);
									}
									if (IsA(actor, ATotemKlass))
									{
										totems.push_back(actor);
									}
								}
								
							}
						
					}
					uintptr_t GameInstance = dbd->Read<uintptr_t>(GWorld + 0x200);
					if (GameInstance) {
						uintptr_t LocalPlayersPtr = dbd->Read<uintptr_t>(GameInstance + 0x58);
						uintptr_t LocalPlayer = dbd->Read<uintptr_t>(LocalPlayersPtr);
					    PlayerController = dbd->Read<uintptr_t>(LocalPlayer + 0x50);
						LocalPawn = dbd->Read<uintptr_t>(PlayerController + 0x388);
					}
				

				}
			}
		}
		Sleep(100);
	}
}
void RenderMenu() {
	if (showMenu) {
		if (!ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
			ImGui::End();
			return;
		}
		ImGui::Text(TranslateCheatState());
		ImGui::End();
	}
}
void RenderESP() {
	if (LocalPawn)
	{
		FMinimalViewInfo POV = GetMyPov();
		FVector LocalPlayerLocation = GetActorLocation(LocalPawn);
		for (int i = 0; i < generators.size(); i++)
		{
			uintptr_t actor = generators.at(i);
			FVector vec = GetActorLocation(actor);
			FVector2D screenPosition = WorldToScreenPoint(vec, POV);
			if (screenPosition.X > 0) {
				auto current_charge = dbd->Read<float>(actor + 0x7b4);
				/*auto generator_charge = DBD::read<uintptr_t>(actor + 0x04F0);
				auto current_charge = DBD::read<float>(generator_charge + 0x01B0 + 0x18);*/
				auto generator_percent = static_cast<int>(current_charge * 100.f);
				auto lol = "Generator [" + std::to_string(generator_percent) + "%]";
				DistanceText(lol, vec, LocalPlayerLocation);
				DrawESPText(lol.c_str(), ImVec2(screenPosition.X, screenPosition.Y), ImColor(105, 202, 255));
			}
		}
		for (int i = 0; i < survivors.size(); i++)
		{
			uintptr_t actor = survivors.at(i);
	

			if (actor != LocalPawn && !IsDeadOrInParadise(actor)) {
				uintptr_t plrstate = GetPlayerState(actor);
				if (!plrstate)continue;
				FVector vec = GetActorLocation(actor);
				FVector2D screenPosition = WorldToScreenPoint(vec, POV);
				if (screenPosition.X > 0) {
					std::string nickname = GetUsername(plrstate);
					if (!nickname.empty()) {
						
							std::string tmp(nickname);
							DistanceText(tmp, vec, LocalPlayerLocation);
							DrawESPText(tmp.c_str(), ImVec2(screenPosition.X, screenPosition.Y), IM_COL32(0, 255, 0, 255));

						
					}
					else {
						if (screenPosition.X > 0) {
							std::string tmp("BOT");
							DistanceText(tmp, vec, LocalPlayerLocation);
							DrawESPText(tmp.c_str(), ImVec2(screenPosition.X, screenPosition.Y), IM_COL32(0, 255, 0, 255));

						}
					}
					//DrawSkeleton(actor, POV);
				}
			}
		}
		for (int i = 0; i < killers.size(); i++)
		{
			uintptr_t actor = killers.at(i);


			if (actor != LocalPawn) {
				
				FVector vec = GetActorLocation(actor);
				FVector2D screenPosition = WorldToScreenPoint(vec, POV);
				if (screenPosition.X > 0) {
					
							std::string tmp("Killer");
							DistanceText(tmp, vec, LocalPlayerLocation);
							DrawESPText(tmp.c_str(), ImVec2(screenPosition.X, screenPosition.Y), IM_COL32(255, 0, 0, 255));

					DrawSkeleton(actor, POV);
				}
			}
		}
	}
}