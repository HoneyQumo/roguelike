#pragma once

#include "UiWidget.h"

namespace XYZEngine
{
	class UiScreen
	{
	public:
		virtual ~UiScreen() = default;

		UiWidget& GetRoot();
		const UiWidget& GetRoot() const;

		void Resize(const sf::Vector2f& screenSize);
		void Draw() const;

		void SetVisible(bool isVisible);
		bool IsVisible() const;

		virtual void Update(float deltaTime) {}

	private:
		UiWidget root;
	};
}
