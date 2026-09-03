#include "MessageOverlayComponent.h"
#include "GameSettings.h"
#include "TextUtils.h"
#include <RenderSystem.h>
#include <ResourceSystem.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    MessageOverlayComponent::MessageOverlayComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        auto font = XYZEngine::ResourceSystem::Instance()->GetFont(HUD_FONT);
        if (font == nullptr)
        {
            LOG_ERROR("Message overlay has no font");
            return;
        }

        background.setFillColor(OVERLAY_BACKGROUND_COLOR);

        titleText.setFont(*font);
        titleText.setCharacterSize(OVERLAY_TITLE_FONT_SIZE);
        titleText.setFillColor(AMMO_HUD_COLOR);
        titleText.setOutlineColor(AMMO_HUD_OUTLINE_COLOR);
        titleText.setOutlineThickness(AMMO_HUD_OUTLINE);

        hintText.setFont(*font);
        hintText.setCharacterSize(OVERLAY_HINT_FONT_SIZE);
        hintText.setFillColor(AMMO_HUD_COLOR);
        hintText.setOutlineColor(AMMO_HUD_OUTLINE_COLOR);
        hintText.setOutlineThickness(AMMO_HUD_OUTLINE);

        isFontReady = true;
    }

    void MessageOverlayComponent::Update(float deltaTime)
    {
    }

    void MessageOverlayComponent::Render()
    {
        if (!isShown || !isFontReady)
        {
            return;
        }

        auto& window = XYZEngine::RenderSystem::Instance()->GetMainWindow();
        sf::View worldView = window.getView();
        window.setView(window.getDefaultView());

        sf::Vector2f screenSize = window.getDefaultView().getSize();
        sf::Vector2f center = screenSize * 0.5f;

        background.setSize(screenSize);
        titleText.setPosition(center.x, center.y - OVERLAY_LINE_GAP);
        hintText.setPosition(center.x, center.y + OVERLAY_LINE_GAP);

        XYZEngine::RenderSystem::Instance()->Render(background);
        XYZEngine::RenderSystem::Instance()->Render(titleText);
        XYZEngine::RenderSystem::Instance()->Render(hintText);

        window.setView(worldView);
    }

    void MessageOverlayComponent::Show(const char* title, const char* hint)
    {
        titleText.setString(FromUtf8(title));
        hintText.setString(FromUtf8(hint));
        CenterOrigin(titleText);
        CenterOrigin(hintText);

        isShown = true;
    }

    void MessageOverlayComponent::Hide()
    {
        isShown = false;
    }

    bool MessageOverlayComponent::IsShown() const
    {
        return isShown;
    }

    void MessageOverlayComponent::CenterOrigin(sf::Text& text)
    {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
    }
}
