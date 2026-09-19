#include "pch.h"
#include "FactionComponent.h"
#include "GameSettings.h"
#include "InteractionComponent.h"
#include "SwitchComponent.h"
#include <GameWorld.h>
#include <InputComponent.h>
#include <InputSystem.h>

using RoguelikeGame::Faction;
using RoguelikeGame::FactionComponent;
using RoguelikeGame::InteractionComponent;
using RoguelikeGame::SwitchComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::InputAction;
using XYZEngine::InputSystem;

namespace
{
	constexpr float HOLD_TIME = 1.f;

	sf::Event KeyEvent(sf::Event::EventType type, sf::Keyboard::Key key)
	{
		sf::Event event;
		event.type = type;
		event.key.code = key;

		return event;
	}

	class HoldToActTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			InputSystem::Instance()->Reset();
			InputSystem::Instance()->BeginFrame();

			interactKey = XYZEngine::GetDefaultBindings()[static_cast<int>(InputAction::Interact)].key;

			GameObject* leverObject = GameWorld::Instance()->CreateGameObject("Lever");
			lever = leverObject->AddComponent<SwitchComponent>();
			lever->SetSwitchId("power");
			lever->SetHoldTime(HOLD_TIME);

			player = GameWorld::Instance()->CreateGameObject("Player");
			player->AddComponent<FactionComponent>()->SetFaction(Faction::Player);
			player->AddComponent<XYZEngine::InputComponent>();
			interaction = player->AddComponent<InteractionComponent>();

			Step();
			interaction->AddCandidate(lever);
		}

		void TearDown() override
		{
			InputSystem::Instance()->Reset();
			GameWorld::Instance()->Clear();
		}

		void PressHold()
		{
			InputSystem::Instance()->BeginFrame();
			InputSystem::Instance()->HandleEvent(KeyEvent(sf::Event::KeyPressed, interactKey));
			GameWorld::Instance()->Update(0.05f);
			GameWorld::Instance()->LateUpdate();
		}

		void Release()
		{
			InputSystem::Instance()->BeginFrame();
			InputSystem::Instance()->HandleEvent(KeyEvent(sf::Event::KeyReleased, interactKey));
			GameWorld::Instance()->Update(0.05f);
			GameWorld::Instance()->LateUpdate();
		}

		void KeepHolding(float seconds)
		{
			for (float passed = 0.f; passed < seconds; passed += 0.05f)
			{
				InputSystem::Instance()->BeginFrame();
				GameWorld::Instance()->Update(0.05f);
				GameWorld::Instance()->LateUpdate();
			}
		}

		void Step()
		{
			InputSystem::Instance()->BeginFrame();
			GameWorld::Instance()->Update(0.05f);
			GameWorld::Instance()->LateUpdate();
		}

		void MovePlayer(float dx)
		{
			player->GetTransform()->SetWorldPosition({dx, 0.f});
		}

		sf::Keyboard::Key interactKey = sf::Keyboard::E;
		SwitchComponent* lever = nullptr;
		InteractionComponent* interaction = nullptr;
		GameObject* player = nullptr;
	};
}

// Удалённый рычаг обязан сказать об этом: выход из триггера при удалении
// не приходит, и без этого взаимодействие бьёт по освобождённой памяти.
TEST_F(HoldToActTest, ALeverDestroyedMidHoldTakesItselfOutOfTheList)
{
	PressHold();
	KeepHolding(0.3f);
	ASSERT_TRUE(interaction->IsHolding());

	GameWorld::Instance()->DestroyGameObject(lever->GetGameObject());
	lever = nullptr;
	KeepHolding(0.5f);

	EXPECT_FALSE(interaction->IsHolding());
	EXPECT_EQ(interaction->GetTarget(), nullptr);
}

// Пока держат - гремит толчками, а не сплошным потоком: сплошной
// шум поднял бы этаж мгновенно.
TEST_F(HoldToActTest, TheSwitchRattlesWhileItIsHeld)
{
	PressHold();
	KeepHolding(0.9f);

	EXPECT_GT(lever->GetHoldTicks(), 0);
	EXPECT_LE(lever->GetHoldTicks(), 3) << "гремит слишком часто";
}

TEST_F(HoldToActTest, ABrokenHoldStopsTheRattle)
{
	PressHold();
	KeepHolding(0.9f);
	int rattles = lever->GetHoldTicks();
	ASSERT_GT(rattles, 0);

	Release();
	KeepHolding(1.f);

	EXPECT_EQ(lever->GetHoldTicks(), rattles);
}

// Мгновенный рычаг работает как работал: цена есть только там, где её задали.
TEST_F(HoldToActTest, AnInstantLeverStillPullsOnOnePress)
{
	lever->SetHoldTime(0.f);

	PressHold();

	EXPECT_TRUE(lever->IsPulled());
	EXPECT_FALSE(interaction->IsHolding());
}

TEST_F(HoldToActTest, OnePressIsNotEnoughWhenThereIsAHoldTime)
{
	PressHold();
	KeepHolding(0.2f);

	EXPECT_FALSE(lever->IsPulled());
	EXPECT_TRUE(interaction->IsHolding());
	EXPECT_GT(interaction->GetHoldPart(), 0.f);
	EXPECT_LT(interaction->GetHoldPart(), 1.f);
}

TEST_F(HoldToActTest, HoldingLongEnoughPullsIt)
{
	PressHold();
	KeepHolding(HOLD_TIME + 0.2f);

	EXPECT_TRUE(lever->IsPulled());
	EXPECT_FALSE(interaction->IsHolding());
	EXPECT_FLOAT_EQ(interaction->GetHoldPart(), 0.f);
}

TEST_F(HoldToActTest, LettingGoEarlyLosesEverything)
{
	PressHold();
	KeepHolding(0.4f);
	ASSERT_GT(interaction->GetHoldPart(), 0.f);

	Release();

	EXPECT_FALSE(lever->IsPulled());
	EXPECT_FALSE(interaction->IsHolding());
	EXPECT_FLOAT_EQ(interaction->GetHoldPart(), 0.f);
}

// Сдвинулся - сорвалось. В этом вся цена: рычаг нельзя дёрнуть на бегу.
TEST_F(HoldToActTest, StepAsideAndTheCountIsLost)
{
	PressHold();
	KeepHolding(0.4f);

	MovePlayer(RoguelikeGame::HOLD_SLIP * 2.f);
	KeepHolding(0.1f);

	EXPECT_FALSE(lever->IsPulled());
	EXPECT_FALSE(interaction->IsHolding());
}

TEST_F(HoldToActTest, ASlightShiftDoesNotBreakIt)
{
	PressHold();
	KeepHolding(0.4f);

	MovePlayer(RoguelikeGame::HOLD_SLIP * 0.5f);
	KeepHolding(HOLD_TIME);

	EXPECT_TRUE(lever->IsPulled());
}
