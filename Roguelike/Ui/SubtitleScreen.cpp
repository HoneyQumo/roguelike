#include "SubtitleScreen.h"
#include "GameSettings.h"
#include <ResourceSystem.h>
#include <TextUtils.h>
#include <TextWrap.h>

namespace RoguelikeGame
{
	namespace
	{
		// Строк на экране не больше, чем в очереди, но каждая может занять
		// несколько экранных: длинную фразу переносит по ширине.
		constexpr std::size_t MAX_LABELS = SUBTITLE_MAX_LINES * 3;

		float LineHeight()
		{
			return SUBTITLE_FONT_SIZE * AMMO_HUD_LINE_HEIGHT + SUBTITLE_LINE_GAP;
		}
	}

	SubtitleScreen::SubtitleScreen()
	{
		const sf::Font* font = XYZEngine::ResourceSystem::Instance()->GetFont(HUD_FONT);

		for (std::size_t index = 0; index < MAX_LABELS; index++)
		{
			auto label = GetRoot().AddChild<XYZEngine::UiLabel>();
			label->SetAnchor(XYZEngine::UiAnchor::Bottom);
			label->SetPivot(XYZEngine::UiAnchor::Bottom);
			label->SetSize({SUBTITLE_WIDTH, SUBTITLE_FONT_SIZE * AMMO_HUD_LINE_HEIGHT});
			label->SetAlign(XYZEngine::UiAnchor::Center);
			label->SetCharacterSize(SUBTITLE_FONT_SIZE);
			label->SetColor(AMMO_HUD_COLOR);
			label->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
			label->SetFont(font);
			label->SetVisible(false);

			labels.push_back(label);
		}
	}

	void SubtitleScreen::SetQueue(const SubtitleQueue* newQueue)
	{
		queue = newQueue;
	}

	void SubtitleScreen::Update(float deltaTime)
	{
		Fill();
	}

	int SubtitleScreen::GetShownLineCount() const
	{
		return shownLineCount;
	}

	void SubtitleScreen::Fill()
	{
		const sf::Font* font = XYZEngine::ResourceSystem::Instance()->GetFont(HUD_FONT);

		std::vector<sf::String> screenLines;
		if (queue != nullptr && font != nullptr)
		{
			for (const Subtitle& said : queue->GetLines())
			{
				sf::String whole = XYZEngine::FromUtf8(said.speaker.c_str()) + ": "
					+ XYZEngine::FromUtf8(said.text.c_str());

				for (const sf::String& piece : XYZEngine::WrapText(whole, *font, SUBTITLE_FONT_SIZE, SUBTITLE_WIDTH))
				{
					screenLines.push_back(piece);
				}
			}
		}

		// Снизу вверх: последнее сказанное ближе к низу экрана, где взгляд.
		shownLineCount = 0;
		for (std::size_t index = 0; index < labels.size(); index++)
		{
			bool isUsed = index < screenLines.size();
			labels[index]->SetVisible(isUsed);

			if (!isUsed)
			{
				continue;
			}

			std::size_t fromBottom = screenLines.size() - 1 - index;

			labels[index]->SetText(screenLines[index]);
			labels[index]->SetOffset({0.f, -SUBTITLE_MARGIN_Y - LineHeight() * static_cast<float>(fromBottom)});
			shownLineCount++;
		}
	}
}
