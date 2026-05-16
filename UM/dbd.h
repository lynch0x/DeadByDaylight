#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include "cordrv.hpp"
#include <mutex>
#include <unordered_map>
constexpr ULONG read_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x776, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG write_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x777, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG base_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x778, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
struct req_t {
	int target_pid;
	int length;
	void* src;
	void* dst;
};
typedef struct _PROCESS_BASIC_INFORMATION {
	PVOID Reserved1;
	PVOID PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;
constexpr uintptr_t PAGE_SIZE = 0x1000;
constexpr uintptr_t PAGE_MASK = ~(PAGE_SIZE - 1);
class DeadByDaylight {


public:
	CorDrv drv;
	unsigned __int64 dtb=0;
	unsigned __int64 base = 0;
	unsigned __int64 process_id=0;
	HANDLE process_handle = 0;
    std::mutex cacheLock;
    std::unordered_map<uint64_t, uint64_t> mappings;
	HANDLE GetProcessBase();
	bool CheckGameAlive();
	void ReadRaw(uintptr_t address, void* buffer, int length);
    uint64_t FindMapping(uintptr_t virtual_address);
	DeadByDaylight();
	~DeadByDaylight();
	void ClearMappings();
    template<class T>
    T* ReadAsReference(uintptr_t address) noexcept
    {
        const uintptr_t page_base = address & PAGE_MASK;
        const uintptr_t page_offset = address - page_base;

        // Cross-page
        if (page_offset + sizeof(T) > PAGE_SIZE)
        {
            const uint64_t phys =
                drv.TranslateVirtualAddress(dtb, address);

            if (!phys)
                return 0;

            // mapujemy dok³adnie sizeof(T)
            const uint64_t mapped =
                drv.MapBuffer(phys, sizeof(T), 0);

            if (!mapped)
                return 0;


            return reinterpret_cast<T*>(mapped);
        }

        uint64_t mapped_page = FindMapping(page_base);

        if (!mapped_page)
        {
            const uint64_t phys =
                drv.TranslateVirtualAddress(dtb, page_base);

            if (!phys)
                return 0;

            mapped_page =
                drv.MapBuffer(phys, PAGE_SIZE, 0);

            if (!mapped_page)
                return 0;

            {
                std::lock_guard lock(cacheLock);
                mappings[page_base] = mapped_page;
            }
        }

        return reinterpret_cast<T*>(
            mapped_page + page_offset
            );
    }
    template<class T>
    T Read(uintptr_t address) noexcept
    {
        T* ref = ReadAsReference<T>(address);
        if (!ref)
            return {};
        return *ref;
    }
   
//	template<class T>
//	T Write(uintptr_t address, T value) noexcept {
//		req_t request = { 0 };
//		request.target_pid = process_id;
//		request.length = sizeof(T);
//		request.src =&value;
//		request.dst = (PVOID)address;
//		if (!DeviceIoControl(handle, write_code, &request, sizeof(req_t), 0, 0, 0, 0))
//		{
//#ifdef _DEBUG
//			int err = GetLastError();
//			__debugbreak();
//#endif 
//		}
//		
//		return value;
//	}
//	void WriteBytes(uintptr_t address,void* value,int len) noexcept {
//		req_t request = { 0 };
//		request.target_pid = process_id;
//		request.length = len;
//		request.src = value;
//		request.dst = (PVOID)address;
//		/*if (!DeviceIoControl(handle, write_code, &request, sizeof(req_t), 0, 0, 0, 0))
//		{
//#ifdef _DEBUG
//			int err = GetLastError();
//			__debugbreak();
//#endif 
//		}*/
//
//	}
};