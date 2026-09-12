#pragma once

#include <memory>
#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include "UiAnchor.h"

namespace XYZEngine
{
	class UiWidget
	{
	public:
		UiWidget() = default;
		virtual ~UiWidget() = default;

		UiWidget(const UiWidget&) = delete;
		UiWidget& operator=(const UiWidget&) = delete;

		template <typename T>
		T* AddChild()
		{
			static_assert(std::is_base_of<UiWidget, T>::value, "T must be derived from UiWidget");

			auto child = std::make_unique<T>();
			T* rawChild = child.get();
			rawChild->parent = this;
			children.push_back(std::move(child));

			return rawChild;
		}

		void SetAnchor(UiAnchor newAnchor);
		void SetPivot(UiAnchor newPivot);
		void SetOffset(const sf::Vector2f& newOffset);
		void SetSize(const sf::Vector2f& newSize);
		void SetStretch(bool newStretchX, bool newStretchY);
		void SetVisible(bool newIsVisible);

		bool IsVisible() const;
		const sf::FloatRect& GetBounds() const;
		std::size_t GetChildrenCount() const;

		bool HitTest(const sf::Vector2f& point) const;
		virtual bool HandlePointer(const sf::Vector2f& point, bool isPressed, bool wasReleased);

		void Layout(const sf::FloatRect& parentBounds);
		void Draw() const;

	protected:
		virtual void OnLayout() {}
		virtual void OnDraw() const {}

	private:
		std::vector<std::unique_ptr<UiWidget>> children;
		UiWidget* parent = nullptr;

		UiAnchor anchor = UiAnchor::TopLeft;
		UiAnchor pivot = UiAnchor::TopLeft;
		sf::Vector2f offset = {0.f, 0.f};
		sf::Vector2f size = {0.f, 0.f};
		bool stretchX = false;
		bool stretchY = false;
		bool isVisible = true;

		sf::FloatRect bounds;
	};
}
