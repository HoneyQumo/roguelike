#include "pch.h"
#include "InputSystem.h"

namespace XYZEngine
{
	InputBindings GetDefaultBindings()
	{
		InputBindings bindings;
		bindings[static_cast<int>(InputAction::MoveUp)].key = sf::Keyboard::W;
		bindings[static_cast<int>(InputAction::MoveDown)].key = sf::Keyboard::S;
		bindings[static_cast<int>(InputAction::MoveLeft)].key = sf::Keyboard::A;
		bindings[static_cast<int>(InputAction::MoveRight)].key = sf::Keyboard::D;
		bindings[static_cast<int>(InputAction::Attack)].button = sf::Mouse::Left;
		bindings[static_cast<int>(InputAction::HeavyAttack)].button = sf::Mouse::Right;
		bindings[static_cast<int>(InputAction::Run)].key = sf::Keyboard::LShift;
		bindings[static_cast<int>(InputAction::Run)].alternativeKey = sf::Keyboard::RShift;
		bindings[static_cast<int>(InputAction::Reload)].key = sf::Keyboard::R;
		bindings[static_cast<int>(InputAction::Roll)].key = sf::Keyboard::Space;
		bindings[static_cast<int>(InputAction::Interact)].key = sf::Keyboard::E;
		bindings[static_cast<int>(InputAction::Inventory)].key = sf::Keyboard::I;
		bindings[static_cast<int>(InputAction::Inventory)].alternativeKey = sf::Keyboard::Tab;
		bindings[static_cast<int>(InputAction::Pause)].key = sf::Keyboard::Escape;
		bindings[static_cast<int>(InputAction::WeaponSlot1)].key = sf::Keyboard::Num1;
		bindings[static_cast<int>(InputAction::WeaponSlot2)].key = sf::Keyboard::Num2;
		bindings[static_cast<int>(InputAction::WeaponSlot3)].key = sf::Keyboard::Num3;

		return bindings;
	}

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
		case sf::Event::MouseMoved:
			mousePosition = {event.mouseMove.x, event.mouseMove.y};
			break;
		case sf::Event::MouseButtonPressed:
			mousePosition = {event.mouseButton.x, event.mouseButton.y};
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
			break;
		default:
			break;
		}
	}

	void InputSystem::Reset()
	{
		hasFocus = true;
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

	void InputSystem::SetBinding(InputAction action, const InputBinding& binding)
	{
		bindings[static_cast<int>(action)] = binding;
	}
	const InputBinding& InputSystem::GetBinding(InputAction action) const
	{
		return bindings[static_cast<int>(action)];
	}

	bool InputSystem::IsActionHeld(InputAction action) const
	{
		const InputBinding& binding = GetBinding(action);
		return IsKeyHeld(binding.key) || IsKeyHeld(binding.alternativeKey) || IsButtonHeld(binding.button);
	}
	bool InputSystem::WasActionPressed(InputAction action) const
	{
		const InputBinding& binding = GetBinding(action);
		return WasKeyPressed(binding.key) || WasKeyPressed(binding.alternativeKey) || WasButtonPressed(binding.button);
	}

	void InputSystem::SetMousePosition(sf::Vector2i newMousePosition)
	{
		mousePosition = newMousePosition;
	}
	sf::Vector2i InputSystem::GetMousePosition() const
	{
		return mousePosition;
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

	void InputSystem::SyncWithDevice()
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
