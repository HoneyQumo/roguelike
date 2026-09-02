#include "pch.h"
#include "InputComponent.h"
#include "RenderSystem.h"

namespace XYZEngine
{
	InputComponent::InputComponent(GameObject* gameObject) : Component(gameObject) {}

	void InputComponent::Update(float deltaTime)
	{
		verticalAxis = 0.f;
		horizontalAxis = 0.f;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			verticalAxis += 1.0f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			verticalAxis -= 1.0f;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			horizontalAxis += 1.0f;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			horizontalAxis -= 1.0f;
		}

		isAttackPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
		isHeavyAttackPressed = sf::Mouse::isButtonPressed(sf::Mouse::Right);
		isRunPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
		isReloadPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::R);
		isRollPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

		selectedWeaponSlot = NO_WEAPON_SLOT;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
		{
			selectedWeaponSlot = 0;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
		{
			selectedWeaponSlot = 1;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
		{
			selectedWeaponSlot = 2;
		}

		auto& window = RenderSystem::Instance()->GetMainWindow();
		auto worldPosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		mouseWorldPosition = Convert<Vector2Df, sf::Vector2f>(worldPosition);
	}
	void InputComponent::Render()
	{

	}

	float InputComponent::GetHorizontalAxis() const
	{
		return horizontalAxis;
	}
	float InputComponent::GetVerticalAxis() const
	{
		return verticalAxis;
	}

	bool InputComponent::IsAttackPressed() const
	{
		return isAttackPressed;
	}
	bool InputComponent::IsHeavyAttackPressed() const
	{
		return isHeavyAttackPressed;
	}
	bool InputComponent::IsRunPressed() const
	{
		return isRunPressed;
	}
	bool InputComponent::IsReloadPressed() const
	{
		return isReloadPressed;
	}
	bool InputComponent::IsRollPressed() const
	{
		return isRollPressed;
	}
	int InputComponent::GetSelectedWeaponSlot() const
	{
		return selectedWeaponSlot;
	}
	Vector2Df InputComponent::GetMouseWorldPosition() const
	{
		return mouseWorldPosition;
	}
}