#include "FogVisibilityComponent.h"
#include "FogOfWar.h"
#include <GameObject.h>

namespace RoguelikeGame
{
    FogVisibilityComponent::FogVisibilityComponent(XYZEngine::GameObject* gameObject)
        : Component(gameObject)
    {
    }

    void FogVisibilityComponent::Update(float deltaTime)
    {
        const FogOfWar& fog = FogOfWar::Current();
        if (!fog.IsEnabled() || gameObject == nullptr || gameObject->GetTransform() == nullptr)
        {
            return;
        }

        bool isOutOfSight = fog.GetStateAt(gameObject->GetTransform()->GetWorldPosition()) != FogState::Seen;
        if (isOutOfSight == isHidden)
        {
            return;
        }

        isHidden = isOutOfSight;
        gameObject->SetVisible(!isHidden);
    }

    bool FogVisibilityComponent::IsHidden() const
    {
        return isHidden;
    }
}
