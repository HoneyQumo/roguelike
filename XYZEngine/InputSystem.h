#pragma once

#include <array>
#include <bitset>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace XYZEngine
{
	enum class InputAction
	{
		MoveUp,
		MoveDown,
		MoveLeft,
		MoveRight,
		Attack,
		HeavyAttack,
		Run,
		Reload,
		Roll,
		Interact,
		Inventory,
		Pause,
		WeaponSlot1,
		WeaponSlot2,
		WeaponSlot3,
		Count
	};

	constexpr int INPUT_ACTIONS_COUNT = static_cast<int>(InputAction::Count);

	struct InputBinding
	{
		sf::Keyboard::Key key = sf::Keyboard::Unknown;
		sf::Keyboard::Key alternativeKey = sf::Keyboard::Unknown;
		sf::Mouse::Button button = sf::Mouse::ButtonCount;
	};

	using InputBindings = std::array<InputBinding, INPUT_ACTIONS_COUNT>;

	InputBindings GetDefaultBindings();

	class InputSystem
	{
	public:
		static InputSystem* Instance();

		void BeginFrame();
		void HandleEvent(const sf::Event& event);
		void SyncWithDevice();
		void Reset();

		bool HasFocus() const;

		void SetBinding(InputAction action, const InputBinding& binding);
		const InputBinding& GetBinding(InputAction action) const;

		bool IsActionHeld(InputAction action) const;
		bool WasActionPressed(InputAction action) const;

		void SetMousePosition(sf::Vector2i newMousePosition);
		sf::Vector2i GetMousePosition() const;

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
		sf::Vector2i mousePosition = {0, 0};
		InputBindings bindings = GetDefaultBindings();

		InputSystem() {}
		~InputSystem() {}

		InputSystem(InputSystem const&) = delete;
		InputSystem& operator=(InputSystem const&) = delete;

		static bool IsValid(sf::Keyboard::Key key);
		static bool IsValid(sf::Mouse::Button button);
	};
}
