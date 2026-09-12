#include "pch.h"
#include "DebugOutputSink.h"
#include <windows.h>

namespace XYZEngine
{
	void DebugOutputSink::Log(const LogEntry& entry)
	{
		OutputDebugStringA((FormatLogLine(entry) + "\n").c_str());
	}
}
