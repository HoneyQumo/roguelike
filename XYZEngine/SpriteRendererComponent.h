#pragma once

#include "TransformComponent.h"
#include "Vector.h"
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>
#include <string>

namespace XYZEngine
{
    class SpriteRendererComponent : public Component
    {
    public:
        SpriteRendererComponent(GameObject* gameObject);
        ~SpriteRendererComponent() override;

        void Update(float deltaTime) override;
        void Render() override;

        const sf::Sprite* GetSprite() const;
        void SetTexture(const sf::Texture& newTexture);
        void SetPixelSize(int newWidth, int newHeight);
        void SetColor(const sf::Color& newColor);
        void SetAdditiveBlending(bool isAdditive);

        void SetPivot(float relativeX, float relativeY);
        void SetVisible(bool newIsVisible);
        bool IsVisible() const;

        void SetShader(sf::Shader* newShader);
        void SetShaderFloat(const std::string& uniformName, float value);

        void FlipX(bool flip);
        void FlipY(bool flip);

    private:
        Vector2Df scale;
        Vector2Df pivot = {0.5f, 0.5f};
        sf::Sprite* sprite;
        TransformComponent* transform;

        sf::Shader* shader = nullptr;
        std::map<std::string, float> shaderFloats;

        bool isFlipX = false;
        bool isFlipY = false;
        bool isAdditiveBlending = false;
        bool isVisible = true;

        void ApplyPivot();
    };
}
