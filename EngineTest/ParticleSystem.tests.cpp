#include "pch.h"
#include "GameWorld.h"
#include "ParticleSystem.h"
#include "ParticleSystemComponent.h"
#include "ParticleEmitterComponent.h"

using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::ParticleEmission;
using XYZEngine::ParticleEmitterComponent;
using XYZEngine::ParticleSpec;
using XYZEngine::ParticleSystem;
using XYZEngine::ParticleSystemComponent;

namespace
{
	class ParticleSystemTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* owner = nullptr;

		ParticleSystemComponent* CreateSystem(std::size_t capacity)
		{
			owner = GameWorld::Instance()->CreateGameObject("Particles");
			auto component = owner->AddComponent<ParticleSystemComponent>();
			component->SetCapacity(capacity);

			return component;
		}

		ParticleSpec MakeSpec(int count, float lifeTime) const
		{
			ParticleSpec spec;
			spec.count = count;
			spec.lifeTime = lifeTime;
			spec.lifeTimeSpread = 0.f;
			spec.startSpeed = 100.f;
			spec.endSpeed = 100.f;
			spec.spreadDegrees = 0.f;
			spec.startSize = 4.f;
			spec.endSize = 1.f;

			return spec;
		}
	};
}

TEST_F(ParticleSystemTest, BurstFillsExactlyCount)
{
	ParticleSystemComponent* particles = CreateSystem(64);
	ParticleSpec spec = MakeSpec(10, 1.f);

	particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});

	EXPECT_EQ(particles->GetActiveCount(), 10u);
}

TEST_F(ParticleSystemTest, PoolNeverExceedsCapacity)
{
	ParticleSystemComponent* particles = CreateSystem(16);
	ParticleSpec spec = MakeSpec(10, 1.f);

	for (int burst = 0; burst < 5; burst++)
	{
		particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});
	}

	EXPECT_EQ(particles->GetActiveCount(), 16u);
	EXPECT_EQ(particles->GetCapacity(), 16u);
}

TEST_F(ParticleSystemTest, OldestParticlesAreDropped)
{
	ParticleSystemComponent* particles = CreateSystem(8);
	ParticleSpec spec = MakeSpec(8, 1.f);
	spec.startSpeed = 0.f;
	spec.endSpeed = 0.f;

	particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});
	particles->Emit(spec, {100.f, 0.f}, {1.f, 0.f});
	particles->Update(0.f);

	EXPECT_EQ(particles->GetActiveCount(), 8u);
}

TEST_F(ParticleSystemTest, ParticlesDieAfterLifeTime)
{
	ParticleSystemComponent* particles = CreateSystem(32);
	ParticleSpec spec = MakeSpec(5, 0.5f);

	particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});
	particles->Update(0.2f);
	EXPECT_EQ(particles->GetActiveCount(), 5u);

	particles->Update(0.4f);

	EXPECT_EQ(particles->GetActiveCount(), 0u);
}

TEST_F(ParticleSystemTest, ZeroLifeTimeSpecIsIgnored)
{
	ParticleSystemComponent* particles = CreateSystem(32);
	ParticleSpec spec = MakeSpec(5, 0.f);

	particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});

	EXPECT_EQ(particles->GetActiveCount(), 0u);
}

TEST_F(ParticleSystemTest, ZeroDeltaKeepsParticlesAlive)
{
	ParticleSystemComponent* particles = CreateSystem(32);
	ParticleSpec spec = MakeSpec(4, 1.f);

	particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});
	particles->Update(0.f);

	EXPECT_EQ(particles->GetActiveCount(), 4u);
}

TEST_F(ParticleSystemTest, PoolDoesNotReallocateWhileRunning)
{
	ParticleSystemComponent* particles = CreateSystem(128);
	ParticleSpec spec = MakeSpec(8, 0.3f);
	std::size_t capacity = particles->GetCapacity();

	for (int frame = 0; frame < 200; frame++)
	{
		particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});
		particles->Update(0.016f);
	}

	EXPECT_EQ(particles->GetCapacity(), capacity);
	EXPECT_LE(particles->GetActiveCount(), capacity);
}

TEST_F(ParticleSystemTest, ComponentRegistersAndUnregisters)
{
	ParticleSystemComponent* particles = CreateSystem(16);

	EXPECT_EQ(ParticleSystem::Instance()->GetActive(), particles);

	GameWorld::Instance()->Clear();

	EXPECT_EQ(ParticleSystem::Instance()->GetActive(), nullptr);
}

TEST_F(ParticleSystemTest, EmitWithoutSystemIsSafe)
{
	GameWorld::Instance()->Clear();
	ParticleSpec spec = MakeSpec(5, 1.f);

	ParticleSystem::Instance()->Emit(spec, {0.f, 0.f}, {1.f, 0.f});

	EXPECT_EQ(ParticleSystem::Instance()->GetActive(), nullptr);
}

