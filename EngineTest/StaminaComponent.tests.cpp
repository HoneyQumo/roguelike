#include "pch.h"
#include "GameWorld.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "MovementComponent.h"
#include "StaminaComponent.h"

using RoguelikeGame::StaminaComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::MovementComponent;

namespace
{
	sf::Event KeyEvent(sf::Keyboard::Key key)
	{
		sf::Event event;
		event.type = sf::Event::KeyPressed;
		event.key.code = key;
		return event;
	}

	class StaminaTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			GameWorld::Instance()->Clear();
			XYZEngine::InputSystem::Instance()->Reset();
		}

		void TearDown() override
		{
			GameWorld::Instance()->Clear();
			XYZEngine::InputSystem::Instance()->Reset();
		}

		GameObject* owner = nullptr;

		StaminaComponent* CreateStamina(float maxStamina = 100.f)
		{
			owner = GameWorld::Instance()->CreateGameObject("Player");
			auto stamina = owner->AddComponent<StaminaComponent>();
			stamina->SetMaxStamina(maxStamina);
			stamina->SetRegen(20.f, 0.5f);
			stamina->SetRunDrain(50.f);
			stamina->SetRunResumePart(0.25f);

			return stamina;
		}
	};
}

TEST_F(StaminaTest, StartsFull)
{
	StaminaComponent* stamina = CreateStamina(80.f);

	EXPECT_FLOAT_EQ(stamina->GetStamina(), 80.f);
	EXPECT_FLOAT_EQ(stamina->GetMaxStamina(), 80.f);
	EXPECT_FLOAT_EQ(stamina->GetStaminaPercent(), 1.f);
	EXPECT_FALSE(stamina->IsExhausted());
}

TEST_F(StaminaTest, SpendingTakesExactlyTheAmount)
{
	StaminaComponent* stamina = CreateStamina();

	EXPECT_TRUE(stamina->TrySpend(30.f));

	EXPECT_FLOAT_EQ(stamina->GetStamina(), 70.f);
}

TEST_F(StaminaTest, SpendingMoreThanLeftIsRefused)
{
	StaminaComponent* stamina = CreateStamina();
	stamina->TrySpend(90.f);

	EXPECT_FALSE(stamina->TrySpend(30.f));
	EXPECT_FLOAT_EQ(stamina->GetStamina(), 10.f);
}

TEST_F(StaminaTest, RegenWaitsForTheDelayThenRefills)
{
	StaminaComponent* stamina = CreateStamina();
	stamina->TrySpend(50.f);

	owner->Update(0.3f);
	EXPECT_FLOAT_EQ(stamina->GetStamina(), 50.f);

	owner->Update(0.3f);
	owner->Update(0.5f);

	EXPECT_GT(stamina->GetStamina(), 50.f);
	EXPECT_LE(stamina->GetStamina(), 100.f);
}

TEST_F(StaminaTest, RegenStopsAtMaximum)
{
	StaminaComponent* stamina = CreateStamina();
	stamina->TrySpend(10.f);

	for (int frame = 0; frame < 200; frame++)
	{
		owner->Update(0.1f);
	}

	EXPECT_FLOAT_EQ(stamina->GetStamina(), 100.f);
}

TEST_F(StaminaTest, ChangeEventCarriesCurrentAndMaximum)
{
	StaminaComponent* stamina = CreateStamina();
	float seen = 0.f;
	float seenMax = 0.f;
	stamina->SubscribeChanged([&seen, &seenMax](float current, float maximum)
	{
		seen = current;
		seenMax = maximum;
	});

	stamina->TrySpend(25.f);

	EXPECT_FLOAT_EQ(seen, 75.f);
	EXPECT_FLOAT_EQ(seenMax, 100.f);
}

TEST_F(StaminaTest, UnsubscribeStopsEvents)
{
	StaminaComponent* stamina = CreateStamina();
	int calls = 0;
	XYZEngine::SubscriptionId id = stamina->SubscribeChanged([&calls](float, float) { calls++; });

	stamina->UnsubscribeChanged(id);
	stamina->TrySpend(10.f);

	EXPECT_EQ(calls, 0);
}

TEST_F(StaminaTest, RunningDrainsStaminaAndStopsAtZero)
{
	XYZEngine::InputSystem::Instance()->Reset();
	XYZEngine::InputSystem::Instance()->HandleEvent(KeyEvent(sf::Keyboard::W));
	XYZEngine::InputSystem::Instance()->HandleEvent(KeyEvent(sf::Keyboard::LShift));

	owner = GameWorld::Instance()->CreateGameObject("Player");
	owner->AddComponent<XYZEngine::InputComponent>();
	auto movement = owner->AddComponent<MovementComponent>();
	auto stamina = owner->AddComponent<StaminaComponent>();
	movement->SetSpeed(100.f);
	stamina->SetMaxStamina(50.f);
	stamina->SetRunDrain(100.f);
	stamina->SetRegen(10.f, 5.f);
	stamina->SetRunResumePart(0.5f);

	owner->Update(0.1f);
	ASSERT_TRUE(movement->IsRunning());
	EXPECT_FLOAT_EQ(stamina->GetStamina(), 40.f);

	for (int frame = 0; frame < 10; frame++)
	{
		owner->Update(0.1f);
	}

	EXPECT_FLOAT_EQ(stamina->GetStamina(), 0.f);
	EXPECT_FALSE(movement->IsRunAllowed());
	EXPECT_FALSE(movement->IsRunning());

	XYZEngine::InputSystem::Instance()->Reset();
}

TEST_F(StaminaTest, ExhaustedRunnerGetsRunBackAfterRecovery)
{
	owner = GameWorld::Instance()->CreateGameObject("Player");
	auto movement = owner->AddComponent<MovementComponent>();
	auto stamina = owner->AddComponent<StaminaComponent>();
	stamina->SetMaxStamina(100.f);
	stamina->SetRegen(100.f, 0.f);
	stamina->SetRunResumePart(0.25f);

	owner->Update(0.016f);
	stamina->TrySpend(100.f);
	movement->SetRunAllowed(false);

	EXPECT_TRUE(stamina->IsExhausted());

	for (int frame = 0; frame < 10; frame++)
	{
		owner->Update(0.1f);
	}

	EXPECT_FALSE(stamina->IsExhausted());
	EXPECT_TRUE(movement->IsRunAllowed());
}
