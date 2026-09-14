#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include "UiWidget.h"

namespace XYZEngine
{
	class UiProgressBar : public UiWidget
	{
	public:
		void SetValue(float newValue);
		float GetValue() const;

		void SetColors(const sf::Color& fill, const sf::Color& background);
		sf::FloatRect GetFillBounds() const;
		const sf::Color& GetFillColor() const;

	protected:
		void OnLayout() override;
		void OnDraw() const override;

	private:
		sf::RectangleShape backgroundShape;
		sf::RectangleShape fillShape;
		float value = 1.f;

		void ApplyFill();
	};
}
