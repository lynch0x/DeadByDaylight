#include <ntifs.h>
#include <ntimage.h>
#include "buf.h"
constexpr ULONG read_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x776, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); 
constexpr ULONG write_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x777, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); 
constexpr ULONG base_code = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x778, METHOD_BUFFERED, FILE_SPECIAL_ACCESS); 
struct req_t {
    int target_pid;
    int length;
    void* src;
    void* dst;
};



extern "C" 	NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(PEPROCESS proc);

// Some WDK setups don't declare ZwProtectVirtualMemory in the headers used here.
// Declare it in a way that matches WDK import semantics to avoid "inconsistent dll linkage".
extern "C" NTSYSAPI NTSTATUS NTAPI ZwProtectVirtualMemory(
    _In_ HANDLE ProcessHandle,
    _Inout_ PVOID* BaseAddress,
    _Inout_ PSIZE_T RegionSize,
    _In_ ULONG NewProtect,
    _Out_ PULONG OldProtect
);

static __forceinline bool is_user_range(const void* ptr, SIZE_T len)
{
    if (!ptr || len == 0) return false;
    const ULONG_PTR start = (ULONG_PTR)ptr;
    const ULONG_PTR end = start + len - 1;
    if (end < start) return false; // overflow
    return start <= (ULONG_PTR)MmHighestUserAddress && end <= (ULONG_PTR)MmHighestUserAddress;
}


