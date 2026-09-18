#include "pch.h"
#include "UiLabel.h"
#include "LoggerRegistry.h"
#include "RenderSystem.h"
#include "TextUtils.h"

namespace XYZEngine
{
	void UiLabel::SetFont(const sf::Font* newFont)
	{
		if (newFont == nullptr)
		{
			isFontReady = false;
			LOG_ERROR("Ui label has no font");
			return;
		}

		text.setFont(*newFont);
		isFontReady = true;
		ApplyAlign();
	}

	void UiLabel::SetText(const sf::String& newText)
	{
		if (text.getString() == newText)
		{
			return;
		}

		text.setString(newText);
		ApplyAlign();
	}

	void UiLabel::SetUtf8Text(const char* utf8Text)
	{
		SetText(FromUtf8(utf8Text));
	}

	void UiLabel::SetCharacterSize(unsigned int size)
	{
		text.setCharacterSize(size);
		ApplyAlign();
	}

	void UiLabel::SetColor(const sf::Color& color)
	{
		text.setFillColor(color);
	}

	void UiLabel::SetOutline(float thickness, const sf::Color& color)
	{
		text.setOutlineThickness(thickness);
		text.setOutlineColor(color);
	}

	void UiLabel::SetAlign(UiAnchor newAlign)
	{
		align = newAlign;
		ApplyAlign();
	}

	const sf::String& UiLabel::GetText() const
	{
		return text.getString();
	}

	const sf::Color& UiLabel::GetColor() const
	{
		return text.getFillColor();
	}

	bool UiLabel::IsReady() const
	{
		return isFontReady;
	}

	sf::Vector2f UiLabel::GetTextPosition() const
	{
		return text.getPosition();
	}

	void UiLabel::OnLayout()
	{
		ApplyAlign();
	}

	void UiLabel::ApplyAlign()
	{
		if (!isFontReady)
		{
			return;
		}

		sf::FloatRect local = text.getLocalBounds();
		sf::Vector2f ratio = AnchorRatio(align);

		text.setOrigin(local.left + local.width * ratio.x, local.top + local.height * ratio.y);
		text.setPosition(AnchorPoint(GetBounds(), align));
	}

	void UiLabel::OnDraw() const
	{
		if (!isFontReady)
		{
			return;
		}

		RenderSystem::Instance()->Render(text);
	}
}
