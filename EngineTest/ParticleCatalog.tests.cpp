#include "pch.h"
#include "ParticleCatalog.h"

using namespace RoguelikeGame;

namespace
{
	constexpr int FX_ATLAS_WIDTH = 640;
	constexpr int FX_ATLAS_HEIGHT = 296;
}

TEST(ParticleCatalogTests, EveryEffectIsFoundAndUnique)
{
	for (const ParticleEffectDefinition& definition : PARTICLE_EFFECTS)
	{
		EXPECT_EQ(FindParticleSpec(definition.effect), &definition.spec) << definition.name;

		int sameEffect = 0;
		for (const ParticleEffectDefinition& other : PARTICLE_EFFECTS)
		{
			if (other.effect == definition.effect)
			{
				sameEffect++;
			}
		}

		EXPECT_EQ(sameEffect, 1) << definition.name;
	}
}

TEST(ParticleCatalogTests, SpecsAreSane)
{
	for (const ParticleEffectDefinition& definition : PARTICLE_EFFECTS)
	{
		const XYZEngine::ParticleSpec& spec = definition.spec;

		EXPECT_GT(spec.count, 0) << definition.name;
		EXPECT_GT(spec.lifeTime, 0.f) << definition.name;
		EXPECT_GE(spec.lifeTimeSpread, 0.f) << definition.name;
		EXPECT_LT(spec.lifeTimeSpread, 1.f) << definition.name;
		EXPECT_GE(spec.startSize, 0.f) << definition.name;
		EXPECT_GE(spec.endSize, 0.f) << definition.name;
		EXPECT_GE(spec.spreadDegrees, 0.f) << definition.name;
		EXPECT_LE(spec.spreadDegrees, 180.f) << definition.name;

		EXPECT_GE(spec.frame.x, 0) << definition.name;
		EXPECT_GE(spec.frame.y, 0) << definition.name;
		EXPECT_LE(spec.frame.x + spec.frame.width, FX_ATLAS_WIDTH) << definition.name;
		EXPECT_LE(spec.frame.y + spec.frame.height, FX_ATLAS_HEIGHT) << definition.name;
	}
}

TEST(ParticleCatalogTests, HealBurstIsGreenAndFadesOut)
{
	const XYZEngine::ParticleSpec* spec = FindParticleSpec(ParticleEffect::HealBurst);
	ASSERT_NE(spec, nullptr);

	EXPECT_GT(spec->startColor.g, spec->startColor.r);
	EXPECT_GT(spec->startColor.g, spec->startColor.b);
	EXPECT_EQ(spec->endColor.a, 0);
	EXPECT_GT(spec->gravity.y, 0.f);
}

TEST(ParticleCatalogTests, HitBurstIsRedAndFadesOut)
{
	const XYZEngine::ParticleSpec* spec = FindParticleSpec(ParticleEffect::HitBurst);
	ASSERT_NE(spec, nullptr);

	EXPECT_GT(spec->startColor.r, spec->startColor.g);
	EXPECT_GT(spec->startColor.r, spec->startColor.b);
	EXPECT_EQ(spec->endColor.a, 0);
	EXPECT_LT(spec->gravity.y, 0.f);
}

TEST(ParticleCatalogTests, FxFrameWalksTheStripByX)
{
	XYZEngine::ParticleFrame first = FxFrame(FX_IMPACT, 0);
	XYZEngine::ParticleFrame third = FxFrame(FX_IMPACT, 2);

	EXPECT_EQ(first.x, FX_IMPACT.x);
	EXPECT_EQ(third.x, FX_IMPACT.x + FX_IMPACT.width * 2);
	EXPECT_EQ(third.y, FX_IMPACT.y);
	EXPECT_EQ(third.width, FX_IMPACT.width);
}
