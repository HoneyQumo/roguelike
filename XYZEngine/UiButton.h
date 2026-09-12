#pragma once

#include <functional>
#include "UiLabel.h"
#include "UiPanel.h"
#include "UiWidget.h"

namespace XYZEngine
{
	class UiButton : public UiWidget
	{
	public:
		enum class State
		{
			Normal,
			Hovered,
			Pressed
		};

		UiButton();

		UiLabel* GetLabel() const;
		State GetState() const;

		void SetColors(const sf::Color& normal, const sf::Color& hovered, const sf::Color& pressed);
		void SetOnClick(std::function<void()> newOnClick);

		bool HandlePointer(const sf::Vector2f& point, bool isPressed, bool wasReleased);

	private:
		UiPanel* background = nullptr;
		UiLabel* label = nullptr;

		State state = State::Normal;
		sf::Color normalColor = sf::Color(60, 60, 60);
		sf::Color hoveredColor = sf::Color(90, 90, 90);
		sf::Color pressedColor = sf::Color(40, 40, 40);

		std::function<void()> onClick;

		void SetState(State newState);
	};
}
