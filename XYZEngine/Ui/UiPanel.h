#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include "UiWidget.h"

namespace XYZEngine
{
	class UiPanel : public UiWidget
	{
	public:
		void SetFillColor(const sf::Color& color);
		void SetOutline(float thickness, const sf::Color& color);

		const sf::RectangleShape& GetShape() const;

	protected:
		void OnLayout() override;
		void OnDraw() const override;

	private:
		sf::RectangleShape shape;
	};
}
