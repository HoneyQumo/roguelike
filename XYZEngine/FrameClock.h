#pragma once

namespace XYZEngine
{
	class FrameClock
	{
	public:
		static FrameClock* Instance();

		void Advance(float newDeltaTime);
		void Reset();

		unsigned int GetFrame() const;
		float GetDeltaTime() const;
		float GetUnscaledDeltaTime() const;
		float GetElapsedSeconds() const;

		void SetTimeScale(float newTimeScale);
		float GetTimeScale() const;

		void HitStop(float duration);
		void SlowMotion(float scale, float duration, float blendBackTime);
		void StopTimeEffects();

	private:
		unsigned int frame = 0;
		float deltaTime = 0.f;
		float unscaledDeltaTime = 0.f;
		double elapsedSeconds = 0.0;

		float timeScale = 1.f;
		float baseTimeScale = 1.f;
		float hitStopLeft = 0.f;
		float slowMotionScale = 1.f;
		float slowMotionLeft = 0.f;
		float blendBackLeft = 0.f;
		float blendBackTime = 0.f;
		float blendFromScale = 1.f;

		float UpdateTimeEffects(float realDeltaTime);

		FrameClock() {}
		~FrameClock() {}

		FrameClock(FrameClock const&) = delete;
		FrameClock& operator=(FrameClock const&) = delete;
	};
}
