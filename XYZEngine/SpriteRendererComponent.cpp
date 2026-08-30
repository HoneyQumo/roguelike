#include "pch.h"
#include "SpriteRendererComponent.h"
#include "TransformComponent.h"
#include "RenderSystem.h"

namespace XYZEngine
{
    SpriteRendererComponent::SpriteRendererComponent(GameObject* gameObject) : Component(gameObject)
    {
        sprite = new sf::Sprite();
        scale = {1, -1};
        sprite->setScale({1, -1});
        transform = gameObject->GetComponent<TransformComponent>();
    }

    SpriteRendererComponent::~SpriteRendererComponent()
    {
        if (sprite != nullptr)
        {
            delete sprite;
        }
    }

    void SpriteRendererComponent::Update(float deltaTime)
    {
    }

    void SpriteRendererComponent::Render()
    {
        if (sprite == nullptr || !isVisible)
        {
            return;
        }

        sprite->setPosition(Convert<sf::Vector2f, Vector2Df>(transform->GetWorldPosition()));
        sprite->setRotation(transform->GetWorldRotation());

        auto transformScale = Convert<sf::Vector2f, Vector2Df>(transform->GetWorldScale());
        sprite->setScale({scale.x * transformScale.x, scale.y * transformScale.y});

        sf::RenderStates states(isAdditiveBlending ? sf::BlendAdd : sf::BlendAlpha);

        if (shader != nullptr)
        {
            shader->setUniform("texture", sf::Shader::CurrentTexture);
            for (const auto& shaderFloat : shaderFloats)
            {
                shader->setUniform(shaderFloat.first, shaderFloat.second);
            }
            states.shader = shader;
        }

        RenderSystem::Instance()->Render(*sprite, states);
    }

    const sf::Sprite* SpriteRendererComponent::GetSprite() const
    {
        return sprite;
    }

    void SpriteRendererComponent::SetTexture(const sf::Texture& newTexture)
    {
        sprite->setTexture(newTexture, true);
        ApplyPivot();
    }

    void SpriteRendererComponent::SetPixelSize(int newWidth, int newHeight)
    {
        auto originalSize = sprite->getTexture()->getSize();
        scale = {static_cast<float>(newWidth) / static_cast<float>(originalSize.x), -static_cast<float>(newHeight) / static_cast<float>(originalSize.y)};
    }

    void SpriteRendererComponent::SetColor(const sf::Color& newColor)
    {
        sprite->setColor(newColor);
    }

    void SpriteRendererComponent::SetAdditiveBlending(bool isAdditive)
    {
        isAdditiveBlending = isAdditive;
    }

    void SpriteRendererComponent::SetPivot(float relativeX, float relativeY)
    {
        pivot = {relativeX, relativeY};
        ApplyPivot();
    }

    void SpriteRendererComponent::SetVisible(bool newIsVisible)
    {
        isVisible = newIsVisible;
    }

    bool SpriteRendererComponent::IsVisible() const
    {
        return isVisible;
    }

    void SpriteRendererComponent::SetShader(sf::Shader* newShader)
    {
        shader = newShader;
    }

    void SpriteRendererComponent::SetShaderFloat(const std::string& uniformName, float value)
    {
        shaderFloats[uniformName] = value;
    }

    void SpriteRendererComponent::FlipX(bool flip)
    {
        if (flip != isFlipX)
        {
            scale = {-scale.x, scale.y};
            isFlipX = flip;
        }
    }

    void SpriteRendererComponent::FlipY(bool flip)
    {
        if (flip != isFlipY)
        {
            scale = {scale.x, -scale.y};
            isFlipY = flip;
        }
    }

    void SpriteRendererComponent::ApplyPivot()
    {
        if (sprite->getTexture() == nullptr)
        {
            return;
        }

        auto textureSize = sprite->getTexture()->getSize();
        sprite->setOrigin({pivot.x * textureSize.x, pivot.y * textureSize.y});
    }
}
