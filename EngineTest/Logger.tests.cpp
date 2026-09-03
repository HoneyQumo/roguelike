#include "pch.h"
#include "Logger.h"
#include "LoggerRegistry.h"
#include "FileSink.h"
#include "ConsoleSink.h"
#include <chrono>
#include <cstdio>

using namespace XYZEngine;

namespace
{
	class CaptureSink : public LogSink
	{
	public:
		std::vector<std::pair<LogLevel, std::string>> entries;

		void Log(LogLevel level, const std::string& message) override
		{
			entries.emplace_back(level, message);
		}
	};

	const char* BENCHMARK_LOG_PATH = "logger_benchmark.txt";

	double MeasureLogInfo(std::shared_ptr<Logger> logger, int iterations)
	{
		LoggerRegistry::Instance()->RegisterLogger("global", logger);

		auto started = std::chrono::steady_clock::now();
		for (int i = 0; i < iterations; i++)
		{
			LOG_INFO("Player takes " + std::to_string(i) + " damage");
		}
		auto elapsed = std::chrono::steady_clock::now() - started;

		return std::chrono::duration<double, std::micro>(elapsed).count() / iterations;
	}
}

TEST(LoggerTests, MessagesReachEverySink)
{
	auto logger = std::make_shared<Logger>();
	auto first = std::make_shared<CaptureSink>();
	auto second = std::make_shared<CaptureSink>();
	logger->AddSink(first);
	logger->AddSink(second);

	logger->Info("hello");
	logger->Error("oops");

	ASSERT_EQ(first->entries.size(), 2u);
	ASSERT_EQ(second->entries.size(), 2u);
	EXPECT_EQ(first->entries[0].first, LogLevel::Info);
	EXPECT_EQ(first->entries[0].second, "hello");
	EXPECT_EQ(second->entries[1].first, LogLevel::Error);
	EXPECT_EQ(second->entries[1].second, "oops");
}

TEST(LoggerTests, MacroUsesRegisteredGlobalLogger)
{
	auto logger = std::make_shared<Logger>();
	auto sink = std::make_shared<CaptureSink>();
	logger->AddSink(sink);
	LoggerRegistry::Instance()->RegisterLogger("global", logger);

	LOG_WARN("via macro");

	ASSERT_EQ(sink->entries.size(), 1u);
	EXPECT_EQ(sink->entries[0].first, LogLevel::Warning);
	EXPECT_EQ(sink->entries[0].second, "via macro");
}

TEST(LoggerTests, MinLevelDropsMessagesBelowIt)
{
	auto logger = std::make_shared<Logger>();
	auto sink = std::make_shared<CaptureSink>();
	logger->AddSink(sink);
	logger->SetMinLevel(LogLevel::Warning);

	logger->Info("dropped");
	logger->Warning("kept");
	logger->Error("kept too");

	ASSERT_EQ(sink->entries.size(), 2u);
	EXPECT_EQ(sink->entries[0].first, LogLevel::Warning);
	EXPECT_EQ(sink->entries[1].first, LogLevel::Error);
}

TEST(LoggerTests, DefaultMinLevelIsInfo)
{
	Logger logger;

	EXPECT_EQ(logger.GetMinLevel(), LogLevel::Info);
	EXPECT_TRUE(logger.IsEnabled(LogLevel::Info));
	EXPECT_TRUE(logger.IsEnabled(LogLevel::Error));
}

TEST(LoggerTests, MacroDoesNotBuildMessageWhenLevelIsDisabled)
{
	auto logger = std::make_shared<Logger>();
	logger->AddSink(std::make_shared<CaptureSink>());
	logger->SetMinLevel(LogLevel::Error);
	LoggerRegistry::Instance()->RegisterLogger("global", logger);

	int evaluations = 0;
	auto buildMessage = [&evaluations]() { evaluations++; return std::string("expensive"); };

	LOG_INFO(buildMessage());
	LOG_WARN(buildMessage());
	EXPECT_EQ(evaluations, 0);

	LOG_ERROR(buildMessage());
	EXPECT_EQ(evaluations, 1);

	LoggerRegistry::Instance()->RegisterLogger("global", std::make_shared<Logger>());
}

TEST(LoggerTests, BenchmarkDisabledLevel)
{
	auto logger = std::make_shared<Logger>();
	logger->AddSink(std::make_shared<CaptureSink>());
	logger->SetMinLevel(LogLevel::Warning);

	double microseconds = MeasureLogInfo(logger, 5000);
	std::cout << "LOG_INFO below min level: " << microseconds << " us per call" << std::endl;

	LoggerRegistry::Instance()->RegisterLogger("global", std::make_shared<Logger>());
}

TEST(LoggerTests, BenchmarkFileSink)
{
	std::remove(BENCHMARK_LOG_PATH);
	{
		auto logger = std::make_shared<Logger>();
		logger->AddSink(std::make_shared<FileSink>(BENCHMARK_LOG_PATH));

		double microseconds = MeasureLogInfo(logger, 5000);
		std::cout << "LOG_INFO with FileSink: " << microseconds << " us per call" << std::endl;
	}
	std::remove(BENCHMARK_LOG_PATH);

	LoggerRegistry::Instance()->RegisterLogger("global", std::make_shared<Logger>());
}

TEST(LoggerTests, BenchmarkCaptureSinkOnly)
{
	auto logger = std::make_shared<Logger>();
	logger->AddSink(std::make_shared<CaptureSink>());

	double microseconds = MeasureLogInfo(logger, 5000);
	std::cout << "LOG_INFO with in-memory sink: " << microseconds << " us per call" << std::endl;

	LoggerRegistry::Instance()->RegisterLogger("global", std::make_shared<Logger>());
}
