#include "pch.h"
#include "GameWorld.h"
#include "Level.h"
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

TEST_F(GameWorldTest, ClearRemovesWholeHierarchy)
{
	GameObject* parent = GameWorld::Instance()->CreateGameObject("Parent");
	GameWorld::Instance()->CreateGameObject("FirstChild", parent);
	GameWorld::Instance()->CreateGameObject("SecondChild", parent);
	GameWorld::Instance()->CreateGameObject("Standalone");

	GameWorld::Instance()->Clear();

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 0u);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Parent"), nullptr);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("FirstChild"), nullptr);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Standalone"), nullptr);
}

TEST_F(GameWorldTest, ClearAfterDestroyRequestLeavesNothingToDestroy)
{
	GameObject* parent = GameWorld::Instance()->CreateGameObject("Parent");
	GameWorld::Instance()->CreateGameObject("Child", parent);

	GameWorld::Instance()->DestroyGameObject(parent);
	GameWorld::Instance()->Clear();
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 0u);
}

TEST_F(GameWorldTest, DestroyingSameObjectTwiceInOneFrameIsSafe)
{
	GameObject* enemy = GameWorld::Instance()->CreateGameObject("Enemy");

	GameWorld::Instance()->DestroyGameObject(enemy);
	GameWorld::Instance()->DestroyGameObject(enemy);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 0u);
}

TEST_F(GameWorldTest, ChildCreatedWithAParentDiesWithIt)
{
	GameObject* parent = GameWorld::Instance()->CreateGameObject("Corpse");
	GameWorld::Instance()->CreateGameObject("Item_glock", parent);
	ASSERT_NE(GameWorld::Instance()->FindGameObject("Item_glock"), nullptr);

	GameWorld::Instance()->DestroyGameObject(parent);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Corpse"), nullptr);
	EXPECT_EQ(GameWorld::Instance()->FindGameObject("Item_glock"), nullptr);
	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 0u);
}

TEST_F(GameWorldTest, DestroyingAnObjectAgainAfterItIsGoneIsIgnored)
{
	GameObject* enemy = GameWorld::Instance()->CreateGameObject("Enemy");
	GameWorld::Instance()->CreateGameObject("Player");

	GameWorld::Instance()->DestroyGameObject(enemy);
	GameWorld::Instance()->LateUpdate();

	// Указатель уже мёртв, но держат его многие: половина игры хранит GameObject* без всякой пометки.
	GameWorld::Instance()->DestroyGameObject(enemy);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 1u) << "the world lost an object it was never asked to destroy";
	EXPECT_NE(GameWorld::Instance()->FindGameObject("Player"), nullptr);
}

TEST_F(GameWorldTest, DestroyingNobodyIsHarmless)
{
	GameWorld::Instance()->CreateGameObject("Player");

	GameWorld::Instance()->DestroyGameObject(nullptr);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 1u);
}

TEST_F(GameWorldTest, ALevelSurvivesAnObjectThatLeftItEarly)
{
	RoguelikeGame::Level level;

	GameObject* wall = GameWorld::Instance()->CreateGameObject("Wall");
	GameObject* scene = GameWorld::Instance()->CreateGameObject("Cutscene");
	level.Add(wall);
	level.Add(scene);

	// Сцена доигрывает и убирает себя сама, а уровень всё ещё держит на неё указатель.
	GameWorld::Instance()->DestroyGameObject(scene);
	GameWorld::Instance()->LateUpdate();

	level.Clear();
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(GameWorld::Instance()->GetObjectsCount(), 0u);
}
