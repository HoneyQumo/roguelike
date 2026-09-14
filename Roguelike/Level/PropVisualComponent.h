#pragma once

#include <SFML/Graphics/Color.hpp>
#include <Component.h>

namespace sf
{
    class Texture;
}

namespace XYZEngine
{
    class RectangleRendererComponent;
    class SpriteRendererComponent;
}

namespace RoguelikeGame
{
    constexpr int KEEP_RENDER_LAYER = -1;

    class PropVisualComponent : public XYZEngine::Component
    {
    public:
        PropVisualComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetSize(float newSize);
        void SetSpentColor(const sf::Color& newSpentColor);
        void SetSpentTexture(const sf::Texture* newSpentTexture);
        void SetSpentLayer(int newSpentLayer);

        void ShowSpent();
        bool IsSpent() const;

    private:
        XYZEngine::SpriteRendererComponent* sprite = nullptr;
        XYZEngine::RectangleRendererComponent* rectangle = nullptr;

        const sf::Texture* spentTexture = nullptr;
        sf::Color spentColor = {80, 60, 35};
        float size = 0.f;
        int spentLayer = KEEP_RENDER_LAYER;
        bool isSpent = false;
    };
}
