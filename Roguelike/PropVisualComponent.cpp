#include "PropVisualComponent.h"
#include <GameObject.h>
#include <RectangleRendererComponent.h>
#include <SpriteRendererComponent.h>

namespace RoguelikeGame
{
    PropVisualComponent::PropVisualComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void PropVisualComponent::Start()
    {
        sprite = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
        rectangle = gameObject->GetComponent<XYZEngine::RectangleRendererComponent>();
    }

    void PropVisualComponent::Update(float deltaTime)
    {
    }

    void PropVisualComponent::Render()
    {
    }

    void PropVisualComponent::SetSize(float newSize)
    {
        size = newSize;
    }

    void PropVisualComponent::SetSpentColor(const sf::Color& newSpentColor)
    {
        spentColor = newSpentColor;
    }

    void PropVisualComponent::SetSpentTexture(const sf::Texture* newSpentTexture)
    {
        spentTexture = newSpentTexture;
    }

    bool PropVisualComponent::IsSpent() const
    {
        return isSpent;
    }

    void PropVisualComponent::ShowSpent()
    {
        if (isSpent)
        {
            return;
        }

        isSpent = true;

        if (sprite != nullptr && spentTexture != nullptr)
        {
            sprite->SetTexture(*spentTexture);
            sprite->SetPixelSize(static_cast<int>(size), static_cast<int>(size));
            return;
        }

        if (sprite != nullptr)
        {
            sprite->SetColor(spentColor);
            return;
        }

        if (rectangle != nullptr)
        {
            rectangle->SetColor(spentColor);
        }
    }
}
