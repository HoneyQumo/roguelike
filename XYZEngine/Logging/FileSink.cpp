#include "pch.h"
#include "FileSink.h"
#include <iostream>

namespace XYZEngine
{
	FileSink::FileSink(const std::string& filePath)
	{
		logFile.open(filePath, std::ios::app);
		if (!logFile.is_open())
		{
			std::cout << "Can't open log file: " << filePath << std::endl;
			return;
		}

		logFile << "===== run started " << CurrentDateTimeText() << " =====" << std::endl;
	}
	FileSink::~FileSink()
	{
		if (logFile.is_open())
		{
			logFile.close();
		}
	}

	void FileSink::Log(const LogEntry& entry)
	{
		if (!logFile.is_open())
		{
			return;
		}

		logFile << FormatLogLine(entry) << std::endl;
	}
}
