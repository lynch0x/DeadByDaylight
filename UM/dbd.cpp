#include "dbd.h"
static DWORD FindDBDProcess() {
    DWORD pid = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe;
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
        process_id = FindDBDProcess();
        Sleep(2000);
    }
    HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id);
    if (!handle) return false;

    DWORD exitCode;
    bool exists = GetExitCodeProcess(handle, &exitCode) && exitCode == STILL_ACTIVE;
    CloseHandle(handle);
    return exists;

}
	HANDLE DeadByDaylight::GetProcessBase()
	{
        while (process_id == 0) {
            process_id = FindDBDProcess();
            Sleep(1200);
        }
        if (!base) {
            
            if (!DeviceIoControl(handle, base_code,(HANDLE*) & process_id, sizeof(HANDLE), &base, sizeof(HANDLE), 0, 0))
            {
                __debugbreak();
            }
        }
        return (HANDLE)base;
	}
    DeadByDaylight::DeadByDaylight() {
        handle = CreateFileA("\\\\.\\daisy", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	}
    DeadByDaylight::~DeadByDaylight() {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }

    }
    void DeadByDaylight::ReadRaw(uintptr_t address, void* buffer, int length)
    {
        req_t request = { 0 };
        request.target_pid = process_id;
        request.length = length;
        request.src = (PVOID)address;
        request.dst = buffer;
        DeviceIoControl(handle, read_code, &request, sizeof(req_t), 0,0, 0, 0);
       
    }