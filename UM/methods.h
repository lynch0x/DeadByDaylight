#pragma once
#include "shared.h"
#pragma region Calculations
#define UCONST_Pi			3.1415926
#define URotationToRadians  UCONST_Pi / 180
inline FVector RotationToVector(FRotator R)
{
	FVector Vec;
	float fYaw = R.Yaw * URotationToRadians;
	float fPitch = R.Pitch * URotationToRadians;
	float CosPitch = cos(fPitch);
	Vec.X = cos(fYaw) * CosPitch;
	Vec.Y = sin(fYaw) * CosPitch;
	Vec.Z = sin(fPitch);

	return Vec;
}

inline float Size(FVector& v)
{
	return sqrt(v.X * v.X + v.Y * v.Y + v.Z * v.Z);
}

inline void Normalize(FVector& v)
{
	float size = Size(v);

	if (!size)
		v.X = v.Y = v.Z = 1;
	else
	{
		v.X /= size;
		v.Y /= size;
		v.Z /= size;
	}
}

extern FVector VectorSubtract(FVector va, FVector vb);

inline void GetAxes(FRotator R, FVector& X, FVector& Y, FVector& Z)
{
	float fYaw = R.Yaw * (UCONST_Pi / 180.f);
	float fPitch = R.Pitch * (UCONST_Pi / 180.f);

	float sp = sinf(fPitch), cp = cosf(fPitch);
	float sy = sinf(fYaw), cy = cosf(fYaw);

	// Forward — bez Normalize, bez RotationToVector
	X = { cp * cy, cp * sy, sp };

	// Right — dokładnie 90° w osi yaw
	Y = { -sy, cy, 0.f };

	// Up — iloczyn wektorowy X × Y
	Z = { -sp * cy, -sp * sy, cp };
}

inline FVector VectorSubtract(FVector va, FVector vb)
{
	FVector out;

	out.X = va.X - vb.X;
	out.Y = va.Y - vb.Y;
	out.Z = va.Z - vb.Z;

	return out;
}

float inline Dot(const FVector& V1, const FVector& V2)
{
	return (V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z);
}


inline bool WorldToScreenPoint(const FVector& Location, const FMinimalViewInfo& cam_cache, FVector2D& OutScreen)
{
	FVector Delta = VectorSubtract(Location, cam_cache.Location);

	// Najpierw tylko forward — najtańszy early exit
	float fYaw = cam_cache.Rotation.Yaw * (UCONST_Pi / 180.f);
	float fPitch = cam_cache.Rotation.Pitch * (UCONST_Pi / 180.f);

	float sp = sinf(fPitch), cp = cosf(fPitch);
	float sy = sinf(fYaw), cy = cosf(fYaw);

	// Z (głębokość) = dot(Delta, Forward)
	float z = Delta.X * (cp * cy)
		+ Delta.Y * (cp * sy)
		+ Delta.Z * sp;

	if (z < 1.f)
		return false; // za kamerą — exit przed liczeniem Right/Up i tan()

	float f = (screenWidth * 0.5f) / tanf(cam_cache.FOV * (UCONST_Pi / 360.f));
	float fOverZ = f / z;

	// X (prawo) = dot(Delta, Right)
	float x = Delta.X * (-sy)
		+ Delta.Y * cy;
	// Y.Z = 0 więc Delta.Z pomijamy

	OutScreen.X = screenWidth * 0.5f + x * fOverZ;

	if (OutScreen.X < 0.f || OutScreen.X > screenWidth)
		return false; // poza ekranem poziomo — exit przed liczeniem Up

	// Y (góra) = dot(Delta, Up)
	float y = Delta.X * (-sp * cy)
		+ Delta.Y * (-sp * sy)
		+ Delta.Z * cp;

	OutScreen.Y = screenHeight * 0.5f - y * fOverZ;

	if (OutScreen.Y < 0.f || OutScreen.Y > screenHeight)
		return false;

	return true;
}
#pragma endregion
inline bool IsA(uintptr_t actor, uintptr_t klass)
{
	uintptr_t temp = dbd->Read<uintptr_t>(actor + 0x10);
	while (temp)
	{
		if (temp == klass)
		{
			return true;
		}
		temp = dbd->Read<uintptr_t>(temp + 0x60);
	}
	
	return false;
}
inline bool IsWorldLoaded(uintptr_t GWorld)
{
	uintptr_t GameState = dbd->Read<uintptr_t>(GWorld + 0x01D8);
	if (!GameState)return false;
	auto time = dbd->Read<int>(GameState + 0x0360);
	return time > 10;
}

inline FMinimalViewInfo GetMyPov(uintptr_t playercontroller) {

	uintptr_t cameraManager = dbd->Read<uintptr_t>(playercontroller + 0x03B0);
	if (cameraManager) {
		FCameraCacheEntry* cache = dbd->ReadAsReference<FCameraCacheEntry>(cameraManager + 0x15A0);

		return cache->POV;
	}
	return {};
}

