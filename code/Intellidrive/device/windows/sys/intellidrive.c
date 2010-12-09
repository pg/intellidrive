#include "intellidrive.h"
#include <stdarg.h>

#define BLOCK_SIZE 4096
#define LOG_BUFFER_SIZE (2*1024*1024)
#define CACHE_BUFFER_SIZE (256*1024)
#define MIN_PRECACHE_LENGTH (256*1024)
#define BUFFER_SIZE 4096

#define BLOCK_MAPPING_ENABLED 0
#define CACHE_ENABLED 0

FILE_HANDLE glogFile = NULL;

int __cdecl _snprintf(char *, size_t, const char *, ...);
int __cdecl _vsnprintf(char *, size_t, const char *, ...);

int precacheBoundaryAtBlock(int* precacheBoundaryMap, int blockNumber);

int readPhysicalBlocks(
	IN INTELLIDRIVE_CONTEXT*	context,
    IN long long		   	    offset,
    IN unsigned long            length,
    OUT void*              		dataBuffer);

int writePhysicalBlocks(
	IN INTELLIDRIVE_CONTEXT*	context,
    IN long long		   	    offset,
    IN unsigned long            length,
    OUT void*              		dataBuffer);

void IntellidriveLogAccess(INTELLIDRIVE_LOG* log, long long offset, unsigned long length);
void IntellidriveWriteLog(INTELLIDRIVE_LOG* log);
void IntellidriveWriteTextLog(const char* fmt, ...);

INTELLIDRIVE_CONTEXT IntellidriveOpenDrive()
{
	INTELLIDRIVE_CONTEXT context;
	char* blockMapFilename = "\\DosDevices\\c:\\intellidrive\\blockmap.bin";
	FILE_HANDLE blockMapFile;

	sysDebugPrint("Intellidrive: OpenDrive\n");
	
	// Initialize read log
	context.readLog.bufferPosition = 0;
	context.readLog.bufferCapacity = LOG_BUFFER_SIZE / sizeof(unsigned long);
	context.readLog.buffer = sysAllocateMemory(LOG_BUFFER_SIZE);
	
	if(BLOCK_MAPPING_ENABLED)
	{
		sysDebugPrint("Intellidrive: Initializing block mapping\n");
		
		blockMapFile = sysOpenFile(blockMapFilename);	

		if(blockMapFile != NULL)
		{
			LARGE_INTEGER size;
			size = sysGetFileSize(blockMapFile);
			context.blockMap = sysAllocateMemory((int)size.QuadPart);
			context.blockMapSize = (int)size.QuadPart;
			sysReadFile(blockMapFile, context.blockMap, 0, context.blockMapSize );	
			sysCloseFile(blockMapFile);

			sysDebugPrint("Intellidrive: Successfully opened and read in blockMapFile with %d bytes: %s\n", context.blockMapSize, blockMapFilename );
		}
		else
		{
			sysDebugPrint("Intellidrive: Error opening the blockMapFile: %s\n", blockMapFilename);
		}
	}
	
	if(CACHE_ENABLED)
	{
		sysDebugPrint("Intellidrive: Initializing cache");
		
		// Initialize cache
		context.cache.buffer = sysAllocateMemory(CACHE_BUFFER_SIZE);
		context.cache.offset = 0;
		context.cache.length = 0;
	
		// TODO: Initialize precache boundary map
	}
	
	return context;
}

void IntellidriveCloseDrive(INTELLIDRIVE_CONTEXT* context)
{
	sysDebugPrint("Intellidrive: CloseDrive\n");

	IntellidriveWriteLog(&context->readLog);

	sysFreeMemory(context->readLog.buffer);

	if(BLOCK_MAPPING_ENABLED)
	{
		sysFreeMemory(context->blockMap);	
	}

	if(CACHE_ENABLED)
	{
		sysFreeMemory(context->cache.buffer);
	}
	
	if( glogFile != NULL )
	{
		sysCloseFile(glogFile);
	}
}

