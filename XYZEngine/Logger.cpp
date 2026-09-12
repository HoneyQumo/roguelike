#include "pch.h"
#include "Logger.h"
#include "FrameClock.h"
#include <chrono>
#include <cstdio>
#include <ctime>

namespace XYZEngine
{
	std::string LogLevelToString(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Debug:
			return "[DEBUG]";
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

	std::string CurrentDateTimeText()
	{
		std::time_t now = std::time(nullptr);
		std::tm local = {};
		localtime_s(&local, &now);

		char text[32];
		std::snprintf(text, sizeof(text), "%04d-%02d-%02d %02d:%02d:%02d",
			local.tm_year + 1900, local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);
		return text;
	}

	std::string CurrentTimeText()
	{
		auto now = std::chrono::system_clock::now();
		std::time_t seconds = std::chrono::system_clock::to_time_t(now);
		int milliseconds = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000);

		std::tm local = {};
		localtime_s(&local, &seconds);

		char text[32];
		std::snprintf(text, sizeof(text), "%02d:%02d:%02d.%03d", local.tm_hour, local.tm_min, local.tm_sec, milliseconds);
		return text;
	}

	std::string FormatLogLine(const LogEntry& entry)
	{
		return entry.time + " #" + std::to_string(entry.frame) + " " + LogLevelToString(entry.level) + " " + entry.message;
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

	void Logger::SetMinLevel(LogLevel level)
	{
		minLevel = level;
	}
	LogLevel Logger::GetMinLevel() const
	{
		return minLevel;
	}
	bool Logger::IsEnabled(LogLevel level) const
	{
		return level >= minLevel;
	}

	void Logger::Log(LogLevel level, const std::string& message)
	{
		if (!IsEnabled(level))
		{
			return;
		}

		LogEntry entry = {level, message, CurrentTimeText(), FrameClock::Instance()->GetFrame()};

		std::vector<std::shared_ptr<LogSink>> targets;
		{
			std::lock_guard<std::mutex> lock(logMutex);
			targets = sinks;
		}

		for (auto& sink : targets)
		{
			sink->Log(entry);
		}
	}

	void Logger::Debug(const std::string& message)
	{
		Log(LogLevel::Debug, message);
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
