#pragma once

#include "Logger.h"

namespace XYZEngine
{
	class DebugOutputSink : public LogSink
	{
	public:
		void Log(const LogEntry& entry) override;
	};
}
