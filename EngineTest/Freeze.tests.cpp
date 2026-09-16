#include "pch.h"
#include "Freeze.h"
#include <GameWorld.h>
#include <SpriteRendererComponent.h>

using RoguelikeGame::Freeze;
using RoguelikeGame::KeepsDrawing;
using RoguelikeGame::Thaw;
using XYZEngine::Component;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;

namespace
{
	// Что-нибудь, что живёт своей жизнью: заморозка должна гасить именно такое.
	class BrainStub : public Component
	{
	public:
		BrainStub(GameObject* gameObject) : Component(gameObject) {}

		void Update(float deltaTime) override { thought += deltaTime; }
		void Render() override {}

		float thought = 0.f;
	};

	class ShotStub : public Component
	{
	public:
		ShotStub(GameObject* gameObject) : Component(gameObject) {}

		void Update(float deltaTime) override {}
		void Render() override {}
	};

	class FreezeTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();

			guard = GameWorld::Instance()->CreateGameObject("Guard");
			brain = guard->AddComponent<BrainStub>();
			sprite = guard->AddComponent<XYZEngine::SpriteRendererComponent>();
			shot = guard->AddComponent<ShotStub>();
		}

		void TearDown() override { GameWorld::Instance()->Clear(); }

		GameObject* guard = nullptr;
		BrainStub* brain = nullptr;
		XYZEngine::SpriteRendererComponent* sprite = nullptr;
		ShotStub* shot = nullptr;
		std::vector<Component*> frozen;
	};
}

TEST_F(FreezeTest, AFrozenGuardStopsThinkingButStaysOnTheScreen)
{
	Freeze(guard, frozen);

	EXPECT_FALSE(brain->IsEnabled());
	EXPECT_FALSE(shot->IsEnabled());
	EXPECT_TRUE(sprite->IsEnabled()) << "the guard vanished instead of freezing";
	EXPECT_TRUE(guard->GetTransform()->IsEnabled());
}

TEST_F(FreezeTest, AFrozenGuardDoesNotRunWhileTheSceneGoes)
{
	Freeze(guard, frozen);

	GameWorld::Instance()->Update(0.5f);

	EXPECT_FLOAT_EQ(brain->thought, 0.f);

	Thaw(frozen);
	GameWorld::Instance()->Update(0.5f);

	EXPECT_FLOAT_EQ(brain->thought, 0.5f) << "the guard never woke up";
}

TEST_F(FreezeTest, ThawGivesBackExactlyWhatWasTaken)
{
	// Кто-то уже выключил компонент до сцены - и решать, когда включить, ему.
	shot->SetEnabled(false);

	Freeze(guard, frozen);
	Thaw(frozen);

	EXPECT_TRUE(brain->IsEnabled());
	EXPECT_FALSE(shot->IsEnabled()) << "the scene switched on something it never switched off";
}

TEST_F(FreezeTest, FreezingTwiceRemembersEachPartOnce)
{
	Freeze(guard, frozen);
	std::size_t after = frozen.size();

	Freeze(guard, frozen);

	EXPECT_EQ(frozen.size(), after) << "the same component was remembered twice";
}

TEST_F(FreezeTest, FreezingNobodyIsHarmless)
{
	Freeze(nullptr, frozen);

	EXPECT_TRUE(frozen.empty());
}

TEST_F(FreezeTest, WhatDrawsIsNeverTouched)
{
	EXPECT_TRUE(KeepsDrawing(sprite));
	EXPECT_TRUE(KeepsDrawing(guard->GetTransform()));
	EXPECT_FALSE(KeepsDrawing(brain));
}
