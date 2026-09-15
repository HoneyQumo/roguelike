#include "AwarenessBarComponent.h"
#include "ChaseComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <RenderSystem.h>

namespace RoguelikeGame
{
    AwarenessBarComponent::AwarenessBarComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();

        background.setFillColor(AWARENESS_BAR_BACK_COLOR);
        fill.setFillColor(AWARENESS_BAR_COLOR);
    }

    void AwarenessBarComponent::Start()
    {
        chase = gameObject->GetComponent<ChaseComponent>();
        if (chase == nullptr)
        {
            LOG_ERROR("Awareness bar needs ChaseComponent on " + gameObject->GetName());
            gameObject->DestroyComponent(this);
        }
    }

    void AwarenessBarComponent::Update(float deltaTime)
    {
    }

    bool AwarenessBarComponent::IsShown() const
    {
        return chase != nullptr && chase->GetAwareness() > 0.f;
    }

    float AwarenessBarComponent::GetShownPart() const
    {
        return chase != nullptr ? AwarenessPart(chase->GetAwareness()) : 0.f;
    }

    void AwarenessBarComponent::Render()
    {
        if (!IsShown())
        {
            return;
        }

        XYZEngine::Vector2Df position = transform->GetWorldPosition();
        sf::Vector2f place = {position.x - 0.5f * size.x + offset.x, position.y + offset.y};

        background.setSize({size.x, size.y});
        background.setPosition(place);

        fill.setSize({size.x * GetShownPart(), size.y});
        fill.setPosition(place);
        fill.setFillColor(chase->GetAwarenessState() == AwarenessState::Provoked
            ? AWARENESS_BAR_ALARM_COLOR
            : AWARENESS_BAR_COLOR);

        XYZEngine::RenderSystem::Instance()->Render(background);
        XYZEngine::RenderSystem::Instance()->Render(fill);
    }

    void AwarenessBarComponent::SetSize(float newWidth, float newHeight)
    {
        size = {newWidth, newHeight};
    }

    void AwarenessBarComponent::SetOffset(float offsetX, float offsetY)
    {
        offset = {offsetX, offsetY};
    }
}
