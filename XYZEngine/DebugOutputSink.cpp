#include "pch.h"
#include "DebugOutputSink.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace XYZEngine
{
	void DebugOutputSink::Log(const LogEntry& entry)
	{
		OutputDebugStringA((FormatLogLine(entry) + "\n").c_str());
	}
}
