#include "pch.h"
#include "Logger.h"

namespace XYZEngine
{
	std::string LogLevelToString(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Info:
			return "[INFO]";
		case LogLevel::Warning:
			return "[WARNING]";
		case LogLevel::Error:
			return "[ERROR]";
		default:
			return "[UNKNOWN]";
		}
	}

	void Logger::AddSink(std::shared_ptr<LogSink> sink)
	{
		if (sink == nullptr)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(logMutex);
		sinks.push_back(sink);
	}

	void Logger::Log(LogLevel level, const std::string& message)
	{
		std::lock_guard<std::mutex> lock(logMutex);
		for (auto& sink : sinks)
		{
			sink->Log(level, message);
		}
	}

	void Logger::Info(const std::string& message)
	{
		Log(LogLevel::Info, message);
	}
	void Logger::Warning(const std::string& message)
	{
		Log(LogLevel::Warning, message);
	}
	void Logger::Error(const std::string& message)
	{
		Log(LogLevel::Error, message);
	}
}
