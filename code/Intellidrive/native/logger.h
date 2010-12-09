#include <stdio.h>
#include <fstream>
#include <iostream>

class Logger
{

public:
	static Logger* GetInstance();
	void LogRead(unsigned long block);
	void LogWrite(unsigned long block);
	void Flush();

private:  // Enums and structures
	enum BlockType
	{
		WRITE_BLOCK = 0x1,
		READ_BLOCK  = 0x0
	};

	enum TimestampType
	{
		WITH_TIMESTAMP = 0x2,
		WITHOUT_TIMESTAMP = 0x0
	};

	union loggingBlock
	{
		unsigned int word;
		struct
		{
			unsigned int block     : 30;
			unsigned int type      : 2;
		}data;
	};

	union loggingTimestamp
	{
		unsigned int word;
		struct
		{
			unsigned int timestamp : 30;
			unsigned int type      : 2;
		}data;
	};

	union loggingTimestampBlock
	{
		unsigned int word[2];
		struct
		{
			unsigned int timestamp : 30;
			unsigned int type      : 2;
			unsigned int block     : 30;
			unsigned int unused    : 2;
		}data;
	};

private:  // member variables
	unsigned long lastTimeStamp;
	std::fstream* logFile;

	static const unsigned long TIME_DIFF_TO_READ = 100;
	static Logger* s_instance;

private:  // member methods
	Logger();
	~Logger();
	unsigned long GetTime();
	void WriteTimeStampAndData(BlockType type, unsigned long timestamp, unsigned long data);
	void WriteData(BlockType type, unsigned long data);
};

