#include "pch.h"
#include "ConsoleSink.h"
#include <iostream>

namespace XYZEngine
{
	void ConsoleSink::Log(LogLevel level, const std::string& message)
	{
		std::cout << LogLevelToString(level) << " " << message << std::endl;
	}
}
