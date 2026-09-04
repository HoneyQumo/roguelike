#include "pch.h"
#include "Logger.h"
#include "LoggerRegistry.h"
#include "FileSink.h"
#include "ConsoleSink.h"
#include "FrameClock.h"
#include <chrono>
#include <cstdio>
#include <regex>

using namespace XYZEngine;

namespace
{
	class CaptureSink : public LogSink
	{
	public:
		std::vector<LogEntry> entries;

		void Log(const LogEntry& entry) override
		{
			entries.push_back(entry);
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
	EXPECT_EQ(first->entries[0].level, LogLevel::Info);
	EXPECT_EQ(first->entries[0].message, "hello");
	EXPECT_EQ(second->entries[1].level, LogLevel::Error);
	EXPECT_EQ(second->entries[1].message, "oops");
}

TEST(LoggerTests, MacroUsesRegisteredGlobalLogger)
{
	auto logger = std::make_shared<Logger>();
	auto sink = std::make_shared<CaptureSink>();
	logger->AddSink(sink);
	LoggerRegistry::Instance()->RegisterLogger("global", logger);

	LOG_WARN("via macro");

	ASSERT_EQ(sink->entries.size(), 1u);
	EXPECT_EQ(sink->entries[0].level, LogLevel::Warning);
	EXPECT_EQ(sink->entries[0].message, "via macro");
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
	EXPECT_EQ(sink->entries[0].level, LogLevel::Warning);
	EXPECT_EQ(sink->entries[1].level, LogLevel::Error);
}

TEST(LoggerTests, DefaultMinLevelIsInfo)
{
	Logger logger;

	EXPECT_EQ(logger.GetMinLevel(), LogLevel::Info);
	EXPECT_TRUE(logger.IsEnabled(LogLevel::Info));
	EXPECT_TRUE(logger.IsEnabled(LogLevel::Error));
}

TEST(LoggerTests, DebugLevelIsOffByDefaultAndCanBeEnabled)
{
	Logger logger;

	EXPECT_FALSE(logger.IsEnabled(LogLevel::Debug));

	logger.SetMinLevel(LogLevel::Debug);

	EXPECT_TRUE(logger.IsEnabled(LogLevel::Debug));
	EXPECT_EQ(LogLevelToString(LogLevel::Debug), "[DEBUG]");
}

TEST(LoggerTests, LogLineCarriesTimeFrameAndLevel)
{
	LogEntry entry = {LogLevel::Warning, "hello", "12:34:56.789", 42u};

	EXPECT_EQ(FormatLogLine(entry), "12:34:56.789 #42 [WARNING] hello");
}

TEST(LoggerTests, EverySinkGetsTheSameStamp)
{
	FrameClock::Instance()->Reset();
	FrameClock::Instance()->Advance(0.016f);

	auto logger = std::make_shared<Logger>();
	auto first = std::make_shared<CaptureSink>();
	auto second = std::make_shared<CaptureSink>();
	logger->AddSink(first);
	logger->AddSink(second);

	logger->Info("stamped");

	ASSERT_EQ(first->entries.size(), 1u);
	ASSERT_EQ(second->entries.size(), 1u);
	EXPECT_EQ(first->entries[0].frame, 1u);
	EXPECT_EQ(first->entries[0].time, second->entries[0].time);
	EXPECT_TRUE(std::regex_match(first->entries[0].time, std::regex(R"(\d{2}:\d{2}:\d{2}\.\d{3})"))) << first->entries[0].time;
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
	LoggerRegistry::Instance()->RegisterLogger("global", std::make_shared<Logger>());
	std::remove(BENCHMARK_LOG_PATH);
}

TEST(LoggerTests, BenchmarkCaptureSinkOnly)
{
	auto logger = std::make_shared<Logger>();
	logger->AddSink(std::make_shared<CaptureSink>());

	double microseconds = MeasureLogInfo(logger, 5000);
	std::cout << "LOG_INFO with in-memory sink: " << microseconds << " us per call" << std::endl;

	LoggerRegistry::Instance()->RegisterLogger("global", std::make_shared<Logger>());
}
