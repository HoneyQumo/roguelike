#pragma once

#include <fstream>
#include "Logger.h"

namespace XYZEngine
{
	class FileSink : public LogSink
	{
	public:
		FileSink(const std::string& filePath);
		~FileSink();

		void Log(const LogEntry& entry) override;
	private:
		std::ofstream logFile;
	};
}