inline FVector GetActorLocation(uintptr_t pawn) {
	uintptr_t rootComponent = dbd->Read<uintptr_t>(pawn + 0x01E0);
	if(rootComponent)
	return dbd->Read<FVector>(rootComponent + 0x0178);
}
inline bool IsDeadOrInParadise(uintptr_t survivor) {
	uintptr_t healthComponent = dbd->Read<uintptr_t>(survivor + 0x18A0);
	if (!healthComponent) return 1;
	int loc = dbd->Read<int>(survivor + 0x1030);
	if(!loc)
	if (dbd->Read<unsigned char>(healthComponent + 644) != 0) {
		return true;
	}

	return loc;
}
inline uintptr_t GetPlayerState(uintptr_t pawn) {
	return dbd->Read<uintptr_t>(pawn + 0x0310);
}
inline std::wstring_view GetUsername(uintptr_t playerState)
{
	const FString& str =
		dbd->Read<FString>(playerState + 0x390);

	if (!str.Data)
		return {};

	if (str.Count <= 0 || str.Count > 50)
		return {};

	const uintptr_t address =
		reinterpret_cast<uintptr_t>(str.Data);

	const uintptr_t page_base =
		address & PAGE_MASK;

	const uintptr_t page_offset =
		address - page_base;

	uint64_t mapped_page =
		dbd->FindMapping(page_base);

	if (!mapped_page)
	{
		const uint64_t phys =
			dbd->drv.TranslateVirtualAddress(
				dbd->dtb,
				page_base
			);

		if (!phys)
			return {};

		mapped_page =
			dbd->drv.MapBuffer(
				phys,
				PAGE_SIZE,
				0
			);

		if (!mapped_page)
			return {};

		{
			std::lock_guard lock(dbd->cacheLock);
			dbd->mappings[page_base] = mapped_page;
		}
	}

	const wchar_t* text =
		reinterpret_cast<const wchar_t*>(
			mapped_page + page_offset
			);

	size_t len = str.Count;

	// FString zwykle zawiera trailing '\0'
	if (len > 0 && text[len - 1] == L'\0')
		--len;

	return std::wstring_view(text, len);

}
inline std::string Utf8FromView(const std::wstring_view wv)
{
	if (wv.empty() || wcsstr(wv.data(),L"None")!=0 )return {};

	int size = WideCharToMultiByte(
		CP_UTF8,
		0,
		wv.data(),
		(int)wv.size(),
		nullptr,
		0,
		nullptr,
		nullptr
	);

	std::string result(size, 0);

	WideCharToMultiByte(
		CP_UTF8,
		0,
		wv.data(),
		(int)wv.size(),
		result.data(),
		size,
		nullptr,
		nullptr
	);

	return result;
}
inline FVector GetBoneWorldPosition(uintptr_t Mesh, int BoneIndex)
{
	FTransform CTW = dbd->Read<FTransform>(Mesh + 0x220);
	if (CTW.Translation.X <= 0)return {};
	TArray<FTransform> BoneArray(dbd,Mesh + 0x678+0x30);
	if (BoneArray.Length <= 0)return FVector::ZERO();
	FTransform Bone = BoneArray.GetAtIndex(BoneIndex);
	

	FVector scaled = CTW.Scale3D * Bone.Translation;
	FVector rotated = CTW.Rotation.Ro(scaled);
	FVector worldPos = rotated + CTW.Translation;

	return worldPos;
}
inline void DrawBoneLine(uintptr_t mesh, FMinimalViewInfo& POV, int a, int b,ImU32 color) {
	FVector WorldPositionA = GetBoneWorldPosition(mesh, a);
	if (WorldPositionA == FVector::ZERO())return;
	FVector WorldPositionB = GetBoneWorldPosition(mesh, b);
	if (WorldPositionB == FVector::ZERO())return;
	FVector2D ScreenPositionA; 
	FVector2D ScreenPositionB; 
	if (WorldToScreenPoint(WorldPositionA, POV, ScreenPositionA) && WorldToScreenPoint(WorldPositionB, POV, ScreenPositionB))
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(ScreenPositionA.X, ScreenPositionA.Y), ImVec2(ScreenPositionB.X, ScreenPositionB.Y),color);
}