NTSTATUS ctl_io(PDEVICE_OBJECT device_obj, PIRP irp) {
    UNREFERENCED_PARAMETER(device_obj);

    auto* irpStack = IoGetCurrentIrpStackLocation(irp);
    NTSTATUS status = STATUS_SUCCESS;
  
    if (!irpStack) {
        status = STATUS_INVALID_PARAMETER;
        goto End;
    }

    {
        const auto ctl_code = irpStack->Parameters.DeviceIoControl.IoControlCode;


        if (ctl_code == base_code) {
            if (irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(HANDLE) ||
                irpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(PVOID)) {
                status = STATUS_BUFFER_TOO_SMALL;
                irp->IoStatus.Status = status;
                irp->IoStatus.Information = 0;
                IoCompleteRequest(irp, IO_NO_INCREMENT);
                return status;
            }
            PVOID buf = irp->AssociatedIrp.SystemBuffer;
            PEPROCESS proc = NULL;
            HANDLE pid = *(HANDLE*)buf;
            status = PsLookupProcessByProcessId(pid, &proc);
            if (!NT_SUCCESS(status)) goto End;

            PVOID ptr = PsGetProcessSectionBaseAddress(proc);

            *(PVOID*)buf = ptr;
            irp->IoStatus.Information = sizeof(PVOID);

            ObDereferenceObject(proc);
            irp->IoStatus.Status = status;
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            return status;
        }

        if (ctl_code == read_code) {
            req_t* request = (req_t*)irp->AssociatedIrp.SystemBuffer;

            if (!request || irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(req_t)) {
                status = STATUS_BUFFER_TOO_SMALL;
                goto End;
            }
            if (request->length <= 0) {
                status = STATUS_INVALID_PARAMETER;
                goto End;
            }

            const ULONG length = (ULONG)request->length;
           
            if (!is_user_range(request->src, length) || !is_user_range(request->dst, length)) {
                status = STATUS_INVALID_PARAMETER;
                goto End;
            }


            PEPROCESS proc = NULL;
            status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)request->target_pid, &proc);
            if (!NT_SUCCESS(status)) goto End;
           

            KAPC_STATE target_state = { 0 };
            KeStackAttachProcess(proc, &target_state);
            MEMORY_BASIC_INFORMATION info = { 0 };
            if (!NT_SUCCESS(ZwQueryVirtualMemory(ZwCurrentProcess(), request->src, MemoryBasicInformation, &info, sizeof(MEMORY_BASIC_INFORMATION), 0))) {
                KeUnstackDetachProcess(&target_state);
                ObDereferenceObject(proc);
                status = STATUS_ACCESS_VIOLATION;
                goto End;
            }
            constexpr ULONG readable = PAGE_READONLY
                | PAGE_READWRITE
                | PAGE_EXECUTE_READ
                | PAGE_EXECUTE_READWRITE
                | PAGE_WRITECOPY
                | PAGE_EXECUTE_WRITECOPY;

            const ULONG protect = info.Protect & ~(PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE);
            if (info.State != MEM_COMMIT
                || !(protect & readable)
                || ((ULONG_PTR)request->src + length >
                    (ULONG_PTR)info.BaseAddress + info.RegionSize))
            {
                KeUnstackDetachProcess(&target_state);
                ObDereferenceObject(proc);
                status = STATUS_ACCESS_VIOLATION;
                goto End;
            }
          
           
         
            if (length == 8)
            {
                unsigned __int64 stackvar = 0;
                stackvar = *(unsigned __int64*)request->src;
                KeUnstackDetachProcess(&target_state);
                *(unsigned __int64*)request->dst = stackvar;
                goto Finish;
            }
            else if (length == 4)
            {
                ULONG stackvar = 0L;
                stackvar = *(ULONG*)request->src;
                KeUnstackDetachProcess(&target_state);
                *(ULONG*)request->dst = stackvar;
                goto Finish;
            }
            else if (length == 1)
            {
                unsigned char stackvar = 0;
                stackvar = *(unsigned char*)request->src;
                KeUnstackDetachProcess(&target_state);
                *(unsigned char*)request->dst = stackvar;
                goto Finish;
            }
            else
            {
                goto ReadBig;
            }
        ReadBig:
            PVOID buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, length, 'lolz');
            if (!buffer) {
                KeUnstackDetachProcess(&target_state);
                ObDereferenceObject(proc);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto End;
            }
            RtlCopyMemory(buffer, request->src, length);
            KeUnstackDetachProcess(&target_state);
            RtlCopyMemory(request->dst, buffer, length);
            ExFreePool(buffer);
            goto Finish;

        Finish:

            ObDereferenceObject(proc);
            goto End;
        }

        //if (ctl_code == write_code) {
        //    req_t* request = (req_t*)irp->AssociatedIrp.SystemBuffer;

        //    if (!request || irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(req_t)) {
        //        status = STATUS_BUFFER_TOO_SMALL;
        //        goto End;
        //    }
        //    if (request->length <= 0) {
        //        status = STATUS_INVALID_PARAMETER;
        //        goto End;
        //    }

        //    const ULONG length = (ULONG)request->length;
        //    if (length > (1024u * 1024u)) { // hard safety cap (1 MiB)
        //        status = STATUS_INVALID_PARAMETER;
        //        goto End;
        //    }

        //    // For write: src points to bytes in requestor; dst points to target VA.
        //    if (!is_user_range(request->src, length) || !is_user_range(request->dst, length)) {
        //        status = STATUS_INVALID_PARAMETER;
        //        goto End;
        //    }
        //    /*PUNICODE_STRING s;
        //    RtlInitUnicodeString(s, L"ZwProtectVirtualMemory");
        //    PVOID protaddress = MmGetSystemRoutineAddress(s);*/

        //    PEPROCESS target_proc = NULL;
        //    status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)request->target_pid, &target_proc);
        //    if (!NT_SUCCESS(status)) goto End;

        //    PVOID buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, length, 'File');
        //    if (!buffer) {
        //        ObDereferenceObject(target_proc);
        //        status = STATUS_INSUFFICIENT_RESOURCES;
        //        goto End;
        //    }

        // 
        //    __try {
        //        ProbeForRead(request->src, length, 1);
        //        RtlCopyMemory(buffer, request->src, length);
        //        status = STATUS_SUCCESS;
        //    }
        //    __except (EXCEPTION_EXECUTE_HANDLER) {
        //        status = GetExceptionCode();
        //        if (!NT_SUCCESS(status)) status = STATUS_ACCESS_VIOLATION;
        //    }
       
        //    if (!NT_SUCCESS(status)) {
        //        ExFreePool(buffer);
        //        ObDereferenceObject(target_proc);
        //        goto End;
        //    }

        //    // 2) Write into target process; allow writing into RX/RO pages by temporarily changing protection.
        //    KAPC_STATE target_state = {};
        //    KeStackAttachProcess(target_proc, &target_state);

        //    MEMORY_BASIC_INFORMATION info = {};
        //    status = ZwQueryVirtualMemory(
        //        ZwCurrentProcess(),
        //        request->dst,
        //        MemoryBasicInformation,
        //        &info,
        //        sizeof(info),
        //        0);

        //    if (!NT_SUCCESS(status)) {
        //        KeUnstackDetachProcess(&target_state);
        //        ExFreePool(buffer);
        //        ObDereferenceObject(target_proc);
        //        status = STATUS_ACCESS_VIOLATION;
        //        goto End;
        //    }

        //    constexpr ULONG allowed =
        //        PAGE_READONLY |
        //        PAGE_READWRITE |
        //        PAGE_EXECUTE_READ |
        //        PAGE_EXECUTE_READWRITE;

        //    ULONG protect = info.Protect & ~(PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE);
        //    if (info.State != MEM_COMMIT || !(protect & allowed)) {
        //        KeUnstackDetachProcess(&target_state);
        //        ExFreePool(buffer);
        //        ObDereferenceObject(target_proc);
        //        status = STATUS_ACCESS_VIOLATION;
        //        goto End;
        //    }

        //    ULONG old_protect = 0;
        //    bool changed_protect = false;
        //  
        //    
        //        PVOID base = info.BaseAddress;
        //        SIZE_T region_size = info.RegionSize;
        //        const bool was_executable =
        //            (protect == PAGE_EXECUTE_READ) ||
        //            (protect == PAGE_EXECUTE) ||
        //            (protect == PAGE_EXECUTE_READWRITE);

        //        ULONG new_protect = was_executable ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE;
        //        status = ZwProtectVirtualMemory(ZwCurrentProcess(), &base, &region_size, new_protect, &old_protect);
        //        if (!NT_SUCCESS(status)) {
        //            KeUnstackDetachProcess(&target_state);
        //            ExFreePool(buffer);
        //            ObDereferenceObject(target_proc);
        //            goto End;
        //        }
        //        changed_protect = true;
        //    

        //    __try {
        //        ProbeForWrite(request->dst, length, 1);
        //        RtlCopyMemory(request->dst, buffer, length);
        //        status = STATUS_SUCCESS;
        //    }
        //    __except (EXCEPTION_EXECUTE_HANDLER) {
        //        status = GetExceptionCode();
        //        if (!NT_SUCCESS(status)) status = STATUS_ACCESS_VIOLATION;
        //    }

        //    if (changed_protect) {
        //        PVOID base = info.BaseAddress;
        //        SIZE_T region_size = info.RegionSize;
        //        ULONG tmp = 0;
        //        (void)ZwProtectVirtualMemory(ZwCurrentProcess(), &base, &region_size, old_protect, &tmp);
        //    }

        //    KeUnstackDetachProcess(&target_state);

        //    ExFreePool(buffer);
        //    ObDereferenceObject(target_proc);
        //    goto End;
        //}
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto End;
    }

