#pragma once

#include <algorithm>

namespace XYZEngine
{
	class Cooldown
	{
	public:
		Cooldown() = default;
		explicit Cooldown(float newDuration) : duration(std::max(0.f, newDuration)) {}

		static Cooldown Started(float newDuration)
		{
			Cooldown result(newDuration);
			result.Restart();
			return result;
		}

		void SetDuration(float newDuration)
		{
			duration = std::max(0.f, newDuration);
		}
		float GetDuration() const
		{
			return duration;
		}

		void Restart()
		{
			left = duration;
		}
		void Start(float newDuration)
		{
			SetDuration(newDuration);
			Restart();
		}
		void Stop()
		{
			left = 0.f;
		}

		void Tick(float deltaTime)
		{
			if (left > 0.f)
			{
				left = std::max(0.f, left - deltaTime);
			}
		}

		bool IsRunning() const
		{
			return left > 0.f;
		}
		bool IsReady() const
		{
			return left <= 0.f;
		}

		float GetLeft() const
		{
			return left;
		}

		float GetProgress() const
		{
			if (duration <= 0.f)
			{
				return 1.f;
			}

			return std::clamp(1.f - left / duration, 0.f, 1.f);
		}

	private:
		float duration = 0.f;
		float left = 0.f;
	};
}
