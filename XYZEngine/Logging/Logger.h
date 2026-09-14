#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace XYZEngine
{
	enum class LogLevel
	{
		Debug,
		Info,
		Warning,
		Error
	};

	struct LogEntry
	{
		LogLevel level;
		std::string message;
		std::string time;
		unsigned int frame;
	};

	std::string LogLevelToString(LogLevel level);
	std::string CurrentDateTimeText();
	std::string CurrentTimeText();
	std::string FormatLogLine(const LogEntry& entry);

	class LogSink
	{
	public:
		virtual ~LogSink() = default;

		virtual void Log(const LogEntry& entry) = 0;
	};

	class Logger
	{
	public:
		void AddSink(std::shared_ptr<LogSink> sink);

		void SetMinLevel(LogLevel level);
		LogLevel GetMinLevel() const;
		bool IsEnabled(LogLevel level) const;

		void Log(LogLevel level, const std::string& message);

		void Debug(const std::string& message);
		void Info(const std::string& message);
		void Warning(const std::string& message);
		void Error(const std::string& message);
	private:
		std::vector<std::shared_ptr<LogSink>> sinks;
		std::mutex logMutex;
		LogLevel minLevel = LogLevel::Info;
	};
}
