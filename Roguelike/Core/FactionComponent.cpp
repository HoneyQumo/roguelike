#include "FactionComponent.h"
#include <GameObject.h>

namespace RoguelikeGame
{
    FactionComponent::FactionComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
    }

    void FactionComponent::Update(float deltaTime)
    {
    }
    void FactionComponent::Render()
    {
    }

    void FactionComponent::SetFaction(Faction newFaction)
    {
        faction = newFaction;
    }
    Faction FactionComponent::GetFaction() const
    {
        return faction;
    }

    Faction GetFactionOf(XYZEngine::GameObject* gameObject)
    {
        if (gameObject == nullptr)
        {
            return Faction::Neutral;
        }

        auto component = gameObject->GetComponent<FactionComponent>();
        return component == nullptr ? Faction::Neutral : component->GetFaction();
    }
}
