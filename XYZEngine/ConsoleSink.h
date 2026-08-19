#pragma once

#include "Logger.h"

namespace XYZEngine
{
	class ConsoleSink : public LogSink
	{
	public:
		void Log(LogLevel level, const std::string& message) override;
	};
}
