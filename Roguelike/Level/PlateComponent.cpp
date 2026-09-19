#include "PlateComponent.h"
#include "FactionComponent.h"
#include "GameSettings.h"
#include <ColliderComponent.h>
#include <GameObject.h>
#include <Trigger.h>

namespace RoguelikeGame
{
    PlateComponent::PlateComponent(XYZEngine::GameObject* gameObject) : SwitchComponent(gameObject)
    {
        texturePrefix = PLATE_TEXTURE_PREFIX;
    }

    void PlateComponent::SetPad(XYZEngine::ColliderComponent* newPad)
    {
        pad = newPad;
        if (pad != nullptr)
        {
            pad->SubscribeTriggerEnter([this](const XYZEngine::Trigger& trigger) { OnStep(trigger); });
        }
    }

    std::string PlateComponent::GetPrompt(XYZEngine::GameObject* actor) const
    {
        return {};
    }

    bool PlateComponent::IsAvailable() const
    {
        return false;
    }

    bool PlateComponent::Interact(XYZEngine::GameObject* actor)
    {
        return false;
    }

    void PlateComponent::OnStep(const XYZEngine::Trigger& trigger)
    {
        XYZEngine::ColliderComponent* other = trigger.GetFirst() == pad ? trigger.GetSecond() : trigger.GetFirst();
        if (other == nullptr || GetFactionOf(other->GetGameObject()) != Faction::Player)
        {
            return;
        }

        Pull();
    }
}
