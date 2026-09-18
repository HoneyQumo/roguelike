#pragma once

#include <SFML/Graphics/Text.hpp>
#include "UiAnchor.h"
#include "UiWidget.h"

namespace XYZEngine
{
	class UiLabel : public UiWidget
	{
	public:
		void SetFont(const sf::Font* newFont);
		void SetText(const sf::String& newText);
		void SetUtf8Text(const char* utf8Text);
		void SetCharacterSize(unsigned int size);
		void SetColor(const sf::Color& color);
		void SetOutline(float thickness, const sf::Color& color);
		void SetAlign(UiAnchor newAlign);

		const sf::String& GetText() const;
		const sf::Color& GetColor() const;
		bool IsReady() const;
		sf::Vector2f GetTextPosition() const;

	protected:
		void OnLayout() override;
		void OnDraw() const override;

	private:
		sf::Text text;
		UiAnchor align = UiAnchor::Left;
		bool isFontReady = false;

		void ApplyAlign();
	};
}
