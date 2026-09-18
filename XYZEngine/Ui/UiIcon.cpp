#include "pch.h"
#include "UiIcon.h"
#include "RenderSystem.h"
#include <algorithm>

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

		FitToBounds();
	}

	void UiIcon::SetTextureRect(const sf::IntRect& rect)
	{
		sprite.setTextureRect(rect);

		FitToBounds();
	}

	// Новая картинка - новые пропорции. Без пересчёта спрайт остаётся в масштабе прежней
	// и вылезает за рамку: раскладку виджетов никто не трогает, пока не сменится размер окна.
	void UiIcon::FitToBounds()
	{
		if (GetBounds().width > 0.f && GetBounds().height > 0.f)
		{
			OnLayout();
		}
	}

	void UiIcon::SetColor(const sf::Color& color)
	{
		sprite.setColor(color);
	}

	void UiIcon::SetAdditiveBlending(bool newIsAdditiveBlending)
	{
		isAdditiveBlending = newIsAdditiveBlending;
	}

	void UiIcon::SetKeepAspect(bool newKeepAspect)
	{
		keepAspect = newKeepAspect;
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

		if (keepAspect)
		{
			float scale = std::min(scaleX, scaleY);
			scaleX = scale;
			scaleY = scale;
		}

		float width = frame.width * scaleX;
		float height = frame.height * scaleY;

		sprite.setScale(scaleX, scaleY);
		sprite.setPosition(bounds.left + 0.5f * (bounds.width - width), bounds.top + 0.5f * (bounds.height - height));
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
