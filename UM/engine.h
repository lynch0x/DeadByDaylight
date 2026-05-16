#pragma once
#include "dbd.h"
#include "offsets.h"
#include <vector>
#include <cmath>

struct FVector
{
	double X = 0.0;
	double Y = 0.0;
	double Z = 0.0;
	static constexpr FVector ZERO()
	{
		return FVector();
	}
	FVector operator*(const FVector& other) const {
		return { X * other.X, Y * other.Y, Z * other.Z };
	}
	bool operator==(const FVector& other)const
	{
		return X == other.X && Y == other.Y && Z == other.Z;
	}

	FVector operator+(const FVector& other) const {
		return { X + other.X, Y + other.Y, Z + other.Z };
	}

	FVector operator-(const FVector& other) const {
		return { X - other.X, Y - other.Y, Z - other.Z };
	}

	double length() const {
		return std::sqrt(X * X + Y * Y + Z * Z);
	}

	static double calculateDistance(const FVector& v1, const FVector& v2) {
		return (v2 - v1).length();
	}

};

struct alignas(0x10) FQuat final
{
public:
	double                                        X;                                                 // 0x0000(0x0008)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	double                                        Y;                                                 // 0x0008(0x0008)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	double                                        Z;                                                 // 0x0010(0x0008)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	double                                        W;
	FVector Ro(const FVector& V) const
	{
		// Wz�r: q * v * q^-1 (uproszczony do formy bez pe�nego mno�enia quat�w)
		FVector Q;
		Q.X = X;
		Q.Y = Y;
		Q.Z = Z;

		// t = 2 * cross(q.xyz, v)
		FVector t;
		t.X = 2.0 * (Q.Y * V.Z - Q.Z * V.Y);
		t.Y = 2.0 * (Q.Z * V.X - Q.X * V.Z);
		t.Z = 2.0 * (Q.X * V.Y - Q.Y * V.X);

		// result = v + W * t + cross(q.xyz, t)
		FVector result;
		result.X = V.X + W * t.X + (Q.Y * t.Z - Q.Z * t.Y);
		result.Y = V.Y + W * t.Y + (Q.Z * t.X - Q.X * t.Z);
		result.Z = V.Z + W * t.Z + (Q.X * t.Y - Q.Y * t.X);
		return result;
	}// 0x0018(0x0008)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};
struct FTransform final
{
public:
	struct FQuat                                  Rotation;                                          // 0x0000(0x0020)(Edit, BlueprintVisible, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FVector                                Translation;                                       // 0x0020(0x0018)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                         Pad_38[0x8];                                       // 0x0038(0x0008)(Fixing Size After Last Property [ Dumper-7 ])
	struct FVector                                Scale3D;                                           // 0x0040(0x0018)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                         Pad_58[0x8];                                       // 0x0058(0x0008)(Fixing Struct Size After Last Property [ Dumper-7 ])
};
struct FString
{
	wchar_t* Data;  // wska�nik do tablicy znak�w UTF-16
	int32_t   Count; // liczba znak�w
	int32_t   MaxNum;
};
struct FVector2D final
{
public:
	double                                       X;                                                 // 0x0000(0x0008)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	double                                        Y;                                                 // 0x0008(0x0008)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

struct FRotator final
{
public:
	double                                        Pitch;                                             // 0x0000(0x0008)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	double                                        Yaw;                                               // 0x0008(0x0008)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	double                                        Roll;                                              // 0x0010(0x0008)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};
struct FMinimalViewInfo final
{
public:
	struct FVector                                Location;                                          // 0x0000(0x0018)(Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FRotator                               Rotation;                                          // 0x0018(0x0018)(Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
	float                                         FOV;                                               // 0x0030(0x0004)(Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};
struct FCameraCacheEntry final
{
public:
	float                                         Timestamp;                                         // 0x0000(0x0004)(ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                         Pad_4[0xC];                                        // 0x0004(0x000C)(Fixing Size After Last Property [ Dumper-7 ])
	struct FMinimalViewInfo                       POV;                                               // 0x0010(0x0800)(NativeAccessSpecifierPublic)
};

template<class T>
class TArray {
public:
	void* Data;
	int Length;
	int Max;
	DeadByDaylight* dbd;
    TArray() : Data(nullptr), Length(0) {}

    TArray(DeadByDaylight* _dbd, unsigned __int64 ptr) {
		dbd = _dbd;
		dbd->ReadRaw(ptr, &Data, 16);
		// sanity: przy złym ptr potrafi wyjść kosmiczna długość i wywalić proces na malloc/ReadRaw
		if (Length <= 0 || Length > Max || !Data ) {
		
			Data = 0;
			Length = 0;
			return;
		}

	
		uintptr_t dataPtr = (uintptr_t)Data;
		int sizeBytes = Length * sizeof(T);
		Data = malloc(sizeBytes);
		if (!Data) {
			Length = 0;
			return;
		}

		dbd->ReadRaw(dataPtr, Data, sizeBytes);
    }

    ~TArray() {
        if (Data) {
          // free(Data);
			
				free(Data);
			
        }
    }

