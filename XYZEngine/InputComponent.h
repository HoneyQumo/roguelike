#pragma once

#include "Component.h"
#include "Vector.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

namespace XYZEngine
{
	constexpr int NO_WEAPON_SLOT = -1;
	constexpr int DIGIT_KEYS_COUNT = 9;

	struct InputBindings
	{
		sf::Keyboard::Key moveUp = sf::Keyboard::W;
		sf::Keyboard::Key moveDown = sf::Keyboard::S;
		sf::Keyboard::Key moveLeft = sf::Keyboard::A;
		sf::Keyboard::Key moveRight = sf::Keyboard::D;
		sf::Keyboard::Key run = sf::Keyboard::LShift;
		sf::Keyboard::Key runAlternative = sf::Keyboard::RShift;
		sf::Keyboard::Key reload = sf::Keyboard::R;
		sf::Keyboard::Key roll = sf::Keyboard::Space;
		sf::Mouse::Button attack = sf::Mouse::Left;
		sf::Mouse::Button heavyAttack = sf::Mouse::Right;
		sf::Keyboard::Key digits[DIGIT_KEYS_COUNT] = {
			sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3,
			sf::Keyboard::Num4, sf::Keyboard::Num5, sf::Keyboard::Num6,
			sf::Keyboard::Num7, sf::Keyboard::Num8, sf::Keyboard::Num9
		};
	};

	class InputComponent : public Component
	{
	public:
		InputComponent(GameObject* gameObject);

		void Update(float deltaTime) override;
		void Render() override;

		void SetBindings(const InputBindings& newBindings);
		const InputBindings& GetBindings() const;

		float GetHorizontalAxis() const;
		float GetVerticalAxis() const;

		bool IsAttackPressed() const;
		bool IsHeavyAttackPressed() const;
		bool IsRunPressed() const;
		bool IsReloadPressed() const;
		bool WasRollJustPressed() const;
		int GetSelectedWeaponSlot() const;
		Vector2Df GetMouseWorldPosition() const;
	private:
		InputBindings bindings;

		float horizontalAxis = 0.f;
		float verticalAxis = 0.f;
		bool isAttackPressed = false;
		bool isHeavyAttackPressed = false;
		bool isRunPressed = false;
		bool isReloadPressed = false;
		bool isRollJustPressed = false;
		int selectedWeaponSlot = NO_WEAPON_SLOT;
		Vector2Df mouseWorldPosition = { 0.f, 0.f };
	};
}
