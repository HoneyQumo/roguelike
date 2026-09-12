#include "pch.h"
#include "GameWorld.h"
#include "PhysicsSystem.h"
#include "BoxColliderComponent.h"
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
