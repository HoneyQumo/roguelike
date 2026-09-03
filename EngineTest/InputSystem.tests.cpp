#include "pch.h"
#include "InputSystem.h"

using XYZEngine::InputSystem;

namespace
{
	sf::Event KeyEvent(sf::Event::EventType type, sf::Keyboard::Key key)
	{
		sf::Event event;
		event.type = type;
		event.key.code = key;
		return event;
	}

	sf::Event ButtonEvent(sf::Event::EventType type, sf::Mouse::Button button)
	{
		sf::Event event;
		event.type = type;
		event.mouseButton.button = button;
		return event;
	}

	sf::Event FocusEvent(sf::Event::EventType type)
	{
		sf::Event event;
		event.type = type;
		return event;
	}

	class InputSystemTest : public ::testing::Test
	{
	protected:
		InputSystem* input = InputSystem::Instance();

		void SetUp() override
		{
			input->Reset();
			input->HandleEvent(FocusEvent(sf::Event::GainedFocus));
			input->BeginFrame();
		}
	};
}

TEST_F(InputSystemTest, PressedOnlyOnTheFrameOfTheEvent)
{
	input->HandleEvent(KeyEvent(sf::Event::KeyPressed, sf::Keyboard::Space));

	EXPECT_TRUE(input->WasKeyPressed(sf::Keyboard::Space));
	EXPECT_TRUE(input->IsKeyHeld(sf::Keyboard::Space));

	input->BeginFrame();

	EXPECT_FALSE(input->WasKeyPressed(sf::Keyboard::Space));
	EXPECT_TRUE(input->IsKeyHeld(sf::Keyboard::Space));
}

TEST_F(InputSystemTest, TapShorterThanAFrameIsStillAPress)
{
	input->HandleEvent(KeyEvent(sf::Event::KeyPressed, sf::Keyboard::Space));
	input->HandleEvent(KeyEvent(sf::Event::KeyReleased, sf::Keyboard::Space));

	EXPECT_TRUE(input->WasKeyPressed(sf::Keyboard::Space));
	EXPECT_TRUE(input->WasKeyReleased(sf::Keyboard::Space));
	EXPECT_FALSE(input->IsKeyHeld(sf::Keyboard::Space));
}

TEST_F(InputSystemTest, ReleaseClearsHeldAndReportsReleasedOnce)
{
	input->HandleEvent(KeyEvent(sf::Event::KeyPressed, sf::Keyboard::W));
	input->BeginFrame();
	input->HandleEvent(KeyEvent(sf::Event::KeyReleased, sf::Keyboard::W));

	EXPECT_FALSE(input->IsKeyHeld(sf::Keyboard::W));
	EXPECT_TRUE(input->WasKeyReleased(sf::Keyboard::W));

	input->BeginFrame();

	EXPECT_FALSE(input->WasKeyReleased(sf::Keyboard::W));
}

TEST_F(InputSystemTest, RepeatedPressWhileHeldIsNotANewPress)
{
	input->HandleEvent(KeyEvent(sf::Event::KeyPressed, sf::Keyboard::R));
	input->BeginFrame();
	input->HandleEvent(KeyEvent(sf::Event::KeyPressed, sf::Keyboard::R));

	EXPECT_FALSE(input->WasKeyPressed(sf::Keyboard::R));
	EXPECT_TRUE(input->IsKeyHeld(sf::Keyboard::R));
}

TEST_F(InputSystemTest, MouseButtonsFollowTheSameRules)
{
	input->HandleEvent(ButtonEvent(sf::Event::MouseButtonPressed, sf::Mouse::Left));

	EXPECT_TRUE(input->WasButtonPressed(sf::Mouse::Left));
	EXPECT_TRUE(input->IsButtonHeld(sf::Mouse::Left));
	EXPECT_FALSE(input->IsButtonHeld(sf::Mouse::Right));

	input->BeginFrame();
	input->HandleEvent(ButtonEvent(sf::Event::MouseButtonReleased, sf::Mouse::Left));

	EXPECT_FALSE(input->IsButtonHeld(sf::Mouse::Left));
	EXPECT_TRUE(input->WasButtonReleased(sf::Mouse::Left));
}

TEST_F(InputSystemTest, LosingFocusForgetsHeldKeysAndMutesQueries)
{
	input->HandleEvent(KeyEvent(sf::Event::KeyPressed, sf::Keyboard::W));
	input->HandleEvent(ButtonEvent(sf::Event::MouseButtonPressed, sf::Mouse::Left));

	input->HandleEvent(FocusEvent(sf::Event::LostFocus));

	EXPECT_FALSE(input->HasFocus());
	EXPECT_FALSE(input->IsKeyHeld(sf::Keyboard::W));
	EXPECT_FALSE(input->IsButtonHeld(sf::Mouse::Left));

	input->HandleEvent(FocusEvent(sf::Event::GainedFocus));

	EXPECT_TRUE(input->HasFocus());
	EXPECT_FALSE(input->IsKeyHeld(sf::Keyboard::W));
}

TEST_F(InputSystemTest, UnknownKeyIsIgnored)
{
	input->HandleEvent(KeyEvent(sf::Event::KeyPressed, sf::Keyboard::Unknown));

	EXPECT_FALSE(input->WasKeyPressed(sf::Keyboard::Unknown));
	EXPECT_FALSE(input->IsKeyHeld(sf::Keyboard::Unknown));
}

TEST_F(InputSystemTest, UnrelatedEventsDoNothing)
{
	sf::Event resized;
	resized.type = sf::Event::Resized;
	input->HandleEvent(resized);

	EXPECT_TRUE(input->HasFocus());
	EXPECT_FALSE(input->IsKeyHeld(sf::Keyboard::W));
}
