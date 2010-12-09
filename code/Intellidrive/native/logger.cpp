#include "logging.h"
#include <sys/time.h>
#include <time.h>
//#include <stdio>

using namespace std;

/**
 * This test produces a file to be manually checked against the couts
 */
int testLogger()
{
	Logger* log = Logger::GetInstance();
	log->LogWrite(0x502);
	log->LogRead(0x100);
	log->LogRead(0x101);
	log->LogRead(0x102);
	log->LogWrite(0x602);
	log->LogWrite(0x702);
	log->Flush();

	return 0;
}

Logger::Logger* Logger::s_instance = NULL;

Logger::Logger* Logger::GetInstance()
{
	if( s_instance == NULL )
	{
		s_instance = new Logger();
	}
	return( s_instance );
}

Logger::Logger()
{
	char filename[255];
	struct tm* tm;
	time_t now;
	now = time(0); // get current time
	tm = localtime(&now); // get structure
	sprintf(filename, "readWriteSectors_%02d%02d%02d_%02d%02d%02d.bin", tm->tm_mon+1,
	    tm->tm_mday, tm->tm_year % 100, tm->tm_hour, tm->tm_min, tm->tm_sec);

	lastTimeStamp = 0;
	logFile = new std::fstream(filename, fstream::out | fstream::binary | fstream::trunc);
}

Logger::~Logger()
{
	logFile->flush();
	logFile->close();
}

void Logger::Flush()
{
	logFile->flush();
}

void Logger::LogRead(unsigned long block)
{
	unsigned long time = GetTime();
	if( (time - lastTimeStamp) > TIME_DIFF_TO_READ )
	{
		WriteTimeStampAndData(READ_BLOCK, time, block);
	}
	else
	{
		WriteData( READ_BLOCK, block );
	}
	lastTimeStamp = time;
}

void Logger::LogWrite(unsigned long block)
{
	unsigned long time = GetTime();
	if( (time - lastTimeStamp) > TIME_DIFF_TO_READ )
	{
		WriteTimeStampAndData(WRITE_BLOCK, time, block);
	}
	else
	{
		WriteData( WRITE_BLOCK, block );
	}
	lastTimeStamp = time;
}

void Logger::WriteTimeStampAndData(BlockType type, unsigned long timestamp, unsigned long data)
{
	loggingTimestampBlock time;
	time.data.type = type | WITH_TIMESTAMP;
	time.data.timestamp = timestamp;
	time.data.block = data;
	time.data.unused = 0;

	cout << "writing " << hex << time.word[0] << hex << time.word[1] << " to file with timestamp" << endl;
	logFile->write(reinterpret_cast<char *>(&time),sizeof(time));
	//*logFile << hex << time.word;
}

void Logger::WriteData(BlockType type, unsigned long data)
{
	loggingBlock dataStr;
	dataStr.data.type = type | WITHOUT_TIMESTAMP;
	dataStr.data.block = data;

	cout << "writing " << hex << dataStr.word << " to file without timestamp" << endl;
	logFile->write(reinterpret_cast<char *>(&dataStr),sizeof(dataStr));
	//*logFile << hex << dataStr.word;
}

unsigned long Logger::GetTime()
{
	timeval tVal;
	unsigned long time = 0;
	int ret = gettimeofday(&tVal, NULL);
	if( ret == 0 )
	{
		unsigned long usec = tVal.tv_usec;
		unsigned long sec = tVal.tv_sec;
		time = (sec*1000) + (usec / 1000);
	}
	return( time );
}