End:
    irp->IoStatus.Status = status;
    irp->IoStatus.Information = 0;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS real_main(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path) {
    UNREFERENCED_PARAMETER(registery_path);

   
    PDEVICE_OBJECT dev_obj;

    UNICODE_STRING dev_name = RTL_CONSTANT_STRING(L"\\Device\\daisy");
    UNICODE_STRING sym_link = RTL_CONSTANT_STRING(L"\\DosDevices\\daisy");
    auto status = IoCreateDevice(driver_obj, 0, &dev_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &dev_obj);
    if (!NT_SUCCESS(status)) return status;

    status = IoCreateSymbolicLink(&sym_link, &dev_name);
    if (!NT_SUCCESS(status)) return status;

    SetFlag(dev_obj->Flags, DO_BUFFERED_IO);

    for (unsigned int t = 0; t <= IRP_MJ_MAXIMUM_FUNCTION; t++)
    {
        driver_obj->MajorFunction[t] = [](PDEVICE_OBJECT, PIRP irp) -> NTSTATUS {
            irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
            IoCompleteRequest(irp, IO_NO_INCREMENT);
            return STATUS_NOT_SUPPORTED;
        };
    }
  
    driver_obj->MajorFunction[IRP_MJ_CREATE] = driver_obj->MajorFunction[IRP_MJ_CLOSE] =
        [](PDEVICE_OBJECT, PIRP irp) -> NTSTATUS {
        irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    };
    driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ctl_io;
    driver_obj->DriverUnload = NULL; 
    ClearFlag(dev_obj->Flags, DO_DEVICE_INITIALIZING); 
    return status;
}

extern "C" 	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);

extern "C" NTSTATUS FxDriverEntry(_In_ PDRIVER_OBJECT  DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);
    UNICODE_STRING  drv_name = RTL_CONSTANT_STRING(L"\\Driver\\daisy");
    return IoCreateDriver(&drv_name, &real_main);
}