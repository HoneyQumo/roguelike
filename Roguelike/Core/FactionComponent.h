#pragma once

#include <Component.h>
#include "Faction.h"

namespace RoguelikeGame
{
    class FactionComponent : public XYZEngine::Component
    {
    public:
        FactionComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetFaction(Faction newFaction);
        Faction GetFaction() const;

    private:
        Faction faction = Faction::Neutral;
    };

    Faction GetFactionOf(XYZEngine::GameObject* gameObject);
}
