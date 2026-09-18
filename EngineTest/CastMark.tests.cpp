#include "pch.h"
#include "GameWorld.h"
#include "CastMark.h"
#include "HealthComponent.h"

using RoguelikeGame::CastMarkComponent;
using RoguelikeGame::CreateCastMark;
using RoguelikeGame::HealthComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Vector2Df;

namespace
{
	class CastMarkTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			boss = GameWorld::Instance()->CreateGameObject("Boss");
			boss->AddComponent<HealthComponent>()->SetMaxHealth(600.f);

			player = GameWorld::Instance()->CreateGameObject("Player");
			player->AddComponent<HealthComponent>()->SetMaxHealth(100.f);
			player->GetTransform()->SetWorldPosition({400.f, 0.f});

			detonations = 0;
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* boss = nullptr;
		GameObject* player = nullptr;
		int detonations = 0;
		Vector2Df detonatedAt = {0.f, 0.f};

		CastMarkComponent* Throw(float fuseTime = 0.9f)
		{
			GameObject* markObject = CreateCastMark({0.f, 0.f}, 190.f, fuseTime);

			auto mark = markObject->GetComponent<CastMarkComponent>();
			mark->SetTargetName("Player");
			mark->SetOwnerName("Boss");
			mark->SetOnDetonate([this](const Vector2Df& center)
			{
				detonations++;
				detonatedAt = center;
			});

			return mark;
		}

		GameObject* FindMark() const { return GameWorld::Instance()->FindGameObject("CastMark"); }

		void Step(int times, float delta = 0.05f)
		{
			for (int frame = 0; frame < times; frame++)
			{
				GameWorld::Instance()->Update(delta);
				GameWorld::Instance()->LateUpdate();
			}
		}
	};
}

TEST_F(CastMarkTest, MarkStartsWhereItWasThrown)
{
	CastMarkComponent* mark = Throw();

	EXPECT_TRUE(mark->IsFlying());
	EXPECT_FLOAT_EQ(mark->GetGameObject()->GetTransform()->GetWorldPosition().x, 0.f);
}

TEST_F(CastMarkTest, MarkFliesTowardsThePlayer)
{
	CastMarkComponent* mark = Throw();

	Step(4);

	EXPECT_TRUE(mark->IsFlying());
	EXPECT_GT(mark->GetGameObject()->GetTransform()->GetWorldPosition().x, 30.f);
	EXPECT_EQ(detonations, 0);
}

TEST_F(CastMarkTest, MarkFollowsThePlayerWhoMoves)
{
	CastMarkComponent* mark = Throw();
	player->GetTransform()->SetWorldPosition({0.f, 400.f});

	Step(4);

	Vector2Df position = mark->GetGameObject()->GetTransform()->GetWorldPosition();
	EXPECT_GT(position.y, 30.f);
	EXPECT_NEAR(position.x, 0.f, 1.f);
}

TEST_F(CastMarkTest, MarkLandsOnTouchAndWaitsForItsFuse)
{
	player->GetTransform()->SetWorldPosition({20.f, 0.f});
	CastMarkComponent* mark = Throw();

	Step(1);

	EXPECT_FALSE(mark->IsFlying());
	EXPECT_EQ(detonations, 0);
	EXPECT_NEAR(mark->GetFusePart(), 1.f, 0.2f);
}

TEST_F(CastMarkTest, LandedMarkDetonatesWhereItLanded)
{
	player->GetTransform()->SetWorldPosition({20.f, 0.f});
	Throw(0.3f);

	Step(10);

	EXPECT_EQ(detonations, 1);
	EXPECT_NEAR(detonatedAt.x, 0.f, 35.f);
	EXPECT_EQ(FindMark(), nullptr);
}

TEST_F(CastMarkTest, PlayerCanLeaveTheLandedMark)
{
	player->GetTransform()->SetWorldPosition({20.f, 0.f});
	Throw(0.3f);
	Step(1);

	player->GetTransform()->SetWorldPosition({2000.f, 0.f});
	Step(10);

	EXPECT_EQ(detonations, 1);
	EXPECT_LT(detonatedAt.x, 100.f);
}

TEST_F(CastMarkTest, MarkGivesUpWhenItCannotCatchTheTarget)
{
	CastMarkComponent* mark = Throw();
	player->GetTransform()->SetWorldPosition({100000.f, 0.f});

	Step(100, 0.1f);

	EXPECT_EQ(detonations, 0);
	EXPECT_EQ(FindMark(), nullptr);
}

TEST_F(CastMarkTest, DeadTargetCancelsTheMark)
{
	Throw();

	player->GetComponent<HealthComponent>()->TakeDamage(999.f);
	Step(2);

	EXPECT_EQ(detonations, 0);
	EXPECT_EQ(FindMark(), nullptr);
}

TEST_F(CastMarkTest, DeadOwnerCancelsTheMark)
{
	Throw();

	boss->GetComponent<HealthComponent>()->TakeDamage(999.f);
	Step(2);

	EXPECT_EQ(detonations, 0);
	EXPECT_EQ(FindMark(), nullptr);
}

TEST_F(CastMarkTest, MarkWithoutTargetDisappears)
{
	GameObject* markObject = CreateCastMark({0.f, 0.f}, 190.f, 0.5f);
	markObject->GetComponent<CastMarkComponent>()->SetOwnerName("Boss");

	Step(2);

	EXPECT_EQ(FindMark(), nullptr);
}

// Метка принадлежит локации и обязана уехать с ней: решение принимается у фабрики.
TEST(CastMarkLifetimeTest, ACastMarkBelongsToTheLevel)
{
	XYZEngine::GameWorld::Instance()->Clear();

	XYZEngine::GameObject* mark = RoguelikeGame::CreateCastMark({0.f, 0.f}, 60.f, 1.f);
	ASSERT_NE(mark, nullptr);

	EXPECT_TRUE(mark->IsTemporary()) << "метка переживёт смену локации";

	XYZEngine::GameWorld::Instance()->Clear();
}
