#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include "UiWidget.h"

namespace XYZEngine
{
	class UiIcon : public UiWidget
	{
	public:
		void SetTexture(const sf::Texture* newTexture);
		void SetTextureRect(const sf::IntRect& rect);
		void SetColor(const sf::Color& color);
		void SetAdditiveBlending(bool newIsAdditiveBlending);
		void SetKeepAspect(bool newKeepAspect);

		bool HasTexture() const;

	protected:
		void OnLayout() override;
		void OnDraw() const override;

	private:
		sf::Sprite sprite;
		const sf::Texture* texture = nullptr;
		bool isAdditiveBlending = false;
		bool keepAspect = true;
	};
}
