#include "pch.h"
#include "GameWorld.h"
#include "CastMark.h"

using RoguelikeGame::CastMarkComponent;
using RoguelikeGame::CreateCastMark;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	class CastMarkTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		void Step(int times, float delta = 0.1f)
		{
			for (int frame = 0; frame < times; frame++)
			{
				GameWorld::Instance()->Update(delta);
				GameWorld::Instance()->LateUpdate();
			}
		}
	};
}

TEST_F(CastMarkTest, MarkStandsWhereTheAbilityAims)
{
	GameObject* mark = CreateCastMark({320.f, 640.f}, 190.f, 0.6f);

	EXPECT_FLOAT_EQ(mark->GetTransform()->GetWorldPosition().x, 320.f);
	EXPECT_FLOAT_EQ(mark->GetTransform()->GetWorldPosition().y, 640.f);
	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 1u);
}

TEST_F(CastMarkTest, MarkFadesWhileItWaits)
{
	GameObject* mark = CreateCastMark({0.f, 0.f}, 100.f, 1.f);
	auto component = mark->GetComponent<CastMarkComponent>();

	Step(1);
	EXPECT_NEAR(component->GetPart(), 0.9f, 0.001f);

	Step(4);
	EXPECT_NEAR(component->GetPart(), 0.5f, 0.001f);
}

TEST_F(CastMarkTest, MarkLeavesTheWorldWhenTheAbilityFires)
{
	CreateCastMark({0.f, 0.f}, 100.f, 0.5f);

	Step(4);
	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 1u);

	Step(2);
	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 0u);
}

TEST_F(CastMarkTest, MarkWithoutLifeTimeDoesNotLinger)
{
	CreateCastMark({0.f, 0.f}, 100.f, 0.f);

	Step(1);

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 0u);
}
