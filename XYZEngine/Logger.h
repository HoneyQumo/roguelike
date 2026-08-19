#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace XYZEngine
{
	enum class LogLevel
	{
		Info,
		Warning,
		Error
	};

	std::string LogLevelToString(LogLevel level);

	class LogSink
	{
	public:
		virtual ~LogSink() = default;

		virtual void Log(LogLevel level, const std::string& message) = 0;
	};

	class Logger
	{
	public:
		void AddSink(std::shared_ptr<LogSink> sink);

		void Log(LogLevel level, const std::string& message);

		void Info(const std::string& message);
		void Warning(const std::string& message);
		void Error(const std::string& message);
	private:
		std::vector<std::shared_ptr<LogSink>> sinks;
		std::mutex logMutex;
	};
}
