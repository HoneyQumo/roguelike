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
		float GetElapsedSeconds() const;

	private:
		unsigned int frame = 0;
		float deltaTime = 0.f;
		double elapsedSeconds = 0.0;

		FrameClock() {}
		~FrameClock() {}

		FrameClock(FrameClock const&) = delete;
		FrameClock& operator=(FrameClock const&) = delete;
	};
}