inline BoneCacheEntry* FindCache(uintptr_t actor)
{
	for (size_t i = 0; i < boneCacheSize; i++)
	{
		if (boneCache[i].actor == actor)
			return &boneCache[i];
	}
	return nullptr;
}
inline BoneCacheEntry* AddCache(uintptr_t actor)
{
	if (boneCacheSize >= boneCache.size())
		return nullptr; // albo nadpisuj najstarszy

	BoneCacheEntry& entry = boneCache[boneCacheSize++];

	entry.actor = actor;
	std::fill_n(entry.bones, 16, -1);

	return &entry;
}
inline void DrawSkeleton(uintptr_t actor, FMinimalViewInfo& POV,ImU32 color)
{

	uintptr_t Mesh = dbd->Read<uintptr_t>(actor + 0x0370);
	BoneCacheEntry* cache = FindCache(actor);
	if (!cache)
	{
		cache = AddCache(actor);
		constexpr const char* boneNames[16] = {
			"joint_Head_01", "joint_NeckA_01", "joint_TorsoC_01", "joint_Pelvis_01",
			"joint_ShoulderLT_01", "joint_ElbowLT_01", "joint_HandLT_01",
			"joint_ShoulderRT_01", "joint_ElbowRT_01", "joint_HandRT_01",
			"joint_HipLT_01", "joint_KneeLT_01", "joint_FootLT_01",
			"joint_HipRT_01", "joint_KneeRT_01", "joint_FootRT_01"
		};


		uintptr_t SkeletalMesh = dbd->Read<uintptr_t>(Mesh + 0x0628);

		auto RawRefBoneArray = TArray<FMeshBoneInfo>(dbd, SkeletalMesh + 0x0358+8);
		if (RawRefBoneArray.Length > 0)
		{
			FNamesPool namesPool;
			for (int i = 0; i < RawRefBoneArray.Length; i++)
			{
				FMeshBoneInfo boneInfo = RawRefBoneArray.GetAtIndex(i);
				std::string name = namesPool.GetAtIndex(dbd, boneInfo.Name.DisplayIndex);
				if (!name.empty())
				{
					for (int j = 0; j < 16; j++)
					{
						if (cache->bones[j] == -1 && name == boneNames[j])
						{
							cache->bones[j] = i;
							break; // found, move to next bone
						}
					}
				}
			}

		}
	}
	int* boneIndices = cache->bones;
	int head = boneIndices[0];
	int neck = boneIndices[1];
	int chest = boneIndices[2];
	int pelvis = boneIndices[3];

	int lShoulder = boneIndices[4];
	int lElbow = boneIndices[5];
	int lHand = boneIndices[6];

	int rShoulder = boneIndices[7];
	int rElbow = boneIndices[8];
	int rHand = boneIndices[9];

	int lHip = boneIndices[10];
	int lKnee = boneIndices[11];
	int lFoot = boneIndices[12];

	int rHip = boneIndices[13];
	int rKnee = boneIndices[14];
	int rFoot = boneIndices[15];
	//head
	DrawBoneLine(Mesh, POV, head, neck, color);
	DrawBoneLine(Mesh, POV, neck, chest, color);
	DrawBoneLine(Mesh, POV, chest, pelvis, color);

	// left arm
	DrawBoneLine(Mesh, POV, chest, lShoulder, color);
	DrawBoneLine(Mesh, POV, lShoulder, lElbow, color);
	DrawBoneLine(Mesh, POV, lElbow, lHand, color);

	// right arm
	DrawBoneLine(Mesh, POV, chest, rShoulder, color);
	DrawBoneLine(Mesh, POV, rShoulder, rElbow, color);
	DrawBoneLine(Mesh, POV, rElbow, rHand, color);

	// left leg
	DrawBoneLine(Mesh, POV, pelvis, lHip, color);
	DrawBoneLine(Mesh, POV, lHip, lKnee, color);
	DrawBoneLine(Mesh, POV, lKnee, lFoot, color);

	// right leg
	DrawBoneLine(Mesh, POV, pelvis, rHip, color);
	DrawBoneLine(Mesh, POV, rHip, rKnee, color);
	DrawBoneLine(Mesh, POV, rKnee, rFoot, color);
	
}

inline void sendSpaceCommand()
{
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_SPACE;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_SPACE;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

//inline bool IsKillerCarryingMe()
//{
//	for (int i = 0; i < killers.size(); i++)
//	{
//		uintptr_t carried = dbd->Read<uintptr_t>(killers.at(i) + 0x1A68);
//		return LocalPawn == carried;
//	}
//	return false;
//}
inline void DistanceText(std::string& str, FVector& a, FVector& b) {
	str += " [";
	str += std::to_string((int)FVector::calculateDistance(a, b) / 100);
	str += " m]";
}
inline void DrawESPText(const char* tmp, ImVec2 pos, ImU32 color) {

	ImU32 outline = IM_COL32(0, 0, 0, 255); // czarny


	// ile pikseli grubo�ci obrysu
	float thickness = 2.0f;

	// obrys (w 8 kierunkach)
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x - thickness, pos.y), outline, tmp);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x + thickness, pos.y), outline, tmp);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x, pos.y - thickness), outline, tmp);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x, pos.y + thickness), outline, tmp);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x - thickness, pos.y - thickness), outline, tmp);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x + thickness, pos.y - thickness), outline, tmp);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x - thickness, pos.y + thickness), outline, tmp);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(pos.x + thickness, pos.y + thickness), outline, tmp);

	// w�a�ciwy tekst
	ImGui::GetBackgroundDrawList()->AddText(pos, color, tmp);
}