#include "system-interface.h"

typedef struct INTELLIDRIVE_LOG
{
	unsigned long*	buffer;
	unsigned int	bufferPosition;
	unsigned int	bufferCapacity;
} INTELLIDRIVE_LOG, *PINTELLIDRIVE_LOG;

typedef struct INTELLIDRIVE_CACHE
{
	void*			buffer;
	long long		offset;
	unsigned int 	length;
	int*			precacheBoundayMap;
} INTELLIDRIVE_CACHE, *PINTELLIDRIVE_CACHE;

typedef struct _INTELLIDRIVE_CONTEXT
{
	FILE_HANDLE				fileHandle;
	int*					blockMap;
	int						blockMapSize;	
	INTELLIDRIVE_CACHE		cache;
	INTELLIDRIVE_LOG		readLog;
} INTELLIDRIVE_CONTEXT, *PINTELLIDRIVE_CONTEXT;

INTELLIDRIVE_CONTEXT IntellidriveOpenDrive();

void IntellidriveCloseDrive(INTELLIDRIVE_CONTEXT* context);

int IntellidriveReadData (
	IN INTELLIDRIVE_CONTEXT*	context,
    IN long long		   	    offset,
    IN unsigned long            length,
    OUT void*              		dataBuffer
);

int IntellidriveWriteData (
    IN INTELLIDRIVE_CONTEXT*	context,
    IN long long	     		offset,
    IN unsigned long           	length,
    IN void*					dataBuffer
);

/* 
	Executes tasks which should be performed while the drive is idle. A call
	to this method should not take more than a fraction of a second to complete.  
	Long tasks should be broken up so that they can be completed over multiple 
	calls to this method.  
	
	Returns 0 if all idle tasks are complete
*/
int IntellidriveExecuteIdleTasks(INTELLIDRIVE_CONTEXT* context);


void CreateSortedMappedArray(OUT int* mappedArray, IN unsigned long len, 
							 IN INTELLIDRIVE_CONTEXT* context, IN long long offset);

 
#define uint32 unsigned int
 
typedef int (*CMPFUN)(int, int);
void Qsort(int This[], CMPFUN fun_ptr, uint32 first, uint32 last);
void ArraySort(int This[], CMPFUN fun_ptr, uint32 the_len);
int cmpfun(int a, int b);

