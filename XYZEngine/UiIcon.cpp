#include "pch.h"
#include "UiIcon.h"
#include "RenderSystem.h"

namespace XYZEngine
{
	void UiIcon::SetTexture(const sf::Texture* newTexture)
	{
		texture = newTexture;
		if (texture == nullptr)
		{
			return;
		}

		sprite.setTexture(*texture, true);
	}

	void UiIcon::SetTextureRect(const sf::IntRect& rect)
	{
		sprite.setTextureRect(rect);
	}

	void UiIcon::SetColor(const sf::Color& color)
	{
		sprite.setColor(color);
	}

	void UiIcon::SetAdditiveBlending(bool newIsAdditiveBlending)
	{
		isAdditiveBlending = newIsAdditiveBlending;
	}

	bool UiIcon::HasTexture() const
	{
		return texture != nullptr;
	}

	void UiIcon::OnLayout()
	{
		const sf::FloatRect& bounds = GetBounds();
		sf::IntRect frame = sprite.getTextureRect();

		float scaleX = frame.width > 0 ? bounds.width / static_cast<float>(frame.width) : 1.f;
		float scaleY = frame.height > 0 ? bounds.height / static_cast<float>(frame.height) : 1.f;

		sprite.setScale(scaleX, scaleY);
		sprite.setPosition(bounds.left, bounds.top);
	}

	void UiIcon::OnDraw() const
	{
		if (texture == nullptr)
		{
			return;
		}

		if (isAdditiveBlending)
		{
			RenderSystem::Instance()->Render(sprite, sf::RenderStates(sf::BlendAdd));
			return;
		}

		RenderSystem::Instance()->Render(sprite);
	}
}