int IntellidriveReadData (
	IN INTELLIDRIVE_CONTEXT*	context,
    IN long long		   	    offset,
    IN unsigned long            length,
    OUT void*              		dataBuffer)
{
	unsigned long blockCount, dataBufOffset, extraBlock, i, j, k, startOff;
	long lenCorrection;
	int retVal = 0;
	int* mappedArray;

	IntellidriveLogAccess(&context->readLog, offset, length);

	if(!BLOCK_MAPPING_ENABLED)
	{
		return readPhysicalBlocks(context, offset, length, dataBuffer);
	}

	extraBlock = (((length > BLOCK_SIZE)&&(length % BLOCK_SIZE))?1:0) + ((offset+(length % BLOCK_SIZE) > BLOCK_SIZE)?1:0);
	blockCount = (length / BLOCK_SIZE) + extraBlock;
	blockCount = ( blockCount > 0 ) ? blockCount : 1;
    mappedArray = sysAllocateMemory(blockCount*sizeof(int));

	CreateSortedMappedArray(mappedArray, blockCount, context, offset);

	sysDebugPrint("Intellidrive: Actually reading the following: ");

	startOff = (unsigned long)offset % BLOCK_SIZE;     // byte offset off of first block to read
	lenCorrection = 0;								   // correction to verify we don't read too far
	dataBufOffset = 0;                                 // data offset in output array
	k = 0;
	for(i = 0; i < blockCount; ++i)
	{
		sysDebugPrint("%d ", mappedArray[i]);
		for( j = i+1; j < blockCount; ++j )
		{
			if( (mappedArray[j] - mappedArray[j-1]) != 1 )
			{
				break;
			}
			sysDebugPrint("%d ", mappedArray[j]);
			i++;
		}
		sysDebugPrint(", ");

		// Make sure we don't read past the length requested
		if( ((BLOCK_SIZE * (j-k)) + dataBufOffset - startOff) > length )
		{
			lenCorrection = length - ((BLOCK_SIZE * (j-k)) + dataBufOffset - startOff);
			if( lenCorrection < 0 )
			{
				lenCorrection *= -1;
			}
		}

//		sysDebugPrint("dataBufOffset = %d, startOff = %d, length = %d, ", dataBufOffset, startOff, length);
//		sysDebugPrint("lenCorrection = %d, extraBlock = %d, blockCount = %d, offset = %d", lenCorrection, extraBlock, blockCount, (unsigned long)offset);

		if( length != ((BLOCK_SIZE * (j-k)) - lenCorrection - startOff) )
		{
			sysDebugPrint("length = %d, offset = %d, k = %d, mappedarray[k] = %d\n", (BLOCK_SIZE * (j-k)) - lenCorrection - startOff, (mappedArray[k] * BLOCK_SIZE) + startOff, k, mappedArray[k]);
			sysDebugPrint("length = %d, offset = %d\n", length, (unsigned long)offset);
			sysDebugPrint("!!!ERROR!!!  Length and offset are not as expected");
		}
		
		if( offset != ((mappedArray[k] * BLOCK_SIZE) + startOff) )
		{
			sysDebugPrint("length = %d, offset = %d, k = %d, mappedarray[k] = %d\n", (BLOCK_SIZE * (j-k)) - lenCorrection - startOff, (mappedArray[k] * BLOCK_SIZE) + startOff, k, mappedArray[k]);
			sysDebugPrint("length = %d, offset = %d\n", length, (unsigned long)offset);
			sysDebugPrint("!!!ERROR!!!  Length and offset are not as expected");
		}
		
		retVal = readPhysicalBlocks(context, (mappedArray[k] * BLOCK_SIZE) + startOff, 
			(BLOCK_SIZE * (j-k)) - lenCorrection - startOff, (void*)((unsigned long)dataBuffer+dataBufOffset));
		dataBufOffset += retVal;
		startOff = 0;
		k = j;
	}

	sysDebugPrint("\n");
	sysFreeMemory(mappedArray);

	return dataBufOffset;
}

int precacheBoundaryAtBlock(int* precacheBoundaryMap, int blockNumber)
{
	if(precacheBoundaryMap == NULL)
	{
		return 1;
	}
	
	// Access the bit corresponding to blockNumber.  The bit is stored in the integer at
	// index blockNumber / 32 in the array.  The bit is at position blockNumber %32 in the
	// integer
	//
	// Example:
	// If 4 bit integers are used
	// [1101] [0011] [1110] [1011] [0110]
	// 
	// The bit corresponding to block 11 would at index 11 / 4 = 2 and would be bit
	// number 11 % 4 = 3 
	return (precacheBoundaryMap[blockNumber / 32] >> (31 - blockNumber % 32)) & 1;
}

void cacheData(
	IN INTELLIDRIVE_CONTEXT* 	context,
	IN long long 				offset, 
	IN unsigned long 			length)
{
	int bytesRead, overflow;
	unsigned int idx;
	
	if(!CACHE_ENABLED)
	{
		return;
	}
	
	bytesRead = sysReadFile(
		context->fileHandle,
		context->cache.buffer,
		offset,
		length);
	
	context->cache.offset = offset;
	context->cache.length = bytesRead;	

	sysDebugPrint("Intellidrive: Cached %u bytes starting at byte %u\n", (unsigned long) bytesRead, (unsigned long) offset);
}

