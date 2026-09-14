#include "pch.h"
#include "GameWorld.h"
#include "PhysicsSystem.h"
#include "BoxColliderComponent.h"
#include "SpatialHashGrid.h"
#include "RigidbodyComponent.h"
#include "SpriteRendererComponent.h"
#include "MovementComponent.h"
#include "AimRotationComponent.h"
#include <chrono>

using namespace XYZEngine;

namespace
{
	constexpr int WALLS = 289;
	constexpr int CHARACTERS = 20;
	constexpr float TILE = 64.f;
	constexpr int FRAMES = 200;

	constexpr int ACT_WALLS = 900;
	constexpr int ACT_COLUMNS = 64;
	constexpr int ACT_CHARACTERS = 55;
	constexpr int ACT_SHOTS = 40;

	class PhysicsTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* CreateWall(float x, float y)
		{
			GameObject* wall = GameWorld::Instance()->CreateGameObject("Wall");
			wall->GetTransform()->SetWorldPosition({x, y});

			auto collider = wall->AddComponent<BoxColliderComponent>();
			collider->SetSize(TILE, TILE);

			return wall;
		}

		// Персонаж носит примерно столько же компонентов, сколько враг в игре.
		GameObject* CreateCharacter(float x, float y)
		{
			GameObject* character = GameWorld::Instance()->CreateGameObject("Character");
			character->GetTransform()->SetWorldPosition({x, y});

			character->AddComponent<SpriteRendererComponent>();
			character->AddComponent<MovementComponent>();
			character->AddComponent<AimRotationComponent>();
			character->AddComponent<RigidbodyComponent>()->SetKinematic(false);

			auto collider = character->AddComponent<BoxColliderComponent>();
			collider->SetSize(30.f, 30.f);

			return character;
		}

		void BuildActLevel()
		{
			for (int i = 0; i < ACT_WALLS; i++)
			{
				CreateWall(static_cast<float>(i % ACT_COLUMNS) * TILE, static_cast<float>(i / ACT_COLUMNS) * TILE);
			}

			for (int i = 0; i < ACT_CHARACTERS; i++)
			{
				CreateCharacter(static_cast<float>(i) * TILE + 16.f, 7.f * TILE);
			}

			GameWorld::Instance()->Update(0.016f);
		}

		void BuildLevel()
		{
			for (int i = 0; i < WALLS; i++)
			{
				CreateWall(static_cast<float>(i % 17) * TILE, static_cast<float>(i / 17) * TILE);
			}

			for (int i = 0; i < CHARACTERS; i++)
			{
				CreateCharacter(static_cast<float>(i) * TILE + 16.f, 8.f * TILE);
			}

			GameWorld::Instance()->Update(0.016f);
		}
	};
}

TEST_F(PhysicsTest, EveryColliderIsRegistered)
{
	BuildLevel();

	EXPECT_EQ(PhysicsSystem::Instance()->GetColliders().size(), WALLS + CHARACTERS);
}

TEST_F(PhysicsTest, DestroyedColliderLeavesThePhysicsSystem)
{
	GameObject* wall = CreateWall(0.f, 0.f);
	GameWorld::Instance()->Update(0.016f);
	ASSERT_EQ(PhysicsSystem::Instance()->GetColliders().size(), 1u);

	GameWorld::Instance()->DestroyGameObject(wall);
	GameWorld::Instance()->LateUpdate();

	EXPECT_EQ(PhysicsSystem::Instance()->GetColliders().size(), 0u);
}

TEST_F(PhysicsTest, BenchmarkUpdateOnLevelSizedWorld)
{
	BuildLevel();

	auto started = std::chrono::steady_clock::now();
	for (int frame = 0; frame < FRAMES; frame++)
	{
		PhysicsSystem::Instance()->Update();
	}
	auto elapsed = std::chrono::steady_clock::now() - started;

	double millisecondsPerFrame = std::chrono::duration<double, std::milli>(elapsed).count() / FRAMES;
	std::cout << "PhysicsSystem::Update with " << (WALLS + CHARACTERS) << " colliders: "
		<< millisecondsPerFrame << " ms per frame" << std::endl;
}

