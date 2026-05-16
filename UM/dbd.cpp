#include "dbd.h"
static DWORD FindDBDProcess() {
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe)) {
        do {
            if (wcsstr(pe.szExeFile, L"DeadByDaylight") != nullptr &&
                wcsstr(pe.szExeFile, L"Shipping") != nullptr) {
                pid = pe.th32ProcessID;
                break;
            }

        } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return pid;
}
bool DeadByDaylight::CheckGameAlive() {
    while (!process_id) {
       
        Sleep(2000);
    }
    

    DWORD exitCode;
    bool exists = GetExitCodeProcess(process_handle, &exitCode) && exitCode == STILL_ACTIVE;
    return exists;

}
uint64_t DeadByDaylight::FindMapping(uintptr_t page_base)
{
    std::lock_guard lock(cacheLock);

    auto it = mappings.find(page_base);

    if (it != mappings.end())
        return it->second == -1 ?0:it->second;

    return 0;
}
void DeadByDaylight::ClearMappings()
{
    std::lock_guard lock(cacheLock);

    for (auto& [_, mapped] : mappings)
    {
        drv.UnmapBuffer(mapped);
    }

    mappings.clear();
}

	HANDLE DeadByDaylight::GetProcessBase()
	{
        while (process_id == 0) {
            process_id = FindDBDProcess();
            Sleep(1200);
        }
        if (!base) {
            if (!process_handle) process_handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, process_id);
            if (!dtb)  dtb = drv.FindProcessDTB(process_id);
            PROCESS_BASIC_INFORMATION pbi = { 0 };
            auto NtQueryInformationProcess = reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, ULONG, PVOID, ULONG, PULONG)>(
                GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess"));
            NtQueryInformationProcess(process_handle, 0, &pbi, sizeof(pbi), nullptr);
            if (!pbi.PebBaseAddress)return 0;
            drv.ReadProcessMemory(dtb, (uintptr_t)pbi.PebBaseAddress + 0x10, &base, sizeof(base));
        }

      
        return (HANDLE)base;
	}
    DeadByDaylight::DeadByDaylight() {
        mappings.reserve(4000);
        drv.Initialize();
       // handle = CreateFileA("\\\\.\\daisy", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	}
    DeadByDaylight::~DeadByDaylight() {
        drv.Close();
        CloseHandle(process_handle);
    }
    void DeadByDaylight::ReadRaw(uintptr_t address, void* buffer, int length)
    {
        uint8_t* out = reinterpret_cast<uint8_t*>(buffer);

        while (length > 0)
        {
            const uintptr_t page_base = address & PAGE_MASK;
            const size_t page_offset = address - page_base;

            const size_t chunk =
                min(PAGE_SIZE - page_offset, length);

            uint64_t mapped_page = FindMapping(page_base);

            if (!mapped_page)
            {
                const uint64_t phys =
                    drv.TranslateVirtualAddress(dtb, page_base);

                if (!phys)
                    return;

                mapped_page =
                    drv.MapBuffer(phys, PAGE_SIZE, 0);

                if (!mapped_page)
                    return;

                {
                    std::lock_guard lock(cacheLock);
                    mappings[page_base] = mapped_page;
                }
            }

            memcpy(
                out,
                reinterpret_cast<void*>(mapped_page + page_offset),
                chunk
            );

            out += chunk;
            address += chunk;
            length -= chunk;
        }

    }