TEST_F(ParticleSystemTest, ContinuousEmitterFollowsItsRate)
{
	ParticleSystemComponent* particles = CreateSystem(256);

	static ParticleSpec spec = MakeSpec(1, 5.f);
	spec.emission = ParticleEmission::Continuous;
	spec.ratePerSecond = 10.f;
	spec.duration = 0.f;

	GameObject* torch = GameWorld::Instance()->CreateGameObject("Torch");
	auto emitter = torch->AddComponent<ParticleEmitterComponent>();
	emitter->SetSpec(&spec);

	for (int frame = 0; frame < 10; frame++)
	{
		torch->Update(0.1f);
	}

	EXPECT_GE(particles->GetActiveCount(), 9u);
	EXPECT_LE(particles->GetActiveCount(), 11u);
}

TEST_F(ParticleSystemTest, BurstEmitterFiresOnceOnStart)
{
	ParticleSystemComponent* particles = CreateSystem(64);

	static ParticleSpec spec = MakeSpec(6, 1.f);
	spec.emission = ParticleEmission::Burst;

	GameObject* spark = GameWorld::Instance()->CreateGameObject("Spark");
	spark->AddComponent<ParticleEmitterComponent>()->SetSpec(&spec);

	spark->Update(0.016f);
	spark->Update(0.016f);

	EXPECT_EQ(particles->GetActiveCount(), 6u);
}

TEST_F(ParticleSystemTest, EveryParticleBecomesOneTexturedQuad)
{
	ParticleSystemComponent* particles = CreateSystem(64);
	ParticleSpec spec = MakeSpec(3, 1.f);
	spec.frame = {0, 176, 32, 24};
	spec.startColor = {10, 200, 30, 255};

	particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});
	particles->Update(0.f);

	const sf::VertexArray& quads = particles->GetAlphaVertices();
	ASSERT_EQ(quads.getVertexCount(), 12u);
	EXPECT_EQ(quads.getPrimitiveType(), sf::Quads);

	EXPECT_EQ(quads[0].color.g, 200);
	EXPECT_EQ(quads[0].color.r, 10);

	EXPECT_FLOAT_EQ(quads[0].texCoords.x, 0.f);
	EXPECT_FLOAT_EQ(quads[0].texCoords.y, 200.f);
	EXPECT_FLOAT_EQ(quads[1].texCoords.x, 32.f);
	EXPECT_FLOAT_EQ(quads[2].texCoords.y, 176.f);
}

TEST_F(ParticleSystemTest, AdditiveParticlesGoToTheirOwnBatch)
{
	ParticleSystemComponent* particles = CreateSystem(64);
	ParticleSpec alphaSpec = MakeSpec(2, 1.f);
	ParticleSpec additiveSpec = MakeSpec(3, 1.f);
	additiveSpec.isAdditive = true;

	particles->Emit(alphaSpec, {0.f, 0.f}, {1.f, 0.f});
	particles->Emit(additiveSpec, {0.f, 0.f}, {1.f, 0.f});
	particles->Update(0.f);

	EXPECT_EQ(particles->GetAlphaVertices().getVertexCount(), 8u);
	EXPECT_EQ(particles->GetAdditiveVertices().getVertexCount(), 12u);
}

TEST_F(ParticleSystemTest, DeadParticlesLeaveNoGeometry)
{
	ParticleSystemComponent* particles = CreateSystem(64);
	ParticleSpec spec = MakeSpec(5, 0.2f);

	particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});
	particles->Update(0.3f);

	EXPECT_EQ(particles->GetAlphaVertices().getVertexCount(), 0u);
	EXPECT_EQ(particles->GetAdditiveVertices().getVertexCount(), 0u);
}

TEST_F(ParticleSystemTest, ParticleFliesAlongItsDirection)
{
	ParticleSystemComponent* particles = CreateSystem(8);
	ParticleSpec spec = MakeSpec(1, 1.f);
	spec.startSpeed = 100.f;
	spec.endSpeed = 100.f;
	spec.startSize = 2.f;
	spec.endSize = 2.f;
	spec.frame = {0, 0, 10, 10};

	particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});
	particles->Update(0.1f);

	const sf::VertexArray& quads = particles->GetAlphaVertices();
	ASSERT_EQ(quads.getVertexCount(), 4u);

	float centerX = 0.5f * (quads[0].position.x + quads[1].position.x);
	EXPECT_NEAR(centerX, 10.f, 0.01f);
	EXPECT_NEAR(0.5f * (quads[0].position.y + quads[2].position.y), 0.f, 0.01f);
}
