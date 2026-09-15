#pragma once

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <Component.h>
#include <TransformComponent.h>
#include <Vector.h>
#include "AwarenessGauge.h"

namespace RoguelikeGame
{
    class ChaseComponent;

    class AwarenessGaugeComponent : public XYZEngine::Component
    {
    public:
        AwarenessGaugeComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetRadius(float newRadius);
        void SetOffset(float offsetX, float offsetY);

        GaugeLook ReadLook() const;
        bool IsShown() const;
        float GetShownPart() const;

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        ChaseComponent* chase = nullptr;

        sf::CircleShape background;
        sf::VertexArray sector;

        float radius = 11.f;
        float sinceSpotted = 0.f;
        XYZEngine::Vector2Df offset = {0.f, 58.f};
    };
}