    T GetAtIndex(int index) const {

		if (index < 0 || index >= Length || index > Max)return {};
        return *(T*)((uintptr_t)Data + (index * sizeof(T)));
    }
};
template<class T>
class TArrayNew {
public:
	void* Data = nullptr;
	int          Length = 0;
	int          Max = 0;
	uint64_t     page = 0;          // ← inicjalizacja w miejscu deklaracji
	DeadByDaylight* dbd = nullptr;

	// Domyślny konstruktor – trywialne zerowanie
	TArrayNew() = default;

	TArrayNew(DeadByDaylight* _dbd, unsigned __int64 ptr) : dbd(_dbd)
	{
		struct { void* data; int length; int max; } raw{};
		dbd->ReadRaw(ptr, &raw, sizeof(raw));
		Data = raw.data;
		Length = raw.length;
		Max = raw.max;

		if (Length <= 0 || Length > Max || !Data)
		{
			Data = nullptr; Length = 0; return;
		}

		const uintptr_t dataPtr = reinterpret_cast<uintptr_t>(Data);
		const int       sizeBytes = Length * static_cast<int>(sizeof(T));
		const uintptr_t page_base = dataPtr & PAGE_MASK;
		const uintptr_t page_offset = dataPtr - page_base;
		page = page_base;

		const bool crossPage =
			(page_offset + static_cast<size_t>(sizeBytes)) > PAGE_SIZE;

		if (crossPage)
		{
			// ── Klucz cache = dokładny adres wirtualny bufora ─────────────
			// (nie page_base, bo ten sam page może hostować różne bufory)
			uint64_t mapped = dbd->FindMapping(dataPtr);
			if (!mapped)
			{
				const uint64_t phys =
					dbd->drv.TranslateVirtualAddress(dbd->dtb, dataPtr);
				if (!phys) { Data = nullptr; Length = 0; return; }

				mapped = dbd->drv.MapBuffer(phys, sizeBytes, 0);
				if (!mapped) { Data = nullptr; Length = 0; return; }

				{
					std::lock_guard lock(dbd->cacheLock);
					dbd->mappings[dataPtr] = mapped;   // ← klucz = dataPtr
				}
			}

			Data = reinterpret_cast<void*>(mapped);
			return;
		}

		// ── Single-page: klucz cache = page_base (bez zmian) ─────────────
		/*uint64_t mapped_page = dbd->FindMapping(page_base);
		if (!mapped_page)
		{
			const uint64_t phys =
				dbd->drv.TranslateVirtualAddress(dbd->dtb, page_base);
			if (!phys) { Data = nullptr; Length = 0; return; }

			mapped_page = dbd->drv.MapBuffer(phys, PAGE_SIZE, 0);
			if (!mapped_page) { Data = nullptr; Length = 0; return; }

			{
				std::lock_guard lock(dbd->cacheLock);
				dbd->mappings[page_base] = mapped_page;
			}
		}*/

	//	Data = reinterpret_cast<void*>(mapped_page + page_offset);
		//if (!Data) { Length = 0; }
	}
	~TArrayNew() = default;   // nic nie zwalniamy – lifetime należy do dbd

	T GetAtIndex(int index) const
	{
		if (!Data || index < 0 || index >= Length || index >= Max)
			return {};
		return *reinterpret_cast<const T*>(
			reinterpret_cast<uintptr_t>(Data) + index * sizeof(T));
	}
};

class FName final
{
public:
	int                                       ComparisonIndex;                                   // 0x0000(0x0004)(NOT AUTO-GENERATED PROPERTY)
	int                                         Number;                                            // 0x0004(0x0004)(NOT AUTO-GENERATED PROPERTY)
	int                                         DisplayIndex;                                      // 0x0008(0x0004)(NOT AUTO-GENERATED PROPERTY)
};
struct FMeshBoneInfo {
	FName Name;
	int ParentIndex;
};
class alignas(0x08) UObject
{
public:                                     

	void* VTable;                                            // 0x0000(0x0008)(NOT AUTO-GENERATED PROPERTY)
	int                                  Flags;                                             // 0x0008(0x0004)(NOT AUTO-GENERATED PROPERTY)
	int                                         Index;                                             // 0x000C(0x0004)(NOT AUTO-GENERATED PROPERTY)
	class UClass* Class;                                             // 0x0010(0x0008)(NOT AUTO-GENERATED PROPERTY)
	class FName                                   Name;                                              // 0x0018(0x000C)(NOT AUTO-GENERATED PROPERTY)
	unsigned char                                         Pad_24[0x4];                                       // 0x0024(0x0004)(Fixing Size After Last Property [ Dumper-7 ])
	class UObject* Outer;                                             // 0x0028(0x0008)(NOT AUTO-GENERATED PROPERTY)
	unsigned char                                           Pad_30[0x18];
};
class FNamesPool {
private:
    static uintptr_t Pool;
public:

	std::string GetAtIndex(DeadByDaylight* dbd,int index);
};
struct SearchKey {
    const char* Key;
    uintptr_t* Return;
};
class GObjectsPool {
private:
    static uintptr_t Pool;
public:

    uintptr_t GetByName(DeadByDaylight* dbd, const char*);
    void GetManyByNames(DeadByDaylight* dbd, SearchKey* keys, unsigned __int16 count);
    uintptr_t GetByIndex(DeadByDaylight* dbd, unsigned int index);
};