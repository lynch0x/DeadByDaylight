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
        HANDLE base = 0;
        req_t request = { 0 };
        request.target_pid = process_id;
        request.dst = &base;
        DeviceIoControl(handle, base_code, &request, sizeof(req_t), &request, sizeof(req_t), 0, 0);
        return base;
	}
    DeadByDaylight::DeadByDaylight() {
        handle = CreateFileA("\\\\.\\daisy", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	}
    DeadByDaylight::~DeadByDaylight() {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }

    }