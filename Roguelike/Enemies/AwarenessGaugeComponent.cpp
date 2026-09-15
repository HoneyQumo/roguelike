#include "AwarenessGaugeComponent.h"
#include "ChaseComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <RenderSystem.h>

namespace RoguelikeGame
{
    namespace
    {
        sf::Color Faded(const sf::Color& color, float fade)
        {
            float part = fade < 0.f ? 0.f : (fade > 1.f ? 1.f : fade);

            return sf::Color(color.r, color.g, color.b, static_cast<sf::Uint8>(color.a * part));
        }
    }

    AwarenessGaugeComponent::AwarenessGaugeComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
        sector.setPrimitiveType(sf::TriangleFan);
    }

    void AwarenessGaugeComponent::Start()
    {
        chase = gameObject->GetComponent<ChaseComponent>();
        if (chase == nullptr)
        {
            LOG_ERROR("Awareness gauge needs ChaseComponent on " + gameObject->GetName());
            gameObject->DestroyComponent(this);
        }
    }

    void AwarenessGaugeComponent::Update(float deltaTime)
    {
        if (chase == nullptr)
        {
            return;
        }

        sinceSpotted = chase->GetAwarenessState() == AwarenessState::Provoked ? sinceSpotted + deltaTime : 0.f;
    }

    GaugeLook AwarenessGaugeComponent::ReadLook() const
    {
        return chase != nullptr
            ? LookFor(chase->GetAwareness(), chase->GetAwarenessState(), sinceSpotted)
            : GaugeLook();
    }

    bool AwarenessGaugeComponent::IsShown() const
    {
        return ReadLook().isShown;
    }

    float AwarenessGaugeComponent::GetShownPart() const
    {
        return ReadLook().part;
    }

    void AwarenessGaugeComponent::Render()
    {
        GaugeLook look = ReadLook();
        if (!look.isShown)
        {
            return;
        }

        XYZEngine::Vector2Df position = transform->GetWorldPosition();
        XYZEngine::Vector2Df center = {position.x + offset.x, position.y + offset.y};

        background.setRadius(radius);
        background.setOrigin(radius, radius);
        background.setPosition(center.x, center.y);
        background.setFillColor(Faded(AWARENESS_GAUGE_BACK_COLOR, look.fade));
        XYZEngine::RenderSystem::Instance()->Render(background);

        std::vector<XYZEngine::Vector2Df> points = SectorPoints(center, radius - AWARENESS_GAUGE_RIM, look.part, GAUGE_STEPS);
        if (points.empty())
        {
            return;
        }

        sf::Color fill = Faded(look.isAlarm ? AWARENESS_GAUGE_ALARM_COLOR : AWARENESS_GAUGE_COLOR, look.fade);

        sector.resize(points.size());
        for (std::size_t at = 0u; at < points.size(); at++)
        {
            sector[at].position = {points[at].x, points[at].y};
            sector[at].color = fill;
        }

        XYZEngine::RenderSystem::Instance()->Render(sector);
    }

    void AwarenessGaugeComponent::SetRadius(float newRadius)
    {
        radius = newRadius;
    }

    void AwarenessGaugeComponent::SetOffset(float offsetX, float offsetY)
    {
        offset = {offsetX, offsetY};
    }
}