TEST_F(PhysicsTest, BenchmarkRenderOrderWithSpawns)
{
	BuildLevel();

	auto started = std::chrono::steady_clock::now();
	for (int frame = 0; frame < FRAMES; frame++)
	{
		GameObject* effect = GameWorld::Instance()->CreateGameObject("Fx");
		effect->SetRenderLayer(6);

		GameWorld::Instance()->Render();

		GameWorld::Instance()->DestroyGameObject(effect);
		GameWorld::Instance()->LateUpdate();
	}
	auto elapsed = std::chrono::steady_clock::now() - started;

	double millisecondsPerFrame = std::chrono::duration<double, std::milli>(elapsed).count() / FRAMES;
	std::cout << "GameWorld::Render order with a spawn every frame, " << (WALLS + CHARACTERS + 1) << " objects: "
		<< millisecondsPerFrame << " ms per frame" << std::endl;
}

TEST_F(PhysicsTest, BenchmarkTransformLookup)
{
	GameObject* character = CreateCharacter(0.f, 0.f);
	constexpr int LOOKUPS = 200000;

	auto searchStarted = std::chrono::steady_clock::now();
	for (int i = 0; i < LOOKUPS; i++)
	{
		ASSERT_NE(character->GetComponent<TransformComponent>(), nullptr);
	}
	auto searchElapsed = std::chrono::steady_clock::now() - searchStarted;

	auto directStarted = std::chrono::steady_clock::now();
	for (int i = 0; i < LOOKUPS; i++)
	{
		ASSERT_NE(character->GetTransform(), nullptr);
	}
	auto directElapsed = std::chrono::steady_clock::now() - directStarted;

	std::cout << "GetComponent<TransformComponent> on a character: "
		<< std::chrono::duration<double, std::micro>(searchElapsed).count() / LOOKUPS << " us per call" << std::endl;
	std::cout << "GetTransform on a character: "
		<< std::chrono::duration<double, std::micro>(directElapsed).count() / LOOKUPS << " us per call" << std::endl;
}

TEST_F(PhysicsTest, BenchmarkMissingComponentLookup)
{
	GameObject* wall = CreateWall(0.f, 0.f);
	constexpr int LOOKUPS = 200000;

	auto started = std::chrono::steady_clock::now();
	for (int i = 0; i < LOOKUPS; i++)
	{
		ASSERT_EQ(wall->GetComponent<RigidbodyComponent>(), nullptr);
	}
	auto elapsed = std::chrono::steady_clock::now() - started;

	double microseconds = std::chrono::duration<double, std::micro>(elapsed).count() / LOOKUPS;
	std::cout << "GetComponent<RigidbodyComponent> missing on a wall: " << microseconds << " us per call" << std::endl;
}

TEST_F(PhysicsTest, MovingBodyIsPushedOutOfAWall)
{
	CreateWall(0.f, 0.f);
	GameObject* character = CreateCharacter(20.f, 0.f);
	GameWorld::Instance()->Update(0.016f);

	PhysicsSystem::Instance()->Update();

	EXPECT_GT(character->GetTransform()->GetWorldPosition().x, 20.f);
}

TEST_F(PhysicsTest, BodyAwayFromWallsStaysWhereItIs)
{
	CreateWall(0.f, 0.f);
	GameObject* character = CreateCharacter(500.f, 0.f);
	GameWorld::Instance()->Update(0.016f);

	PhysicsSystem::Instance()->Update();

	EXPECT_FLOAT_EQ(character->GetTransform()->GetWorldPosition().x, 500.f);
}

TEST_F(PhysicsTest, IgnoredLayerLetsTheBodyThrough)
{
	GameObject* wall = CreateWall(0.f, 0.f);
	wall->GetComponent<BoxColliderComponent>()->SetCollisionLayer(4u);

	GameObject* character = CreateCharacter(20.f, 0.f);
	character->GetComponent<BoxColliderComponent>()->SetIgnoredLayers(4u);
	GameWorld::Instance()->Update(0.016f);

	PhysicsSystem::Instance()->Update();

	EXPECT_FLOAT_EQ(character->GetTransform()->GetWorldPosition().x, 20.f);
}

