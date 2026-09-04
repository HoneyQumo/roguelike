#include "pch.h"
#include "FrameClock.h"

namespace XYZEngine
{
	FrameClock* FrameClock::Instance()
	{
		static FrameClock clock;
		return &clock;
	}

	void FrameClock::Advance(float newDeltaTime)
	{
		frame++;
		deltaTime = newDeltaTime;
		elapsedSeconds += newDeltaTime;
	}

	void FrameClock::Reset()
	{
		frame = 0;
		deltaTime = 0.f;
		elapsedSeconds = 0.0;
	}

	unsigned int FrameClock::GetFrame() const
	{
		return frame;
	}
	float FrameClock::GetDeltaTime() const
	{
		return deltaTime;
	}
	float FrameClock::GetElapsedSeconds() const
	{
		return static_cast<float>(elapsedSeconds);
	}
}
