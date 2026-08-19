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
		}
	}
	FileSink::~FileSink()
	{
		if (logFile.is_open())
		{
			logFile.close();
		}
	}

	void FileSink::Log(LogLevel level, const std::string& message)
	{
		if (!logFile.is_open())
		{
			return;
		}

		logFile << LogLevelToString(level) << " " << message << std::endl;
	}
}
