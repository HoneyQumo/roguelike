#pragma once

#include "Logger.h"

namespace XYZEngine
{
	class ConsoleSink : public LogSink
	{
	public:
		void Log(const LogEntry& entry) override;
	};
}
