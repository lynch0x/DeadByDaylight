#pragma once
#include <string>
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
	X = RotationToVector(R);
	Normalize(X);
	R.Yaw += 89.8f;
	FRotator R2 = R;
	R2.Pitch = 0.0f;
	Y = RotationToVector(R2);
	Normalize(Y);
	Y.Z = 0.0f;
	R.Yaw -= 89.8f;
	R.Pitch += 89.8f;
	Z = RotationToVector(R);
	Normalize(Z);
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


static FVector2D WorldToScreenPoint(FVector Location, FMinimalViewInfo cam_cache)
{
	FVector2D Return;

	FVector AxisX, AxisY, AxisZ, Delta, Transformed;
	GetAxes(cam_cache.Rotation, AxisX, AxisY, AxisZ);

	Delta = VectorSubtract(Location, cam_cache.Location);
	Transformed.X = Dot(Delta, AxisY);
	Transformed.Y = Dot(Delta, AxisZ);
	Transformed.Z = Dot(Delta, AxisX);

	if (Transformed.Z < 1.00f)
		Transformed.Z = 1.00f;
	Return.X = (screenWidth / 2) + Transformed.X * ((screenWidth / 2) / tan(cam_cache.FOV * UCONST_Pi / 360.0f)) / Transformed.Z;
	Return.Y = (screenHeight / 2) + -Transformed.Y * ((screenWidth / 2) / tan(cam_cache.FOV * UCONST_Pi / 360.0f)) / Transformed.Z;

	return Return;
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
extern uintptr_t PlayerController;
inline FMinimalViewInfo GetMyPov() {

	uintptr_t cameraManager = dbd->Read<uintptr_t>(PlayerController + 0x0398);

	FCameraCacheEntry cache = dbd->Read<FCameraCacheEntry>(cameraManager + 0x1400);

	return cache.POV;
}
inline FVector GetActorLocation(uintptr_t pawn) {
	uintptr_t rootComponent = dbd->Read<uintptr_t>(pawn + 0x01C8);
	return dbd->Read<FVector>(rootComponent + 0x0160);
}
inline bool IsDeadOrInParadise(uintptr_t survivor) {
	uintptr_t healthComponent = dbd->Read<uintptr_t>(survivor + 0x18A0);
	if (!healthComponent) return 1;
	//int loc = DBD::read<int>(survivor + 0x1030);
	if (dbd->Read<unsigned char>(healthComponent + 644) != 0) {
		return true;
	}
	return false;
	//return loc;
}
inline uintptr_t GetPlayerState(uintptr_t pawn) {
	return dbd->Read<uintptr_t>(pawn + 0x02F8);
}
inline std::string GetUsername(uintptr_t playerState)
{
	auto str = dbd->Read<FString>(playerState + 0x0378);
	if (str.Count <= 0)
		return {};

	std::wstring ws(str.Count, L'\0');
	dbd->ReadRaw((uintptr_t)str.Data, ws.data(), str.Count * sizeof(wchar_t));

	return std::string(ws.begin(), ws.end());
}
inline FVector GetBoneWorldPosition(uintptr_t Mesh, int BoneIndex)
{
	TArray<FTransform> BoneArray(dbd,Mesh + 0x680);
	if (BoneArray.Length <= 0)return FVector::ZERO();
	FTransform Bone = BoneArray.GetAtIndex(BoneIndex);
	FTransform CTW = dbd->Read<FTransform>(Mesh + 0x210);

	FVector scaled = CTW.Scale3D * Bone.Translation;
	FVector rotated = CTW.Rotation.Ro(scaled);
	FVector worldPos = rotated + CTW.Translation;

	return worldPos;
}
inline void DrawBoneLine(uintptr_t mesh, FMinimalViewInfo& POV, int a, int b) {
	FVector WorldPositionA = GetBoneWorldPosition(mesh, a);
	if (WorldPositionA == FVector::ZERO())return;
	FVector WorldPositionB = GetBoneWorldPosition(mesh, b);
	if (WorldPositionB == FVector::ZERO())return;
	FVector2D ScreenPositionA = WorldToScreenPoint(WorldPositionA, POV);
	FVector2D ScreenPositionB = WorldToScreenPoint(WorldPositionB, POV);
	if (ScreenPositionA.X > 0 && ScreenPositionB.X > 0)
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(ScreenPositionA.X, ScreenPositionA.Y), ImVec2(ScreenPositionB.X, ScreenPositionB.Y), IM_COL32(255, 0, 0, 255));
}
static void DrawSkeleton(uintptr_t actor, FMinimalViewInfo& POV)
{
	constexpr const char* boneNames[16] = {
			"joint_Head_01", "joint_NeckA_01", "joint_TorsoC_01", "joint_Pelvis_01",
			"joint_ShoulderLT_01", "joint_ElbowLT_01", "joint_HandLT_01",
			"joint_ShoulderRT_01", "joint_ElbowRT_01", "joint_HandRT_01",
			"joint_HipLT_01", "joint_KneeLT_01", "joint_FootLT_01",
			"joint_HipRT_01", "joint_KneeRT_01", "joint_FootRT_01"
	};
	int boneIndices[16];
	std::fill_n(boneIndices, 16, -1);

	uintptr_t Mesh = dbd->Read<uintptr_t>(actor + 0x0360);

	uintptr_t SkeletalMesh = dbd->Read<uintptr_t>(Mesh + 0x608);

	auto RawRefBoneArray = TArray<FMeshBoneInfo>(dbd,SkeletalMesh + 0x2E8);
	if (RawRefBoneArray.Length > 0)
	{
		FNamesPool namesPool;
		for (int i = 0; i < RawRefBoneArray.Length; i++)
		{
			FMeshBoneInfo boneInfo = RawRefBoneArray.GetAtIndex(i);
			std::string name = namesPool.GetAtIndex(dbd,boneInfo.Name.DisplayIndex);
			if (!name.empty())
			{
				for (int j = 0; j < 16; j++)
				{
					if (boneIndices[j] == -1 && name == boneNames[j])
					{
						boneIndices[j] = i;
						break; // found, move to next bone
					}
				}
			}
		}
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
		DrawBoneLine(Mesh, POV, head, neck);
		DrawBoneLine(Mesh, POV, neck, chest);
		DrawBoneLine(Mesh, POV, chest, pelvis);

		// left arm
		DrawBoneLine(Mesh, POV, chest, lShoulder);
		DrawBoneLine(Mesh, POV, lShoulder, lElbow);
		DrawBoneLine(Mesh, POV, lElbow, lHand);

		// right arm
		DrawBoneLine(Mesh, POV, chest, rShoulder);
		DrawBoneLine(Mesh, POV, rShoulder, rElbow);
		DrawBoneLine(Mesh, POV, rElbow, rHand);

		// left leg
		DrawBoneLine(Mesh, POV, pelvis, lHip);
		DrawBoneLine(Mesh, POV, lHip, lKnee);
		DrawBoneLine(Mesh, POV, lKnee, lFoot);

		// right leg
		DrawBoneLine(Mesh, POV, pelvis, rHip);
		DrawBoneLine(Mesh, POV, rHip, rKnee);
		DrawBoneLine(Mesh, POV, rKnee, rFoot);
	}
}
inline void DistanceText(std::string& str, FVector& a, FVector& b) {
	str += " [";
	str += std::to_string((int)FVector::calculateDistance(a, b) / 100);
	str += " m]";
}
inline void DrawESPText(const char* tmp, ImVec2 pos, ImU32 color) {

	ImU32 outline = IM_COL32(0, 0, 0, 255); // czarny


	// ile pikseli gruboœci obrysu
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

	// w³aœciwy tekst
	ImGui::GetBackgroundDrawList()->AddText(pos, color, tmp);
}