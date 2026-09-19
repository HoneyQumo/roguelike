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

		// Пересчёт по требованию: если с прошлого кадра ничего не двигали,
		// ходить по дереву незачем.
		void Relayout(const sf::Vector2f& screenSize);
		void Draw() const;
		virtual bool HandlePointer(const sf::Vector2f& point, bool isPressed, bool wasReleased);

		void SetVisible(bool isVisible);
		bool IsVisible() const;

		virtual void Update(float deltaTime) {}

	private:
		UiWidget root;
	};
}
