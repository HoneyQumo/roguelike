#include "pch.h"
#include "FrameClock.h"
#include <algorithm>

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
		unscaledDeltaTime = newDeltaTime;

		timeScale = baseTimeScale * UpdateTimeEffects(newDeltaTime);

		deltaTime = newDeltaTime * timeScale;
		elapsedSeconds += deltaTime;
	}

	void FrameClock::Reset()
	{
		frame = 0;
		deltaTime = 0.f;
		unscaledDeltaTime = 0.f;
		elapsedSeconds = 0.0;

		StopTimeEffects();
	}

	void FrameClock::SetTimeScale(float newTimeScale)
	{
		baseTimeScale = std::max(newTimeScale, 0.f);
		timeScale = baseTimeScale;
	}
	float FrameClock::GetTimeScale() const
	{
		return timeScale;
	}

	void FrameClock::HitStop(float duration)
	{
		if (duration <= 0.f)
		{
			return;
		}

		hitStopLeft = std::max(hitStopLeft, duration);
	}

	void FrameClock::SlowMotion(float scale, float duration, float newBlendBackTime)
	{
		if (scale >= 1.f || duration <= 0.f)
		{
			return;
		}

		slowMotionScale = std::max(scale, 0.f);
		slowMotionLeft = duration;
		blendBackTime = std::max(newBlendBackTime, 0.f);
		blendBackLeft = 0.f;
	}

	void FrameClock::StopTimeEffects()
	{
		timeScale = baseTimeScale;
		hitStopLeft = 0.f;
		slowMotionScale = 1.f;
		slowMotionLeft = 0.f;
		blendBackLeft = 0.f;
		blendBackTime = 0.f;
		blendFromScale = 1.f;
	}

	float FrameClock::UpdateTimeEffects(float realDeltaTime)
	{
		if (hitStopLeft > 0.f)
		{
			hitStopLeft -= realDeltaTime;
			if (hitStopLeft > 0.f)
			{
				return 0.f;
			}

			hitStopLeft = 0.f;
		}

		if (slowMotionLeft > 0.f)
		{
			slowMotionLeft -= realDeltaTime;

			if (slowMotionLeft > 0.f)
			{
				return slowMotionScale;
			}

			slowMotionLeft = 0.f;
			blendFromScale = slowMotionScale;
			blendBackLeft = blendBackTime;
		}

		if (blendBackLeft > 0.f)
		{
			blendBackLeft -= realDeltaTime;

			if (blendBackLeft <= 0.f || blendBackTime <= 0.f)
			{
				blendBackLeft = 0.f;
				return 1.f;
			}

			float progress = 1.f - blendBackLeft / blendBackTime;
			return blendFromScale + (1.f - blendFromScale) * progress;
		}

		return 1.f;
	}

	unsigned int FrameClock::GetFrame() const
	{
		return frame;
	}
	float FrameClock::GetDeltaTime() const
	{
		return deltaTime;
	}
	float FrameClock::GetUnscaledDeltaTime() const
	{
		return unscaledDeltaTime;
	}
	float FrameClock::GetElapsedSeconds() const
	{
		return static_cast<float>(elapsedSeconds);
	}
}
