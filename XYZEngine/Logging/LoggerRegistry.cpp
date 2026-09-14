#include "pch.h"
#include "LoggerRegistry.h"
#include "ConsoleSink.h"

namespace XYZEngine
{
	LoggerRegistry* LoggerRegistry::Instance()
	{
		static LoggerRegistry registry;
		return &registry;
	}

	std::shared_ptr<Logger> LoggerRegistry::GetLogger(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(registryMutex);

		auto loggerPair = loggers.find(name);
		if (loggerPair != loggers.end())
		{
			return loggerPair->second;
		}

		if (defaultLogger == nullptr)
		{
			defaultLogger = CreateFallbackLogger();
		}

		return defaultLogger;
	}
	void LoggerRegistry::RegisterLogger(const std::string& name, std::shared_ptr<Logger> logger)
	{
		if (logger == nullptr)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(registryMutex);
		loggers[name] = logger;
	}

	void LoggerRegistry::SetDefaultLogger(std::shared_ptr<Logger> logger)
	{
		if (logger == nullptr)
		{
			return;
		}

		std::lock_guard<std::mutex> lock(registryMutex);
		defaultLogger = logger;
	}
	std::shared_ptr<Logger> LoggerRegistry::GetDefaultLogger()
	{
		std::lock_guard<std::mutex> lock(registryMutex);

		if (defaultLogger == nullptr)
		{
			defaultLogger = CreateFallbackLogger();
		}

		return defaultLogger;
	}

	std::shared_ptr<Logger> LoggerRegistry::CreateFallbackLogger()
	{
		auto logger = std::make_shared<Logger>();
		logger->AddSink(std::make_shared<ConsoleSink>());
		return logger;
	}
}
