#include "pch.h"
#include "ConsoleSink.h"
#include <iostream>

namespace XYZEngine
{
	void ConsoleSink::Log(const LogEntry& entry)
	{
		std::cout << FormatLogLine(entry) << std::endl;
	}
}
