#include "ThreatMarkComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <RenderSystem.h>
#include <ResourceSystem.h>

namespace RoguelikeGame
{
    ThreatMarkComponent::ThreatMarkComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void ThreatMarkComponent::Start()
    {
        const sf::Font* font = XYZEngine::ResourceSystem::Instance()->GetFont(HUD_FONT);
        if (font == nullptr)
        {
            LOG_ERROR("Threat marks need the hud font");
            return;
        }

        hasFont = true;
        label.setFont(*font);
        label.setCharacterSize(THREAT_MARK_FONT_SIZE);
        label.setOutlineThickness(THREAT_MARK_OUTLINE);
        label.setOutlineColor(THREAT_MARK_OUTLINE_COLOR);

        // Мировой вид перевёрнут по оси Y, поэтому текст надо отразить обратно -
        // иначе знак рисуется вверх ногами.
        label.setScale(1.f, -1.f);
    }

    const char* ThreatMarkComponent::GlyphOf(ThreatKind kind)
    {
        return IsAlarming(kind) ? "!" : "?";
    }

    sf::Color ThreatMarkComponent::ColorOf(ThreatKind kind, float strength)
    {
        sf::Color color = IsAlarming(kind) ? THREAT_PROVOKED_COLOR : THREAT_ALERTED_COLOR;
        color.a = static_cast<sf::Uint8>(255.f * std::clamp(strength, 0.f, 1.f));

        return color;
    }

    void ThreatMarkComponent::SetMarks(const std::vector<ThreatMark>& newMarks)
    {
        marks = newMarks;
    }

    const std::vector<ThreatMark>& ThreatMarkComponent::GetMarks() const
    {
        return marks;
    }

    void ThreatMarkComponent::Render()
    {
        if (!hasFont)
        {
            return;
        }

        for (const ThreatMark& mark : marks)
        {
            label.setString(GlyphOf(mark.kind));

            sf::FloatRect bounds = label.getLocalBounds();
            label.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
            label.setPosition(mark.position.x, mark.position.y);

            sf::Color color = ColorOf(mark.kind, mark.strength);
            label.setFillColor(color);

            sf::Color outline = THREAT_MARK_OUTLINE_COLOR;
            outline.a = color.a;
            label.setOutlineColor(outline);

            XYZEngine::RenderSystem::Instance()->Render(label);
        }
    }
}
