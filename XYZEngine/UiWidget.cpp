#include "pch.h"
#include "UiWidget.h"

namespace XYZEngine
{
	void UiWidget::SetAnchor(UiAnchor newAnchor)
	{
		anchor = newAnchor;
	}
	void UiWidget::SetPivot(UiAnchor newPivot)
	{
		pivot = newPivot;
	}
	void UiWidget::SetOffset(const sf::Vector2f& newOffset)
	{
		offset = newOffset;
	}
	void UiWidget::SetSize(const sf::Vector2f& newSize)
	{
		size = newSize;
	}
	void UiWidget::SetStretch(bool newStretchX, bool newStretchY)
	{
		stretchX = newStretchX;
		stretchY = newStretchY;
	}
	void UiWidget::SetVisible(bool newIsVisible)
	{
		isVisible = newIsVisible;
	}

	bool UiWidget::IsVisible() const
	{
		return isVisible;
	}
	const sf::FloatRect& UiWidget::GetBounds() const
	{
		return bounds;
	}
	std::size_t UiWidget::GetChildrenCount() const
	{
		return children.size();
	}

	bool UiWidget::HitTest(const sf::Vector2f& point) const
	{
		return isVisible && bounds.contains(point);
	}

	bool UiWidget::HandlePointer(const sf::Vector2f& point, bool isPressed, bool wasReleased)
	{
		if (!isVisible)
		{
			return false;
		}

		for (auto child = children.rbegin(); child != children.rend(); ++child)
		{
			if ((*child)->HandlePointer(point, isPressed, wasReleased))
			{
				return true;
			}
		}

		return false;
	}

	void UiWidget::Layout(const sf::FloatRect& parentBounds)
	{
		if (parent == nullptr)
		{
			bounds = parentBounds;

			OnLayout();

			for (const auto& child : children)
			{
				child->Layout(bounds);
			}

			return;
		}

		sf::Vector2f anchorRatio = AnchorRatio(anchor);
		sf::Vector2f pivotRatio = AnchorRatio(pivot);

		if (stretchX)
		{
			bounds.left = parentBounds.left + offset.x;
			bounds.width = parentBounds.width - 2.f * offset.x;
		}
		else
		{
			bounds.width = size.x;
			bounds.left = parentBounds.left + parentBounds.width * anchorRatio.x + offset.x - size.x * pivotRatio.x;
		}

		if (stretchY)
		{
			bounds.top = parentBounds.top + offset.y;
			bounds.height = parentBounds.height - 2.f * offset.y;
		}
		else
		{
			bounds.height = size.y;
			bounds.top = parentBounds.top + parentBounds.height * anchorRatio.y + offset.y - size.y * pivotRatio.y;
		}

		OnLayout();

		for (const auto& child : children)
		{
			child->Layout(bounds);
		}
	}

	void UiWidget::Draw() const
	{
		if (!isVisible)
		{
			return;
		}

		OnDraw();

		for (const auto& child : children)
		{
			child->Draw();
		}
	}
}
