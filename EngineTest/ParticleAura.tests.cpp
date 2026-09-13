#include "pch.h"
#include "GameWorld.h"
#include "ParticleAuraComponent.h"
#include "ParticleSystemComponent.h"
#include "SpriteRendererComponent.h"

using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::ParticleAuraComponent;
using XYZEngine::ParticleEmission;
using XYZEngine::ParticleSpec;
using XYZEngine::ParticleSystemComponent;

namespace
{
	class ParticleAuraTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			GameObject* pool = GameWorld::Instance()->CreateGameObject("Particles");
			particles = pool->AddComponent<ParticleSystemComponent>();
			particles->SetCapacity(256);

			spec = ParticleSpec();
			spec.count = 1;
			spec.ratePerSecond = 20.f;
			spec.duration = 0.f;
			spec.lifeTime = 10.f;
			spec.startSpeed = 50.f;
			spec.endSpeed = 10.f;
			spec.startSize = 8.f;
			spec.endSize = 2.f;
			spec.emission = ParticleEmission::Continuous;

			owner = GameWorld::Instance()->CreateGameObject("Owner");
			owner->GetTransform()->SetWorldPosition({500.f, 300.f});
			aura = owner->AddComponent<ParticleAuraComponent>();
			aura->SetSpec(&spec);
			aura->SetRadius(40.f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		ParticleSystemComponent* particles = nullptr;
		ParticleAuraComponent* aura = nullptr;
		GameObject* owner = nullptr;
		ParticleSpec spec;

		void Step(int times, float delta = 0.1f)
		{
			for (int frame = 0; frame < times; frame++)
			{
				GameWorld::Instance()->Update(delta);
			}
		}
	};
}

TEST_F(ParticleAuraTest, SleepingAuraEmitsNothing)
{
	Step(10);

	EXPECT_FALSE(aura->IsActive());
	EXPECT_EQ(particles->GetActiveCount(), 0u);
}

TEST_F(ParticleAuraTest, ActiveAuraKeepsEmitting)
{
	aura->SetActive(true);

	Step(10);

	EXPECT_EQ(particles->GetActiveCount(), 20u);
}

TEST_F(ParticleAuraTest, RateSetsTheDensity)
{
	spec.ratePerSecond = 5.f;
	aura->SetActive(true);

	Step(10);

	EXPECT_EQ(particles->GetActiveCount(), 5u);
}

TEST_F(ParticleAuraTest, SwitchedOffAuraStopsEmitting)
{
	aura->SetActive(true);
	Step(5);
	std::size_t emitted = particles->GetActiveCount();
	ASSERT_GT(emitted, 0u);

	aura->SetActive(false);
	Step(10);

	EXPECT_EQ(particles->GetActiveCount(), emitted);
}

TEST_F(ParticleAuraTest, AuraFollowsItsOwner)
{
	aura->SetActive(true);
	Step(2);
	std::size_t before = particles->GetActiveCount();

	owner->GetTransform()->SetWorldPosition({-1000.f, -1000.f});
	Step(2);

	EXPECT_GT(particles->GetActiveCount(), before);
}

TEST_F(ParticleAuraTest, AuraWithoutSpecIsHarmless)
{
	aura->SetSpec(nullptr);
	aura->SetActive(true);

	Step(10);

	EXPECT_EQ(particles->GetActiveCount(), 0u);
}

TEST_F(ParticleAuraTest, NegativeRadiusIsClamped)
{
	aura->SetRadius(-50.f);

	EXPECT_FLOAT_EQ(aura->GetRadius(), 0.f);
}

TEST_F(ParticleAuraTest, RestartDropsTheLeftover)
{
	aura->SetActive(true);
	Step(1, 0.04f);
	ASSERT_EQ(particles->GetActiveCount(), 0u);

	aura->SetActive(false);
	aura->SetActive(true);
	Step(1, 0.04f);

	EXPECT_EQ(particles->GetActiveCount(), 0u);
}

TEST(SpriteRendererTest, SizeWithoutTextureIsIgnored)
{
	GameWorld::Instance()->Clear();

	GameObject* object = GameWorld::Instance()->CreateGameObject("Sprite");
	auto renderer = object->AddComponent<XYZEngine::SpriteRendererComponent>();

	renderer->SetPixelSize(96, 96);

	EXPECT_EQ(renderer->GetSprite()->getTexture(), nullptr);

	GameWorld::Instance()->Clear();
}

TEST(GameWorldCleanupTest, ObjectsAreDestroyedByName)
{
	GameWorld::Instance()->Clear();

	GameWorld::Instance()->CreateGameObject("BloodPool");
	GameWorld::Instance()->CreateGameObject("BloodPool");
	GameWorld::Instance()->CreateGameObject("BloodPool");
	GameObject* player = GameWorld::Instance()->CreateGameObject("Player");
	ASSERT_EQ(GameWorld::Instance()->GetObjectsCount(), 4u);

	GameWorld::Instance()->DestroyGameObjects("BloodPool");
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 1u);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Player"), player);

	GameWorld::Instance()->Clear();
}

TEST(GameWorldCleanupTest, UnknownNameChangesNothing)
{
	GameWorld::Instance()->Clear();
	GameWorld::Instance()->CreateGameObject("Player");

	GameWorld::Instance()->DestroyGameObjects("Rocket");
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 1u);

	GameWorld::Instance()->Clear();
}

TEST(ParticleClearTest, ClearDropsLiveParticles)
{
	GameWorld::Instance()->Clear();

	GameObject* pool = GameWorld::Instance()->CreateGameObject("Particles");
	auto particles = pool->AddComponent<ParticleSystemComponent>();
	particles->SetCapacity(64);

	ParticleSpec spec;
	spec.count = 10;
	spec.lifeTime = 10.f;
	spec.startSpeed = 20.f;
	particles->Emit(spec, {0.f, 0.f}, {1.f, 0.f});
	ASSERT_EQ(particles->GetActiveCount(), 10u);

	particles->Clear();

	EXPECT_EQ(particles->GetActiveCount(), 0u);

	GameWorld::Instance()->Clear();
}
