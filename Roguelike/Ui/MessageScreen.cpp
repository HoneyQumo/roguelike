#include "MessageScreen.h"
#include "GameSettings.h"
#include <ResourceSystem.h>

namespace RoguelikeGame
{
    MessageScreen::MessageScreen()
    {
        const sf::Font* font = XYZEngine::ResourceSystem::Instance()->GetFont(HUD_FONT);

        background = GetRoot().AddChild<XYZEngine::UiPanel>();
        background->SetStretch(true, true);
        background->SetFillColor(OVERLAY_BACKGROUND_COLOR);

        titleLabel = GetRoot().AddChild<XYZEngine::UiLabel>();
        titleLabel->SetAnchor(XYZEngine::UiAnchor::Center);
        titleLabel->SetPivot(XYZEngine::UiAnchor::Center);
        titleLabel->SetOffset({0.f, -OVERLAY_LINE_GAP});
        titleLabel->SetSize({OVERLAY_LINE_WIDTH, OVERLAY_TITLE_FONT_SIZE * AMMO_HUD_LINE_HEIGHT});
        titleLabel->SetAlign(XYZEngine::UiAnchor::Center);
        titleLabel->SetCharacterSize(OVERLAY_TITLE_FONT_SIZE);
        titleLabel->SetColor(AMMO_HUD_COLOR);
        titleLabel->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        titleLabel->SetFont(font);

        hintLabel = GetRoot().AddChild<XYZEngine::UiLabel>();
        hintLabel->SetAnchor(XYZEngine::UiAnchor::Center);
        hintLabel->SetPivot(XYZEngine::UiAnchor::Center);
        hintLabel->SetOffset({0.f, OVERLAY_LINE_GAP});
        hintLabel->SetSize({OVERLAY_LINE_WIDTH, OVERLAY_HINT_FONT_SIZE * AMMO_HUD_LINE_HEIGHT});
        hintLabel->SetAlign(XYZEngine::UiAnchor::Center);
        hintLabel->SetCharacterSize(OVERLAY_HINT_FONT_SIZE);
        hintLabel->SetColor(AMMO_HUD_COLOR);
        hintLabel->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        hintLabel->SetFont(font);

        SetVisible(false);
    }

    void MessageScreen::Show(const char* title, const char* hint)
    {
        titleLabel->SetUtf8Text(title);
        hintLabel->SetUtf8Text(hint);

        SetVisible(true);
    }

    void MessageScreen::Hide()
    {
        SetVisible(false);
    }

    bool MessageScreen::IsShown() const
    {
        return IsVisible();
    }
}
