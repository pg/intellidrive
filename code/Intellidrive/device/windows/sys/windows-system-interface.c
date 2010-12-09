#include "system-interface.h"

void* sysAllocateMemory(int size)
{
	return ExAllocatePool(NonPagedPool, size);
}

void sysFreeMemory(void* memory)
{
	ExFreePool(memory);
}

void sysCopyMemory(void* dest, void* src, int length)
{
	RtlCopyMemory(dest, src, length);
}

FILE_HANDLE sysOpenFile(IN const char* filename)
{
	HANDLE handle;
	OBJECT_ATTRIBUTES objAttr;	
	IO_STATUS_BLOCK ioStatus;
	ANSI_STRING afilename;
    UNICODE_STRING ufilename;
	NTSTATUS status;
	
	RtlInitAnsiString(&afilename, filename);
	
    status = RtlAnsiStringToUnicodeString(
        &ufilename,
        &afilename,
        TRUE);	
		
	if(status != STATUS_SUCCESS)
	{
		return NULL;
	}
		
	InitializeObjectAttributes(
        &objAttr,
        &ufilename,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL,
        NULL);

	status = ZwCreateFile(
        &handle,
        GENERIC_READ | GENERIC_WRITE,
        &objAttr,
        &ioStatus,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN_IF,
        FILE_NON_DIRECTORY_FILE |
        FILE_RANDOM_ACCESS |
        FILE_NO_INTERMEDIATE_BUFFERING |
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );

    if (status != STATUS_SUCCESS)
    {
		return NULL;
	}
	
	RtlFreeUnicodeString(&ufilename);
	
	return handle;
}

int sysReadFile(
	IN FILE_HANDLE handle,
	IN void* buffer,
	IN long long offset,
	IN unsigned long length)
{
	IO_STATUS_BLOCK IoStatus;
	LARGE_INTEGER byteOffset;
	NTSTATUS status;
	
	byteOffset.QuadPart = offset;
	
	status = ZwReadFile(
		handle,
		NULL,
		NULL,
		NULL,
		&IoStatus,
		buffer,
		length,
		&byteOffset,
		NULL
		);

	if(status != STATUS_SUCCESS)
	{
		return -1;
	}
	
	return IoStatus.Information;
}

int sysWriteFile(
	IN FILE_HANDLE		handle,
	IN void*			buffer,
	IN long long		offset,
	IN unsigned long 	length)
{
	IO_STATUS_BLOCK IoStatus;
	LARGE_INTEGER byteOffset;
	NTSTATUS status;
	
	byteOffset.QuadPart = offset;
	
	status = ZwWriteFile(
		handle,
		NULL,
		NULL,
		NULL,
		&IoStatus,
		buffer,
		length,
		&byteOffset,
		NULL
		);

	if(status != STATUS_SUCCESS)
	{
		return -1;
	}
	
	return IoStatus.Information;	
}

void sysCloseFile(IN FILE_HANDLE handle)
{
    ZwClose(handle);
}

LARGE_INTEGER sysGetFileSize(IN FILE_HANDLE handle)
{
    IO_STATUS_BLOCK  IoStatusBlock;
	FILE_STANDARD_INFORMATION FileInformation;
	NTSTATUS status;

	status = ZwQueryInformationFile( 
		handle,
		&IoStatusBlock,
		&FileInformation,
		sizeof(FileInformation),
		FileStandardInformation
		);

	if(status != STATUS_SUCCESS)
	{
		LARGE_INTEGER retVal;
		retVal.QuadPart = -1;
		return retVal;
	}

	return(FileInformation.EndOfFile);
}

unsigned long getSystemTime()
{
	LARGE_INTEGER time;	
	KeQuerySystemTime(&time);
	
	return (unsigned long) ((time.QuadPart / 10000000) - 11643609600);
}

