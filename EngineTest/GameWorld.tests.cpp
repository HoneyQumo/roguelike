#include "pch.h"
#include "GameWorld.h"
#include <chrono>

using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	class GameWorldTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }
	};
}

TEST_F(GameWorldTest, FindsObjectByName)
{
	GameObject* created = GameWorld::Instance()->CreateGameObject("Player");

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Player"), created);
}

TEST_F(GameWorldTest, ReturnsNullForUnknownName)
{
	GameWorld::Instance()->CreateGameObject("Player");

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("NoSuchObject"), nullptr);
}

TEST_F(GameWorldTest, ReturnsFirstCreatedAmongSameNames)
{
	GameObject* first = GameWorld::Instance()->CreateGameObject("Grunt");
	GameWorld::Instance()->CreateGameObject("Grunt");
	GameWorld::Instance()->CreateGameObject("Grunt");

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Grunt"), first);
}

TEST_F(GameWorldTest, DestroyedObjectIsNotFoundAfterLateUpdate)
{
	GameObject* player = GameWorld::Instance()->CreateGameObject("Player");
	GameWorld::Instance()->DestroyGameObject(player);

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Player"), player);

	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Player"), nullptr);
}

TEST_F(GameWorldTest, DestroyingOneOfSameNamesKeepsTheOthers)
{
	GameObject* first = GameWorld::Instance()->CreateGameObject("Grunt");
	GameObject* second = GameWorld::Instance()->CreateGameObject("Grunt");

	GameWorld::Instance()->DestroyGameObject(first);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Grunt"), second);
}

TEST_F(GameWorldTest, ClearForgetsEveryName)
{
	GameWorld::Instance()->CreateGameObject("Player");
	GameWorld::Instance()->CreateGameObject("Grunt");

	GameWorld::Instance()->Clear();

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Player"), nullptr);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Grunt"), nullptr);
}

TEST_F(GameWorldTest, LookupDoesNotScaleWithObjectCount)
{
	for (int i = 0; i < 600; i++)
	{
		GameWorld::Instance()->CreateGameObject("Wall");
	}
	GameObject* player = GameWorld::Instance()->CreateGameObject("Player");

	constexpr int LOOKUPS = 100000;
	auto started = std::chrono::steady_clock::now();
	for (int i = 0; i < LOOKUPS; i++)
	{
		ASSERT_EQ(GameWorld::Instance()->FindGameObject("Player"), player);
	}
	auto elapsed = std::chrono::steady_clock::now() - started;

	double microsecondsPerLookup = std::chrono::duration<double, std::micro>(elapsed).count() / LOOKUPS;
	std::cout << "FindGameObject among 601 objects: " << microsecondsPerLookup << " us per call" << std::endl;
	EXPECT_LT(microsecondsPerLookup, 5.0);
}

TEST_F(GameWorldTest, DestroyingChildAndParentInOneFrameIsSafe)
{
	GameObject* parent = GameWorld::Instance()->CreateGameObject("Parent");
	GameObject* child = GameWorld::Instance()->CreateGameObject("Child");
	child->GetTransform()->SetParent(parent->GetTransform());

	GameWorld::Instance()->DestroyGameObject(child);
	GameWorld::Instance()->DestroyGameObject(parent);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Parent"), nullptr);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Child"), nullptr);
	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 0u);
}

TEST_F(GameWorldTest, ChildObjectStartsAtTheParentOrigin)
{
	GameObject* parent = GameWorld::Instance()->CreateGameObject("Parent");
	parent->GetTransform()->SetWorldPosition({100.f, 50.f});

	GameObject* child = GameWorld::Instance()->CreateGameObject("Child", parent);

	auto childTransform = child->GetTransform();
	EXPECT_EQ(childTransform->GetParent(), parent->GetTransform());
	EXPECT_FLOAT_EQ(childTransform->GetLocalPosition().x, 0.f);
	EXPECT_FLOAT_EQ(childTransform->GetLocalPosition().y, 0.f);
	EXPECT_FLOAT_EQ(childTransform->GetWorldPosition().x, 100.f);
	EXPECT_FLOAT_EQ(childTransform->GetWorldPosition().y, 50.f);
}
