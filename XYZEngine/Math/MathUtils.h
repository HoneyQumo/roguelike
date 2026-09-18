#pragma once

#include <cmath>
#include "Vector.h"

namespace XYZEngine
{
	constexpr float PI = 3.14159265358979323846f;
	constexpr float TWO_PI = 2.f * PI;

	constexpr float ToRadians(float degrees)
	{
		return degrees * (PI / 180.f);
	}

	constexpr float ToDegrees(float radians)
	{
		return radians * (180.f / PI);
	}

	inline Vector2Df DirectionFromDegrees(float degrees)
	{
		float radians = ToRadians(degrees);
		return { std::cos(radians), std::sin(radians) };
	}

	inline float DegreesFromDirection(const Vector2Df& direction)
	{
		return ToDegrees(std::atan2(direction.y, direction.x));
	}

	// Кратчайший угол между направлениями: -180..180, без накрутки оборотов.
	inline float ShortestAngleDegrees(float fromDegrees, float toDegrees)
	{
		float difference = std::fmod(toDegrees - fromDegrees, 360.f);

		if (difference > 180.f)
		{
			difference -= 360.f;
		}
		else if (difference < -180.f)
		{
			difference += 360.f;
		}

		return difference;
	}

	// Шаг ноль или меньше - без ограничения, объект встаёт на угол сразу.
	inline float TurnTowardsDegrees(float fromDegrees, float toDegrees, float maxStepDegrees)
	{
		float difference = ShortestAngleDegrees(fromDegrees, toDegrees);

		if (maxStepDegrees <= 0.f || std::fabs(difference) <= maxStepDegrees)
		{
			return toDegrees;
		}

		return fromDegrees + (difference > 0.f ? maxStepDegrees : -maxStepDegrees);
	}

	inline Vector2Df RotateByDegrees(const Vector2Df& direction, float degrees)
	{
		if (degrees == 0.f)
		{
			return direction;
		}

		float radians = ToRadians(degrees);
		float sinValue = std::sin(radians);
		float cosValue = std::cos(radians);

		return { direction.x * cosValue - direction.y * sinValue, direction.x * sinValue + direction.y * cosValue };
	}
}