TEST_F(PhysicsTest, WallIsFoundWhereverTheBodyStands)
{
	for (int i = 0; i < ACT_COLUMNS; i++)
	{
		CreateWall(static_cast<float>(i) * TILE, 0.f);
	}

	GameObject* character = CreateCharacter(-10000.f, 0.f);

	for (int i = 0; i < ACT_COLUMNS; i++)
	{
		float standing = static_cast<float>(i) * TILE + 10.f;
		character->GetTransform()->SetWorldPosition({standing, 0.f});
		GameWorld::Instance()->Update(0.016f);
		PhysicsSystem::Instance()->Update();

		EXPECT_NE(character->GetTransform()->GetWorldPosition().x, standing) << "wall " << i;
	}
}

TEST_F(PhysicsTest, TriggerFiresOnEnterAndOnExit)
{
	GameObject* zone = GameWorld::Instance()->CreateGameObject("Zone");
	zone->GetTransform()->SetWorldPosition({0.f, 0.f});

	auto zoneCollider = zone->AddComponent<BoxColliderComponent>();
	zoneCollider->SetSize(TILE, TILE);
	zoneCollider->SetTrigger(true);

	int entered = 0;
	int exited = 0;
	zoneCollider->SubscribeTriggerEnter([&entered](const Trigger&) { entered++; });
	zoneCollider->SubscribeTriggerExit([&exited](const Trigger&) { exited++; });

	GameObject* character = CreateCharacter(1000.f, 0.f);
	GameWorld::Instance()->Update(0.016f);
	PhysicsSystem::Instance()->Update();
	EXPECT_EQ(entered, 0);

	character->GetTransform()->SetWorldPosition({0.f, 0.f});
	GameWorld::Instance()->Update(0.016f);
	PhysicsSystem::Instance()->Update();
	EXPECT_EQ(entered, 1);
	EXPECT_EQ(exited, 0);

	character->GetTransform()->SetWorldPosition({1000.f, 0.f});
	GameWorld::Instance()->Update(0.016f);
	PhysicsSystem::Instance()->Update();
	EXPECT_EQ(entered, 1);
	EXPECT_EQ(exited, 1);
}

TEST_F(PhysicsTest, OverlapReturnsOnlyTouchedColliders)
{
	GameObject* touched = CreateWall(0.f, 0.f);
	CreateWall(1000.f, 0.f);
	GameWorld::Instance()->Update(0.016f);

	auto found = PhysicsSystem::Instance()->Overlap(sf::FloatRect(-10.f, -10.f, 20.f, 20.f));

	ASSERT_EQ(found.size(), 1u);
	EXPECT_EQ(found[0]->GetGameObject(), touched);
}

TEST_F(PhysicsTest, OverlapKeepsCreationOrder)
{
	GameObject* first = CreateWall(0.f, 0.f);
	GameObject* second = CreateWall(10.f, 0.f);
	GameObject* third = CreateWall(20.f, 0.f);
	GameWorld::Instance()->Update(0.016f);

	auto found = PhysicsSystem::Instance()->Overlap(sf::FloatRect(-5.f, -5.f, 40.f, 10.f));

	ASSERT_EQ(found.size(), 3u);
	EXPECT_EQ(found[0]->GetGameObject(), first);
	EXPECT_EQ(found[1]->GetGameObject(), second);
	EXPECT_EQ(found[2]->GetGameObject(), third);
}

TEST_F(PhysicsTest, OverlapSeesABodyThatJustMoved)
{
	GameObject* character = CreateCharacter(0.f, 0.f);
	GameWorld::Instance()->Update(0.016f);
	ASSERT_EQ(PhysicsSystem::Instance()->Overlap(sf::FloatRect(-10.f, -10.f, 20.f, 20.f)).size(), 1u);

	character->GetTransform()->SetWorldPosition({2000.f, 2000.f});
	GameWorld::Instance()->Update(0.016f);

	EXPECT_TRUE(PhysicsSystem::Instance()->Overlap(sf::FloatRect(-10.f, -10.f, 20.f, 20.f)).empty());
	EXPECT_EQ(PhysicsSystem::Instance()->Overlap(sf::FloatRect(1990.f, 1990.f, 20.f, 20.f)).size(), 1u);
}

TEST_F(PhysicsTest, DestroyedColliderIsNotReturnedByOverlap)
{
	GameObject* wall = CreateWall(0.f, 0.f);
	GameWorld::Instance()->Update(0.016f);
	ASSERT_EQ(PhysicsSystem::Instance()->Overlap(sf::FloatRect(-10.f, -10.f, 20.f, 20.f)).size(), 1u);

	GameWorld::Instance()->DestroyGameObject(wall);
	GameWorld::Instance()->LateUpdate();

	EXPECT_TRUE(PhysicsSystem::Instance()->Overlap(sf::FloatRect(-10.f, -10.f, 20.f, 20.f)).empty());
}

