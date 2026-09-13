#include "pch.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "RectangleRendererComponent.h"
#include "DestructibleComponent.h"
#include "HealthComponent.h"

using RoguelikeGame::DestructibleComponent;
using RoguelikeGame::HealthComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	class DestructibleTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			crate = GameWorld::Instance()->CreateGameObject("Prop_crate_wood");
			crate->GetTransform()->SetWorldPosition({320.f, 128.f});

			health = crate->AddComponent<HealthComponent>();
			health->SetMaxHealth(40.f);

			collider = crate->AddComponent<XYZEngine::BoxColliderComponent>();
			collider->SetSize(48.f, 48.f);

			crate->AddComponent<XYZEngine::RectangleRendererComponent>()->SetSize(48.f, 48.f);

			destructible = crate->AddComponent<DestructibleComponent>();

			breaks = 0;
			destructible->SubscribeBroken([this](const Vector2Df& place)
			{
				breaks++;
				brokenAt = place;
			});

			GameWorld::Instance()->Update(0.016f);
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* crate = nullptr;
		HealthComponent* health = nullptr;
		XYZEngine::BoxColliderComponent* collider = nullptr;
		DestructibleComponent* destructible = nullptr;
		int breaks = 0;
		Vector2Df brokenAt = {0.f, 0.f};
	};
}

TEST_F(DestructibleTest, FreshCrateIsWholeAndSolid)
{
	EXPECT_FALSE(destructible->IsBroken());
	EXPECT_FALSE(collider->IsTrigger());
	EXPECT_EQ(breaks, 0);
}

TEST_F(DestructibleTest, PartialDamageDoesNotBreakIt)
{
	health->TakeDamage(20.f);

	EXPECT_FALSE(destructible->IsBroken());
	EXPECT_EQ(breaks, 0);
	EXPECT_FALSE(collider->IsTrigger());
}

TEST_F(DestructibleTest, LastHitBreaksItAndTellsWhere)
{
	health->TakeDamage(20.f);
	health->TakeDamage(20.f);

	EXPECT_TRUE(destructible->IsBroken());
	EXPECT_EQ(breaks, 1);
	EXPECT_FLOAT_EQ(brokenAt.x, 320.f);
	EXPECT_FLOAT_EQ(brokenAt.y, 128.f);
}

TEST_F(DestructibleTest, BrokenCrateStopsBlockingTheWay)
{
	health->TakeDamage(100.f);

	EXPECT_TRUE(collider->IsTrigger());
}

TEST_F(DestructibleTest, CrateBreaksOnlyOnce)
{
	health->TakeDamage(100.f);
	health->TakeDamage(100.f);

	EXPECT_EQ(breaks, 1);
}

TEST_F(DestructibleTest, BrokenCrateStaysInTheWorld)
{
	health->TakeDamage(100.f);
	GameWorld::Instance()->Update(0.016f);
	GameWorld::Instance()->LateUpdate();

	EXPECT_NE(GameWorld::Instance()->FindGameObject("Prop_crate_wood"), nullptr);
}
