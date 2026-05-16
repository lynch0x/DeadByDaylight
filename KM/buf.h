#pragma once
#include <ntifs.h>

class PoolBuffer {
public:
	void* ptr;
	unsigned int length;
	NTSTATUS EnsureReady(unsigned int len);
	
private:
	bool Allocate(unsigned int len);
	void Destroy();
};