int readPhysicalBlocks(
	IN INTELLIDRIVE_CONTEXT*	context,
    IN long long		   	    offset,
    IN unsigned long            length,
    OUT void*              		dataBuffer)
{
	unsigned int bytesRead = 0, cacheBufferOffset, idx;
	
	if(!CACHE_ENABLED)
	{
		bytesRead = sysReadFile(
			context->fileHandle,
			dataBuffer,
			offset,
			length);			
	
		return bytesRead;
	}
	
	// Determine if the requested data is not contained in the cache
	
	if(offset < context->cache.offset ||
	   offset + length > context->cache.offset + context->cache.length)
	{
		sysDebugPrint("Intellidrive: Cache miss\n");
	
		if(length < MIN_PRECACHE_LENGTH)
		{
			cacheData(context, offset, MIN_PRECACHE_LENGTH);
		} else
		{
			cacheData(context, offset, length);
		}
	} else
	{
		sysDebugPrint("Intellidrive: Cache hit\n");
	}
	
	// Copy cached data into the buffer
		
	cacheBufferOffset = (unsigned int) (offset - context->cache.offset);
	
	sysCopyMemory(
		dataBuffer, 
		&((char*)context->cache.buffer)[cacheBufferOffset], 
		length);
		
	return length;
}

int IntellidriveWriteData (
    IN INTELLIDRIVE_CONTEXT*	context,
    IN long long	     		offset,
    IN unsigned long           	length,
    IN void*					dataBuffer)
{		
	unsigned long blockCount, dataBufOffset, extraBlock, i, j, k, startOff, blockOffset;
	long lenCorrection;
	int retVal = 0;
	int* mappedArray;

	if(!BLOCK_MAPPING_ENABLED)
	{
		return writePhysicalBlocks(context, offset, length, dataBuffer);
	}

	extraBlock = (((length > BLOCK_SIZE)&&(length % BLOCK_SIZE))?1:0) + ((offset+(length % BLOCK_SIZE) > BLOCK_SIZE)?1:0);
	blockCount = (length / BLOCK_SIZE) + extraBlock;
	blockCount = ( blockCount > 0 ) ? blockCount : 1;
    mappedArray = sysAllocateMemory(blockCount*sizeof(int));
    
	CreateSortedMappedArray(mappedArray, blockCount, context, offset);

	sysDebugPrint("Intellidrive: Actually writing the following: ");

	startOff = (unsigned long)offset % BLOCK_SIZE;
	lenCorrection = 0;
	dataBufOffset = 0;
	k = 0;
	for(i = 0; i < blockCount; ++i)
	{
		sysDebugPrint("%d ", mappedArray[i]);
		for( j = i+1; j < blockCount; ++j )
		{
			if( (mappedArray[j] - mappedArray[j-1]) != 1 )
			{
				break;
			}
			sysDebugPrint("%d ", mappedArray[j]);
			i++;
		}
		sysDebugPrint(", ");

		if( ((BLOCK_SIZE * (j-k)) + dataBufOffset - startOff) > length )
		{
			lenCorrection = length - ((BLOCK_SIZE * (j-k)) + dataBufOffset - startOff);
			if( lenCorrection < 0 )
			{
				lenCorrection *= -1;
			}
		}

		if( length != ((BLOCK_SIZE * (j-k)) - lenCorrection - startOff) )
		{
			sysDebugPrint("length = %d, offset = %d, k = %d, mappedarray[k] = %d\n", (BLOCK_SIZE * (j-k)) - lenCorrection - startOff, (mappedArray[k] * BLOCK_SIZE) + startOff, k, mappedArray[k]);
			sysDebugPrint("length = %d, offset = %d\n", length, (unsigned long)offset);
			sysDebugPrint("!!!ERROR!!!  Length and offset are not as expected");
		}
		
		if( offset != ((mappedArray[k] * BLOCK_SIZE) + startOff) )
		{
			sysDebugPrint("length = %d, offset = %d, k = %d, mappedarray[k] = %d\n", (BLOCK_SIZE * (j-k)) - lenCorrection - startOff, (mappedArray[k] * BLOCK_SIZE) + startOff, k, mappedArray[k]);
			sysDebugPrint("length = %d, offset = %d\n", length, (unsigned long)offset);
			sysDebugPrint("!!!ERROR!!!  Length and offset are not as expected");
		}
		
		retVal = writePhysicalBlocks(context, (mappedArray[k] * BLOCK_SIZE) + startOff, 
			(BLOCK_SIZE * (j-k)) - lenCorrection - startOff, (void*)((unsigned long)dataBuffer+dataBufOffset));
		dataBufOffset += retVal;
		startOff = 0;
		k = j;
	}
	
	sysDebugPrint("\n");
	sysFreeMemory(mappedArray);

	return dataBufOffset;
}

