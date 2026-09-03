#include "pch.h"
#include "InputSystem.h"

namespace XYZEngine
{
	InputSystem* InputSystem::Instance()
	{
		static InputSystem input;
		return &input;
	}

	void InputSystem::BeginFrame()
	{
		keysPressed.reset();
		keysReleased.reset();
		buttonsPressed.reset();
		buttonsReleased.reset();
	}

	void InputSystem::HandleEvent(const sf::Event& event)
	{
		switch (event.type)
		{
		case sf::Event::KeyPressed:
			if (IsValid(event.key.code) && !keysHeld[event.key.code])
			{
				keysHeld[event.key.code] = true;
				keysPressed[event.key.code] = true;
			}
			break;
		case sf::Event::KeyReleased:
			if (IsValid(event.key.code))
			{
				keysHeld[event.key.code] = false;
				keysReleased[event.key.code] = true;
			}
			break;
		case sf::Event::MouseButtonPressed:
			if (IsValid(event.mouseButton.button) && !buttonsHeld[event.mouseButton.button])
			{
				buttonsHeld[event.mouseButton.button] = true;
				buttonsPressed[event.mouseButton.button] = true;
			}
			break;
		case sf::Event::MouseButtonReleased:
			if (IsValid(event.mouseButton.button))
			{
				buttonsHeld[event.mouseButton.button] = false;
				buttonsReleased[event.mouseButton.button] = true;
			}
			break;
		case sf::Event::LostFocus:
			Reset();
			hasFocus = false;
			break;
		case sf::Event::GainedFocus:
			hasFocus = true;
			SyncHeldWithDevice();
			break;
		default:
			break;
		}
	}

	void InputSystem::Reset()
	{
		keysHeld.reset();
		keysPressed.reset();
		keysReleased.reset();
		buttonsHeld.reset();
		buttonsPressed.reset();
		buttonsReleased.reset();
	}

	bool InputSystem::HasFocus() const
	{
		return hasFocus;
	}

	bool InputSystem::IsKeyHeld(sf::Keyboard::Key key) const
	{
		return hasFocus && IsValid(key) && keysHeld[key];
	}
	bool InputSystem::WasKeyPressed(sf::Keyboard::Key key) const
	{
		return hasFocus && IsValid(key) && keysPressed[key];
	}
	bool InputSystem::WasKeyReleased(sf::Keyboard::Key key) const
	{
		return hasFocus && IsValid(key) && keysReleased[key];
	}

	bool InputSystem::IsButtonHeld(sf::Mouse::Button button) const
	{
		return hasFocus && IsValid(button) && buttonsHeld[button];
	}
	bool InputSystem::WasButtonPressed(sf::Mouse::Button button) const
	{
		return hasFocus && IsValid(button) && buttonsPressed[button];
	}
	bool InputSystem::WasButtonReleased(sf::Mouse::Button button) const
	{
		return hasFocus && IsValid(button) && buttonsReleased[button];
	}

	void InputSystem::SyncHeldWithDevice()
	{
		for (int key = 0; key < sf::Keyboard::KeyCount; key++)
		{
			keysHeld[key] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(key));
		}
		for (int button = 0; button < sf::Mouse::ButtonCount; button++)
		{
			buttonsHeld[button] = sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(button));
		}
	}

	bool InputSystem::IsValid(sf::Keyboard::Key key)
	{
		return key >= 0 && key < sf::Keyboard::KeyCount;
	}
	bool InputSystem::IsValid(sf::Mouse::Button button)
	{
		return button >= 0 && button < sf::Mouse::ButtonCount;
	}
}
