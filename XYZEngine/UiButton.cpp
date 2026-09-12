#include "pch.h"
#include "UiButton.h"

namespace XYZEngine
{
	UiButton::UiButton()
	{
		background = AddChild<UiPanel>();
		background->SetStretch(true, true);
		background->SetFillColor(normalColor);

		label = AddChild<UiLabel>();
		label->SetStretch(true, true);
		label->SetAlign(UiAnchor::Center);
	}

	UiLabel* UiButton::GetLabel() const
	{
		return label;
	}

	UiButton::State UiButton::GetState() const
	{
		return state;
	}

	void UiButton::SetColors(const sf::Color& normal, const sf::Color& hovered, const sf::Color& pressed)
	{
		normalColor = normal;
		hoveredColor = hovered;
		pressedColor = pressed;

		SetState(state);
	}

	void UiButton::SetOnClick(std::function<void()> newOnClick)
	{
		onClick = std::move(newOnClick);
	}

	bool UiButton::HandlePointer(const sf::Vector2f& point, bool isPressed, bool wasReleased)
	{
		if (!IsVisible())
		{
			return false;
		}

		if (!HitTest(point))
		{
			SetState(State::Normal);
			return false;
		}

		if (isPressed)
		{
			SetState(State::Pressed);
			return true;
		}

		bool isClick = wasReleased && state == State::Pressed;
		SetState(State::Hovered);

		if (isClick && onClick != nullptr)
		{
			onClick();
		}

		return true;
	}

	void UiButton::SetState(State newState)
	{
		state = newState;

		switch (state)
		{
		case State::Hovered:
			background->SetFillColor(hoveredColor);
			break;
		case State::Pressed:
			background->SetFillColor(pressedColor);
			break;
		default:
			background->SetFillColor(normalColor);
			break;
		}
	}
}
