#pragma once
#include <windows.h>
#include <tlhelp32.h>
constexpr ULONG read_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x776, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG write_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x777, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG base_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x778, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
struct req_t {
	int target_pid;
	int length;
	uintptr_t src;
	void* dst;
};
class DeadByDaylight {
public:
	HANDLE handle=0;
	DWORD process_id=0;
	HANDLE GetProcessBase();
	DeadByDaylight();
	~DeadByDaylight();
	template<class T>
	T Read(uintptr_t address) noexcept {
		T value = { 0 };
		req_t request = { 0 };
		request.target_pid = process_id;
		request.length = sizeof(T);
		request.src = address;
		request.dst = &value;
		DeviceIoControl(handle, read_code, &request, sizeof(req_t), &request, sizeof(req_t), 0, 0);
		return value;
	}
};