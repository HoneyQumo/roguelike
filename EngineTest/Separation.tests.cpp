#include "pch.h"
#include "GameWorld.h"
#include "BoxColliderComponent.h"
#include "PhysicsSystem.h"
#include "RigidbodyComponent.h"
#include "Separation.h"

using XYZEngine::BoxColliderComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::Push;
using XYZEngine::RigidbodyComponent;
using XYZEngine::SplitPush;
using XYZEngine::Vector2Df;

TEST(SeparationTests, AWallTakesNothingOfThePush)
{
	Push push = SplitPush({-10.f, 0.f}, {5.f, 0.f}, {0.f, 0.f}, false);

	EXPECT_FLOAT_EQ(push.first.x, -10.f);
	EXPECT_FLOAT_EQ(push.second.x, 0.f);
}

TEST(SeparationTests, TheOneWhoDroveInGivesWayAlone)
{
	Push push = SplitPush({-10.f, 0.f}, {5.f, 0.f}, {0.f, 0.f}, true);

	EXPECT_FLOAT_EQ(push.first.x, -10.f);
	EXPECT_FLOAT_EQ(push.second.x, 0.f);
}

TEST(SeparationTests, AStandingBodyIsNotShovedAside)
{
	Push push = SplitPush({10.f, 0.f}, {0.f, 0.f}, {5.f, 0.f}, true);

	EXPECT_FLOAT_EQ(push.first.x, 0.f);
	EXPECT_FLOAT_EQ(push.second.x, -10.f);
}

TEST(SeparationTests, TwoBodiesMeetingHeadOnShareThePush)
{
	Push push = SplitPush({-10.f, 0.f}, {5.f, 0.f}, {-5.f, 0.f}, true);

	EXPECT_FLOAT_EQ(push.first.x, -5.f);
	EXPECT_FLOAT_EQ(push.second.x, 5.f);
}

TEST(SeparationTests, BodiesBornOverlappedStillComeApart)
{
	Push push = SplitPush({0.f, -8.f}, {0.f, 0.f}, {0.f, 0.f}, true);

	EXPECT_FLOAT_EQ(push.first.y, -4.f);
	EXPECT_FLOAT_EQ(push.second.y, 4.f);
}

TEST(SeparationTests, NoOverlapMovesNobody)
{
	Push push = SplitPush({0.f, 0.f}, {5.f, 0.f}, {-5.f, 0.f}, true);

	EXPECT_TRUE(push.first.IsZero());
	EXPECT_TRUE(push.second.IsZero());
}

namespace
{
	constexpr float BODY = 30.f;
	constexpr float WALL = 64.f;
	constexpr float STEP = 0.016f;

	class SolidBodyTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* CreateBody(const std::string& name, float x, float y)
		{
			GameObject* body = GameWorld::Instance()->CreateGameObject(name);
			body->GetTransform()->SetWorldPosition({x, y});
			body->AddComponent<RigidbodyComponent>()->SetKinematic(false);
			body->AddComponent<BoxColliderComponent>()->SetSize(BODY, BODY);

			return body;
		}

		GameObject* CreateWall(float x, float y)
		{
			GameObject* wall = GameWorld::Instance()->CreateGameObject("Wall");
			wall->GetTransform()->SetWorldPosition({x, y});
			wall->AddComponent<BoxColliderComponent>()->SetSize(WALL, WALL);

			return wall;
		}

		// Ходит только тот, кого двигают: шаг руками, как это делает MovementComponent.
		void WalkRight(GameObject* walker, float speed, int frames)
		{
			for (int frame = 0; frame < frames; frame++)
			{
				walker->GetTransform()->MoveBy({speed * STEP, 0.f});
				Step();
			}
		}

		void Step()
		{
			GameWorld::Instance()->Update(STEP);
			GameWorld::Instance()->UpdatePhysics();
		}
	};
}

TEST_F(SolidBodyTest, WalkingIntoABodyDoesNotShoveItAcrossTheRoom)
{
	GameObject* walker = CreateBody("Walker", 0.f, 0.f);
	GameObject* standing = CreateBody("Standing", BODY, 0.f);
	Step();

	float standingBefore = standing->GetTransform()->GetWorldPosition().x;
	WalkRight(walker, 200.f, 60);

	float shoved = standing->GetTransform()->GetWorldPosition().x - standingBefore;

	EXPECT_LT(shoved, 2.f) << "the standing body was pushed " << shoved << " units";
}

TEST_F(SolidBodyTest, TheWalkerStopsAtTheBodyInsteadOfPassingThrough)
{
	GameObject* walker = CreateBody("Walker", 0.f, 0.f);
	GameObject* standing = CreateBody("Standing", BODY, 0.f);
	Step();

	WalkRight(walker, 200.f, 60);

	float gap = standing->GetTransform()->GetWorldPosition().x - walker->GetTransform()->GetWorldPosition().x;

	EXPECT_GT(gap, BODY - 2.f) << "the walker went through the body";
}

TEST_F(SolidBodyTest, AWallStillStopsTheWalkerCompletely)
{
	GameObject* walker = CreateBody("Walker", 0.f, 0.f);
	GameObject* wall = CreateWall(0.5f * (BODY + WALL), 0.f);
	Step();

	float wallBefore = wall->GetTransform()->GetWorldPosition().x;
	WalkRight(walker, 200.f, 60);

	EXPECT_FLOAT_EQ(wall->GetTransform()->GetWorldPosition().x, wallBefore);
	EXPECT_LT(walker->GetTransform()->GetWorldPosition().x, 0.5f * WALL);
}

TEST_F(SolidBodyTest, BodiesSpawnedOnTopOfEachOtherDriftApart)
{
	GameObject* first = CreateBody("First", 0.f, 0.f);
	GameObject* second = CreateBody("Second", 4.f, 0.f);

	for (int frame = 0; frame < 30; frame++)
	{
		Step();
	}

	Vector2Df apart = second->GetTransform()->GetWorldPosition() - first->GetTransform()->GetWorldPosition();

	EXPECT_GE(apart.GetLength(), BODY - 1.f);
}
