#pragma once

#include <SFML/Graphics/RectangleShape.hpp>
#include <Component.h>
#include <TransformComponent.h>
#include <Vector.h>

namespace RoguelikeGame
{
    class ChaseComponent;

    class AwarenessBarComponent : public XYZEngine::Component
    {
    public:
        AwarenessBarComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetSize(float newWidth, float newHeight);
        void SetOffset(float offsetX, float offsetY);

        bool IsShown() const;
        float GetShownPart() const;

    private:
        XYZEngine::TransformComponent* transform = nullptr;
        ChaseComponent* chase = nullptr;

        sf::RectangleShape background;
        sf::RectangleShape fill;

        XYZEngine::Vector2Df size = {34.f, 5.f};
        XYZEngine::Vector2Df offset = {0.f, 52.f};
    };
}
