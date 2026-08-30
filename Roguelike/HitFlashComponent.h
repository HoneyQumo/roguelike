#pragma once

#include <vector>
#include <SFML/Graphics/Shader.hpp>
#include <Component.h>
#include <SpriteRendererComponent.h>

namespace RoguelikeGame
{
    class HitFlashComponent : public XYZEngine::Component
    {
    public:
        HitFlashComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void AddRenderer(XYZEngine::SpriteRendererComponent* renderer);
        void Flash();

    private:
        std::vector<XYZEngine::SpriteRendererComponent*> renderers;
        sf::Shader* shader = nullptr;
        float amount = 0.f;
    };
}
