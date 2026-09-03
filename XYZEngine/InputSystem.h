#pragma once

#include <bitset>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace XYZEngine
{
	class InputSystem
	{
	public:
		static InputSystem* Instance();

		void BeginFrame();
		void HandleEvent(const sf::Event& event);
		void Reset();

		bool HasFocus() const;

		bool IsKeyHeld(sf::Keyboard::Key key) const;
		bool WasKeyPressed(sf::Keyboard::Key key) const;
		bool WasKeyReleased(sf::Keyboard::Key key) const;

		bool IsButtonHeld(sf::Mouse::Button button) const;
		bool WasButtonPressed(sf::Mouse::Button button) const;
		bool WasButtonReleased(sf::Mouse::Button button) const;

	private:
		std::bitset<sf::Keyboard::KeyCount> keysHeld;
		std::bitset<sf::Keyboard::KeyCount> keysPressed;
		std::bitset<sf::Keyboard::KeyCount> keysReleased;

		std::bitset<sf::Mouse::ButtonCount> buttonsHeld;
		std::bitset<sf::Mouse::ButtonCount> buttonsPressed;
		std::bitset<sf::Mouse::ButtonCount> buttonsReleased;

		bool hasFocus = true;

		InputSystem() {}
		~InputSystem() {}

		InputSystem(InputSystem const&) = delete;
		InputSystem& operator=(InputSystem const&) = delete;

		void SyncHeldWithDevice();

		static bool IsValid(sf::Keyboard::Key key);
		static bool IsValid(sf::Mouse::Button button);
	};
}
