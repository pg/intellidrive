#include <ntddk.h>

#define FILE_HANDLE HANDLE

#define sysDebugPrint(msg, ...) DbgPrint(msg, __VA_ARGS__) 

void* sysAllocateMemory(int size);

void sysFreeMemory(void* memory);

void sysCopyMemory(void* dest, void* src, int length);

FILE_HANDLE sysOpenFile(IN const char* filename);

int sysReadFile(
	IN FILE_HANDLE		handle,
	IN void*			buffer,
	IN long long		offset,
	IN unsigned long	length
);

int sysWriteFile(
	IN FILE_HANDLE		handle,
	IN void*			buffer,
	IN long long		offset,
	IN unsigned long 	length
);

void sysCloseFile(IN FILE_HANDLE handle);
LARGE_INTEGER sysGetFileSize(IN FILE_HANDLE handle);

unsigned long getSystemTime();
