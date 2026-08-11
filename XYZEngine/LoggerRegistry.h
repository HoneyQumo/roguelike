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

#define LOG_INFO(message) XYZEngine::LoggerRegistry::Instance()->GetLogger("global")->Info(message)
#define LOG_WARN(message) XYZEngine::LoggerRegistry::Instance()->GetLogger("global")->Warning(message)
#define LOG_ERROR(message) XYZEngine::LoggerRegistry::Instance()->GetLogger("global")->Error(message)
