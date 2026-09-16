#pragma once

#include "Vector.h"

namespace XYZEngine
{
	struct Push
	{
		Vector2Df first = {0.f, 0.f};
		Vector2Df second = {0.f, 0.f};
	};

	/**
	*	Кто въехал, тот и отъезжает. Долю считаем по шагу, который тело сделало за кадр:
	*	едет один - он уходит на всю глубину, едут оба - поровну, не едет никто - тоже
	*	поровну, иначе тела, появившиеся внахлёст, остались бы слипшимися.
	*/
	inline Push SplitPush(const Vector2Df& overlap, const Vector2Df& firstStep, const Vector2Df& secondStep,
		bool isSecondMovable)
	{
		if (!isSecondMovable)
		{
			return {overlap, {0.f, 0.f}};
		}

		float depth = overlap.GetLength();
		if (depth <= 0.f)
		{
			return {};
		}

		Vector2Df away = (1.f / depth) * overlap;

		float firstApproach = -(firstStep.x * away.x + firstStep.y * away.y);
		float secondApproach = secondStep.x * away.x + secondStep.y * away.y;

		firstApproach = firstApproach > 0.f ? firstApproach : 0.f;
		secondApproach = secondApproach > 0.f ? secondApproach : 0.f;

		float total = firstApproach + secondApproach;
		float share = total > 0.f ? firstApproach / total : 0.5f;

		return {share * overlap, (share - 1.f) * overlap};
	}
}
