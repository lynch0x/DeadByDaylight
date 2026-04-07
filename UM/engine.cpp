#include "engine.h"


uintptr_t FNamesPool::Pool = 0;
uintptr_t GObjectsPool::Pool = 0;

std::string FNamesPool::GetAtIndex(DeadByDaylight* dbd, int actor_id)
{

    if (!Pool) {
        if (dbd->base == 0) return {};
        Pool = dbd->Read<uintptr_t>(dbd->base + Offsets::GNames);
    }

    char buffer[128];

    int TableLocation = (unsigned int)(actor_id >> 0x10);
    uint16_t RowLocation = (uint16_t)actor_id;

    uint64_t GNameTable = (uintptr_t)dbd->base + Offsets::GNames;

    uint64_t TableLocationAddress =
        dbd->Read<uint64_t>(GNameTable + 0x10 + TableLocation * 0x8)
        + (uint32_t)(4 * RowLocation);

    uint64_t sLength = (uint64_t)(dbd->Read<uint16_t>(TableLocationAddress + 4)) >> 1;

    if (sLength >= 128)
        return {};

    dbd->ReadRaw((TableLocationAddress + 6), buffer, sLength);
    buffer[sLength] = '\0';

    return std::string(buffer);
}

uintptr_t GObjectsPool::GetByName(DeadByDaylight* dbd, const char* name)
{
    if (!Pool) {
        if (dbd->base == 0) return 0;
        Pool = dbd->Read<uintptr_t>(dbd->base + Offsets::GObjects);
    }


    uintptr_t objArray = dbd->Read<uintptr_t>(Pool);

    FNamesPool namesPool;
    for (int i = 0; i < 86895; i++)
    {
        uintptr_t objectItem = objArray + (i * 0x18);
        uintptr_t objectPtr = dbd->Read<uintptr_t>(objectItem);
        if (!objectPtr) continue;

        int id = dbd->Read<int>(objectPtr + 0x18);
        std::string rname = namesPool.GetAtIndex(dbd, id);

        if (strcmp(name, rname.c_str()) == 0) {
            return objectPtr;
        }
    }
    return 0;
}
void GObjectsPool::GetManyByNames(DeadByDaylight* dbd, SearchKey* keys, unsigned __int16 count)
{
    if (!Pool) {
        if (dbd->base == 0) return;
        Pool = dbd->Read<uintptr_t>(dbd->base + Offsets::GObjects);
    }


    uintptr_t objArray = dbd->Read<uintptr_t>(Pool);

    FNamesPool namesPool;
    for (int i = 4000; i < 86895; i++)
    {
        uintptr_t objectItem = objArray + (i * 0x18);
        uintptr_t objectPtr = dbd->Read<uintptr_t>(objectItem);
        if (!objectPtr) continue;

        int id = dbd->Read<int>(objectPtr + 0x18);
        std::string rname = namesPool.GetAtIndex(dbd, id);
        for (unsigned __int16 j = 0; j < count; j++)
        {
            auto key = keys[j];
            if (strcmp(key.Key, rname.c_str()) == 0) {
                *key.Return = objectPtr;
            }

        }
       
    }
    
}