#include <ntifs.h>
#include <ntimage.h>
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
        req_t* request = (req_t*)irp->AssociatedIrp.SystemBuffer;

        if (!request || irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(req_t)) {
            status = STATUS_BUFFER_TOO_SMALL;
            goto End;
        }

        if (ctl_code == base_code) {
            PEPROCESS proc = NULL;
            status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)request->target_pid, &proc);
            if (!NT_SUCCESS(status)) goto End;

            PVOID ptr = PsGetProcessSectionBaseAddress(proc);

            __try {
                if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(PVOID)) {
                    ExRaiseStatus(STATUS_BUFFER_TOO_SMALL);
                }
                ProbeForWrite(request->dst, sizeof(PVOID), alignof(PVOID));
                *(PVOID*)request->dst = ptr;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }

            ObDereferenceObject(proc);
            goto End;
        }

        if (ctl_code == read_code) {
      
            if (request->length <= 0) {
                status = STATUS_INVALID_PARAMETER;
                goto End;
            }

            uintptr_t start = (uintptr_t)request->src;
            ULONG     length = (ULONG)request->length;

       
            if (start > (uintptr_t)MmHighestUserAddress ||
                length > ((uintptr_t)MmHighestUserAddress - start))
            {
                status = STATUS_INVALID_PARAMETER;
                goto End;
            }

         
            PEPROCESS proc = NULL;
            status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)request->target_pid, &proc);
            if (!NT_SUCCESS(status)) goto End;


            PMDL mdl = IoAllocateMdl(request->src, length, FALSE, FALSE, NULL);
            if (!mdl) {
                ObDereferenceObject(proc);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto End;
            }

            PVOID buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, length, 'lolz');
            if (!buffer) {
                IoFreeMdl(mdl);
                ObDereferenceObject(proc);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto End;
            }

            KAPC_STATE state = { 0 };
            KeStackAttachProcess(proc, &state);
            MEMORY_BASIC_INFORMATION info = { 0 };
            if (!NT_SUCCESS(ZwQueryVirtualMemory(ZwCurrentProcess(), request->src, MemoryBasicInformation, &info, sizeof(MEMORY_BASIC_INFORMATION), 0))) {
                KeUnstackDetachProcess(&state);
                IoFreeMdl(mdl);
                ExFreePool(buffer);
                ObDereferenceObject(proc);
                status = STATUS_ACCESS_VIOLATION;
                goto End;
            }
            constexpr ULONG readable =
                PAGE_READONLY |
                PAGE_READWRITE |
                PAGE_EXECUTE_READ |
                PAGE_EXECUTE_READWRITE;

            ULONG protect = info.Protect & ~(PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE);
            if (info.State != MEM_COMMIT || !(protect & readable)) {
                KeUnstackDetachProcess(&state);
                IoFreeMdl(mdl);
                ExFreePool(buffer);
                ObDereferenceObject(proc);
                status = STATUS_ACCESS_VIOLATION;
                goto End;
            }
            __try {
                MmProbeAndLockPages(mdl, UserMode, IoReadAccess);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                KeUnstackDetachProcess(&state);
                IoFreeMdl(mdl);
                ExFreePool(buffer);
                ObDereferenceObject(proc);
                status = STATUS_ACCESS_VIOLATION;
                goto End;
            }

        /*    PVOID source = MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority | MdlMappingNoExecute);
            if (!source) {
                MmUnlockPages(mdl);
                KeUnstackDetachProcess(&state);
                IoFreeMdl(mdl);
                ExFreePool(buffer);
                ObDereferenceObject(proc);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto End;
            }*/

            RtlCopyMemory(buffer, request->src, length);


            MmUnlockPages(mdl);
            KeUnstackDetachProcess(&state);
            IoFreeMdl(mdl);

            __try {
                ProbeForWrite(request->dst, length, 1);
                RtlCopyMemory(request->dst, buffer, length);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                status = STATUS_ACCESS_VIOLATION;
            }

            ExFreePool(buffer);
            ObDereferenceObject(proc);
            goto End;
        }
        status = STATUS_INVALID_DEVICE_REQUEST;
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