int writePhysicalBlocks(
	IN INTELLIDRIVE_CONTEXT*	context,
    IN long long		   	    offset,
    IN unsigned long            length,
    OUT void*              		dataBuffer)
{
	unsigned long bytesWritten;

	if(CACHE_ENABLED)
	{		
		// Invalidate the cache
		sysDebugPrint("Invalidating the cache");
		context->cache.offset = 0;
		context->cache.length = 0;
	}

	bytesWritten = sysWriteFile(
		context->fileHandle,
		dataBuffer,
		offset,
		length);
		
	return bytesWritten;
}

int IntellidriveExecuteIdleTasks(INTELLIDRIVE_CONTEXT* context)
{
	sysDebugPrint("Intellidrive: Executing idle tasks\n");

	/*
	// Check if read log buffer should be flushed to disk
	if(context->readLog.bufferPosition >= context->readLog.bufferCapacity / 2)
	{
		IntellidriveWriteLog(&context->readLog);
	}
    */
	
	sysDebugPrint("Intellidrive: Idle tasks complete\n");
	
	return 0;
}

void IntellidriveLogAccess(
	INTELLIDRIVE_LOG* log, 
	long long offset, 
	unsigned long length)
{
	unsigned long startBlock, endBlock, blockCount, blockNumber;

	startBlock = (unsigned long) offset / BLOCK_SIZE;
	blockCount = length / BLOCK_SIZE;
	
	if(blockCount == 0)
	{
		return;
	}
	
	endBlock = startBlock + blockCount - 1;
	
	for(blockNumber = startBlock; blockNumber <= endBlock; blockNumber++)
	{
		log->buffer[log->bufferPosition] = getSystemTime();
		
		log->bufferPosition++;
		
		log->buffer[log->bufferPosition] = blockNumber;
		log->bufferPosition++;
		
		if(log->bufferPosition >= log->bufferCapacity)
		{
			IntellidriveWriteLog(log);
		}
	}
}

void IntellidriveWriteLog(INTELLIDRIVE_LOG* log)
{	
	FILE_HANDLE logFile;
	char* logFilename;
	long startTime;

	if(log->bufferPosition == 0)
	{
		return;
	}

	startTime = log->buffer[0];

	logFilename = sysAllocateMemory(BUFFER_SIZE);
	
	_snprintf(
		logFilename, 
		BUFFER_SIZE, 
		"\\DosDevices\\c:\\intellidrive\\log\\%u.log", 
		startTime);
		
	sysDebugPrint("Intellidrive: Writing log to %s\n", logFilename);

	logFile = sysOpenFile(logFilename);	

	if(logFile != NULL)
	{
		sysWriteFile(logFile, log->buffer, 0, log->bufferPosition * sizeof(unsigned long));	
		sysCloseFile(logFile);
	}
	
	sysFreeMemory(logFilename);
	
	log->bufferPosition = 0;
}

void IntellidriveWriteTextLog(const char* fmt, ...)
{	
	char* log;
	va_list arg;
	
	if( glogFile == NULL )
	{
		long startTime;
		char* logFilename;
	
		startTime = getSystemTime();
	
		logFilename = sysAllocateMemory(BUFFER_SIZE);
		
		_snprintf(
			logFilename, 
			BUFFER_SIZE, 
			"\\DosDevices\\c:\\intellidrive\\log\\textLog%u.log", 
			startTime);
			
		glogFile = sysOpenFile(logFilename);	
	}

	log = sysAllocateMemory(BUFFER_SIZE);

	va_start(arg, fmt);
	_vsnprintf(log, BUFFER_SIZE-1, fmt, arg);
	va_end(arg);

	sysDebugPrint(log);
	if(glogFile != NULL)
	{
		sysWriteFile(glogFile, log, 0, strlen(log)+1);	
	}
	
	sysFreeMemory(log);
}

