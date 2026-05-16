#include "buf.h"
bool PoolBuffer::Allocate(unsigned int len)
{
	length = 0;
	ptr = ExAllocatePool2(POOL_FLAG_NON_PAGED, len, 'File');
	if (!ptr)
	{
		return false;
	}
	length = len;
	return true;
}
void PoolBuffer::Destroy()
{
	if (ptr && length > 0)
	{
		ExFreePool(ptr);
	}
}
NTSTATUS PoolBuffer::EnsureReady(unsigned int newLen)
{
	if (!ptr)
	{
		return Allocate(newLen) ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES;
	}
	if (length < newLen)
	{
		Destroy();
		return Allocate(newLen) ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES;
	}
	return STATUS_SUCCESS;
}