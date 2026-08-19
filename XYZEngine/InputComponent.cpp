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
	Vector2Df InputComponent::GetMouseWorldPosition() const
	{
		return mouseWorldPosition;
	}
}