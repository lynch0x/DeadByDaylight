#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
constexpr ULONG read_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x776, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG write_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x777, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG base_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x778, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
struct req_t {
	int target_pid;
	int length;
	void* src;
	void* dst;
};
class DeadByDaylight {
public:
	HANDLE handle=0;
	unsigned __int64 base = 0;
	unsigned __int64 process_id=0;
	HANDLE GetProcessBase();
	bool CheckGameAlive();
	void ReadRaw(uintptr_t address, void* buffer, int length);
	DeadByDaylight();
	~DeadByDaylight();
	template<class T>
	T Read(uintptr_t address) noexcept {
		T value = { 0 };
		req_t request = { 0 };
		request.target_pid = process_id;
		request.length = sizeof(T);
		request.src = (PVOID)address;
		request.dst = &value;
		DeviceIoControl(handle, read_code, &request, sizeof(req_t), 0, 0, 0, 0);
		
		return value;
	}
	template<class T>
	T Write(uintptr_t address, T value) noexcept {
		req_t request = { 0 };
		request.target_pid = process_id;
		request.length = sizeof(T);
		request.src =&value;
		request.dst = (PVOID)address;
		DeviceIoControl(handle, write_code, &request, sizeof(req_t), 0, 0, 0, 0);
		
		return value;
	}
};