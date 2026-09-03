#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "Logger.h"

namespace XYZEngine
{
	class LoggerRegistry
	{
	public:
		static LoggerRegistry* Instance();

		std::shared_ptr<Logger> GetLogger(const std::string& name);
		void RegisterLogger(const std::string& name, std::shared_ptr<Logger> logger);

		void SetDefaultLogger(std::shared_ptr<Logger> logger);
		std::shared_ptr<Logger> GetDefaultLogger();
	private:
		std::unordered_map<std::string, std::shared_ptr<Logger>> loggers;
		std::shared_ptr<Logger> defaultLogger;
		std::mutex registryMutex;

		LoggerRegistry() {}
		~LoggerRegistry() {}

		LoggerRegistry(LoggerRegistry const&) = delete;
		LoggerRegistry& operator= (LoggerRegistry const&) = delete;

		std::shared_ptr<Logger> CreateFallbackLogger();
	};
}

#define XYZ_LOG_AT(level, message) \
	do \
	{ \
		auto xyzLogger = XYZEngine::LoggerRegistry::Instance()->GetLogger("global"); \
		if (xyzLogger->IsEnabled(level)) \
		{ \
			xyzLogger->Log(level, message); \
		} \
	} while (false)

#define LOG_INFO(message) XYZ_LOG_AT(XYZEngine::LogLevel::Info, message)
#define LOG_WARN(message) XYZ_LOG_AT(XYZEngine::LogLevel::Warning, message)
#define LOG_ERROR(message) XYZ_LOG_AT(XYZEngine::LogLevel::Error, message)
