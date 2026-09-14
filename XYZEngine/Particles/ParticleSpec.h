#pragma once

#include "Vector.h"

namespace XYZEngine
{
	enum class ParticleEmission
	{
		Burst,
		Continuous
	};

	struct ParticleColor
	{
		unsigned char r = 255;
		unsigned char g = 255;
		unsigned char b = 255;
		unsigned char a = 255;
	};

	struct ParticleFrame
	{
		int x = 0;
		int y = 0;
		int width = 1;
		int height = 1;
	};

	struct ParticleSpec
	{
		int count = 1;
		float ratePerSecond = 0.f;
		float duration = 0.f;
		float lifeTime = 1.f;
		float lifeTimeSpread = 0.f;
		float startSpeed = 0.f;
		float endSpeed = 0.f;
		Vector2Df gravity = {0.f, 0.f};
		float startSize = 1.f;
		float endSize = 1.f;
		ParticleColor startColor;
		ParticleColor endColor;
		float spreadDegrees = 0.f;
		ParticleFrame frame;
		ParticleEmission emission = ParticleEmission::Burst;
		bool isAdditive = false;
	};
}
