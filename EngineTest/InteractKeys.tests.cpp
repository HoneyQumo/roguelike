#include "pch.h"
#include "ContainerComponent.h"
#include "DoorComponent.h"
#include "EscapeCarComponent.h"
#include "HatchComponent.h"
#include "ItemPickupComponent.h"
#include "SwitchComponent.h"
#include <BoxColliderComponent.h>
#include <GameWorld.h>
#include "GameSettings.h"
#include <InputSystem.h>

using RoguelikeGame::ContainerComponent;
using RoguelikeGame::DoorComponent;
using RoguelikeGame::EscapeCarComponent;
using RoguelikeGame::HatchComponent;
using RoguelikeGame::ItemPickupComponent;
using RoguelikeGame::SwitchComponent;
using XYZEngine::GameObject;
using XYZEngine::GameWorld;
using XYZEngine::InputAction;

namespace
{
	/**
	*	Две клавиши, два смысла: E - что-то взять или включить, F - куда-то пройти.
	*	Тест держит именно это правило, а не список компонентов: новая интерактивная
	*	вещь должна осознанно выбрать сторону, а не унаследовать её случайно.
	*/
	class InteractKeysTest : public ::testing::Test
	{
	protected:
		void SetUp() override { GameWorld::Instance()->Clear(); }
		void TearDown() override { GameWorld::Instance()->Clear(); }

		template <class T>
		T* Create(const std::string& name)
		{
			GameObject* body = GameWorld::Instance()->CreateGameObject(name);

			return body->AddComponent<T>();
		}
	};
}

TEST_F(InteractKeysTest, WhatYouTakeOrSwitchIsOnTheInteractKey)
{
	EXPECT_EQ(Create<ItemPickupComponent>("Item")->GetAction(), InputAction::Interact);
	EXPECT_EQ(Create<ContainerComponent>("Crate")->GetAction(), InputAction::Interact);
	EXPECT_EQ(Create<SwitchComponent>("Lever")->GetAction(), InputAction::Interact);
}

TEST_F(InteractKeysTest, WhatYouWalkThroughIsOnThePassKey)
{
	EXPECT_EQ(Create<DoorComponent>("Door")->GetAction(), InputAction::Pass);
	EXPECT_EQ(Create<HatchComponent>("Hatch")->GetAction(), InputAction::Pass);
	EXPECT_EQ(Create<EscapeCarComponent>("Car")->GetAction(), InputAction::Pass);
}

TEST_F(InteractKeysTest, TheTwoKeysAreNotTheSameKey)
{
	auto input = XYZEngine::InputSystem::Instance();

	EXPECT_NE(input->GetBinding(InputAction::Interact).key, input->GetBinding(InputAction::Pass).key)
		<< "both prompts would fire at once";
}

namespace
{
	const char* KeyMark(InputAction action)
	{
		return action == InputAction::Pass ? "[F]" : "[E]";
	}

	// Подсказка обязана называть ту клавишу, на которую вещь отвечает: разойтись
	// они могут незаметно - действие меняют в коде, а текст остаётся в настройках.
	void ExpectPromptMatchesKey(const char* what, const std::string& prompt, InputAction action)
	{
		EXPECT_NE(prompt.find(KeyMark(action)), std::string::npos)
			<< what << " answers to " << KeyMark(action) << " but its prompt says: " << prompt;
	}
}

TEST_F(InteractKeysTest, EveryPromptNamesTheKeyItAnswersTo)
{
	ExpectPromptMatchesKey("the hatch", RoguelikeGame::HATCH_FLEE_PROMPT, InputAction::Pass);
	ExpectPromptMatchesKey("the escape car", RoguelikeGame::ESCAPE_CAR_PROMPT, InputAction::Pass);
	ExpectPromptMatchesKey("the door", RoguelikeGame::DOOR_OPEN_PROMPT, InputAction::Pass);

	ExpectPromptMatchesKey("the lever", RoguelikeGame::LEVER_PROMPT, InputAction::Interact);
	ExpectPromptMatchesKey("an item", RoguelikeGame::INTERACT_PROMPT_PREFIX, InputAction::Interact);
	ExpectPromptMatchesKey("a container", RoguelikeGame::CONTAINER_OPEN_PREFIX, InputAction::Interact);
}

TEST_F(InteractKeysTest, TheDoorAnswersToTheKeyItsPromptNames)
{
	DoorComponent* door = Create<DoorComponent>("Door");

	ExpectPromptMatchesKey("the door", RoguelikeGame::DOOR_OPEN_PROMPT, door->GetAction());
}