TEST_F(PhysicsTest, CellSizeChangeKeepsCollidersFindable)
{
	CreateWall(0.f, 0.f);
	GameWorld::Instance()->Update(0.016f);

	PhysicsSystem::Instance()->SetCellSize(32.f);
	EXPECT_EQ(PhysicsSystem::Instance()->GetCellSize(), 32.f);
	EXPECT_EQ(PhysicsSystem::Instance()->Overlap(sf::FloatRect(-10.f, -10.f, 20.f, 20.f)).size(), 1u);

	PhysicsSystem::Instance()->SetCellSize(DEFAULT_GRID_CELL_SIZE);
	EXPECT_EQ(PhysicsSystem::Instance()->Overlap(sf::FloatRect(-10.f, -10.f, 20.f, 20.f)).size(), 1u);
}

TEST_F(PhysicsTest, ColliderWithNegativeSizeIsStillFound)
{
	GameObject* odd = GameWorld::Instance()->CreateGameObject("Odd");
	odd->GetTransform()->SetWorldPosition({0.f, 0.f});
	odd->AddComponent<BoxColliderComponent>()->SetSize(-40.f, -40.f);
	GameWorld::Instance()->Update(0.016f);

	EXPECT_EQ(PhysicsSystem::Instance()->Overlap(sf::FloatRect(-10.f, -10.f, 20.f, 20.f)).size(), 1u);
}

TEST_F(PhysicsTest, WorldClearedFromInsideACallbackSurvivesTheFrame)
{
	GameObject* zone = GameWorld::Instance()->CreateGameObject("Zone");
	zone->GetTransform()->SetWorldPosition({0.f, 0.f});

	auto zoneCollider = zone->AddComponent<BoxColliderComponent>();
	zoneCollider->SetSize(TILE, TILE);
	zoneCollider->SetTrigger(true);
	zoneCollider->SubscribeTriggerEnter([](const Trigger&) { GameWorld::Instance()->Clear(); });

	CreateWall(10.f, 10.f);
	CreateWall(-10.f, -10.f);
	CreateCharacter(0.f, 0.f);
	GameWorld::Instance()->Update(0.016f);

	PhysicsSystem::Instance()->Update();

	EXPECT_EQ(PhysicsSystem::Instance()->GetColliders().size(), 0u);
}

TEST_F(PhysicsTest, BenchmarkUpdateOnActSizedWorld)
{
	BuildActLevel();

	auto started = std::chrono::steady_clock::now();
	for (int frame = 0; frame < FRAMES; frame++)
	{
		PhysicsSystem::Instance()->Update();
	}
	auto elapsed = std::chrono::steady_clock::now() - started;

	double millisecondsPerFrame = std::chrono::duration<double, std::milli>(elapsed).count() / FRAMES;
	std::cout << "PhysicsSystem::Update with " << (ACT_WALLS + ACT_CHARACTERS) << " colliders: "
		<< millisecondsPerFrame << " ms per frame" << std::endl;
}

TEST_F(PhysicsTest, BenchmarkOverlapOnActSizedWorld)
{
	BuildActLevel();

	auto started = std::chrono::steady_clock::now();
	for (int frame = 0; frame < FRAMES; frame++)
	{
		for (int shot = 0; shot < ACT_SHOTS; shot++)
		{
			float x = static_cast<float>((frame * ACT_SHOTS + shot) % (ACT_COLUMNS * static_cast<int>(TILE)));
			PhysicsSystem::Instance()->Overlap(sf::FloatRect(x, 7.f * TILE, 16.f, 16.f));
		}
	}
	auto elapsed = std::chrono::steady_clock::now() - started;

	double millisecondsPerFrame = std::chrono::duration<double, std::milli>(elapsed).count() / FRAMES;
	std::cout << "PhysicsSystem::Overlap " << ACT_SHOTS << " times with " << (ACT_WALLS + ACT_CHARACTERS) << " colliders: "
		<< millisecondsPerFrame << " ms per frame" << std::endl;
}