void CreateSortedMappedArray(OUT int* mappedArray, IN unsigned long blockCount, 
							 IN INTELLIDRIVE_CONTEXT* context, IN long long offset)
{
	unsigned long startBlock, endBlock, blockNumber, dataBufOffset;
	int i;
 
	startBlock = (unsigned long) offset / BLOCK_SIZE;
	endBlock = startBlock + blockCount - 1;
	
	dataBufOffset = 0;
	i = 0;
	sysDebugPrint("Intellidrive: Told to read: ");
	for(blockNumber = startBlock; blockNumber <= endBlock; blockNumber++)
	{
		sysDebugPrint("%d ", blockNumber);
		mappedArray[i++] = context->blockMap[blockNumber];
	}
	sysDebugPrint("\n");

	ArraySort(mappedArray, cmpfun, blockCount);
}


#define INSERTION_SORT_BOUND 16 /* boundary point to use insertion sort */
 
/* explain function
 * Description:
 *   fixarray::Qsort() is an internal subroutine that implements quick sort.
 *
 * Return Value: none
 */
void Qsort(int This[], CMPFUN fun_ptr, uint32 first, uint32 last)
{
  uint32 stack_pointer = 0;
  int first_stack[32];
  int last_stack[32];

  for (;;)
  {
    if (last - first <= INSERTION_SORT_BOUND)
    {
      /* for small sort, use insertion sort */
      uint32 indx;
      int prev_val = This[first];
      int cur_val;

      for (indx = first + 1; indx <= last; ++indx)
      {
        cur_val = This[indx];
        if ((*fun_ptr)(prev_val, cur_val) > 0)
        {
          /* out of order: array[indx-1] > array[indx] */
          uint32 indx2;
          This[indx] = prev_val; /* move up the larger item first */

          /* find the insertion point for the smaller item */
          for (indx2 = indx - 1; indx2 > first; )
          {
            int temp_val = This[indx2 - 1];
            if ((*fun_ptr)(temp_val, cur_val) > 0)
            {
              This[indx2--] = temp_val;
              /* still out of order, move up 1 slot to make room */
            }
            else
              break;
          }
          This[indx2] = cur_val; /* insert the smaller item right here */
        }
        else
        {
          /* in order, advance to next element */
          prev_val = cur_val;
        }
      }
    }
    else
    {
      int pivot;
 
      /* try quick sort */
      {
        int temp;
        uint32 med = (first + last) >> 1;
        /* Choose pivot from first, last, and median position. */
        /* Sort the three elements. */
        temp = This[first];
        if ((*fun_ptr)(temp, This[last]) > 0)
        {
          This[first] = This[last]; This[last] = temp;
        }
        temp = This[med];
        if ((*fun_ptr)(This[first], temp) > 0)
        {
          This[med] = This[first]; This[first] = temp;
        }
        temp = This[last];
        if ((*fun_ptr)(This[med], temp) > 0)
        {
          This[last] = This[med]; This[med] = temp;
        }
        pivot = This[med];
      }
      {
        uint32 up;
        {
	  uint32 down;
          /* First and last element will be loop stopper. */
	  /* Split array into two partitions. */
	  down = first;
	  up = last;
	  for (;;)
	  {
	    do
	    {
	      ++down;
	    } while ((*fun_ptr)(pivot, This[down]) > 0);
 
	    do
	    {
	      --up;
	    } while ((*fun_ptr)(This[up], pivot) > 0);
 
	    if (up > down)
	    {
	      int temp;
	      /* interchange L[down] and L[up] */
	      temp = This[down]; This[down]= This[up]; This[up] = temp;
	    }
	    else
	      break;
	  }
	}
	{
	  uint32 len1; /* length of first segment */
	  uint32 len2; /* length of second segment */
	  len1 = up - first + 1;
	  len2 = last - up;
	  /* stack the partition that is larger */
	  if (len1 >= len2)
	  {
	    first_stack[stack_pointer] = first;
	    last_stack[stack_pointer++] = up;
 
	    first = up + 1;
	    /*  tail recursion elimination of
	     *  Qsort(This,fun_ptr,up + 1,last)
	     */
	  }
	  else
	  {
	    first_stack[stack_pointer] = up + 1;
	    last_stack[stack_pointer++] = last;

	    last = up;
	    /* tail recursion elimination of
	     * Qsort(This,fun_ptr,first,up)
	     */
	  }
	}
        continue;
      }
      /* end of quick sort */
    }
    if (stack_pointer > 0)
    {
      /* Sort segment from stack. */
      first = first_stack[--stack_pointer];
      last = last_stack[stack_pointer];
    }
    else
      break;
  } /* end for */
}
 
 
void ArraySort(int This[], CMPFUN fun_ptr, uint32 the_len)
{
  Qsort(This, fun_ptr, 0, the_len - 1);
}

int cmpfun(int a, int b)
{
  if (a > b)
    return 1;
  else if (a < b)
    return -1;
  else
    return 0;
}
 