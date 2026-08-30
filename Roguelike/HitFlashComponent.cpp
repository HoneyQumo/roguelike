#include "HitFlashComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <ResourceSystem.h>
#include <algorithm>

namespace RoguelikeGame
{
    HitFlashComponent::HitFlashComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        shader = XYZEngine::ResourceSystem::Instance()->GetShader(HIT_FLASH_SHADER);
    }

    void HitFlashComponent::Update(float deltaTime)
    {
        if (shader == nullptr || amount <= 0.f)
        {
            return;
        }

        amount = std::max(0.f, amount - deltaTime / HIT_FLASH_DURATION);

        for (auto renderer : renderers)
        {
            if (amount <= 0.f)
            {
                renderer->SetShader(nullptr);
            }
            else
            {
                renderer->SetShaderFloat(HIT_FLASH_UNIFORM, amount);
            }
        }
    }

    void HitFlashComponent::Render()
    {
    }

    void HitFlashComponent::AddRenderer(XYZEngine::SpriteRendererComponent* renderer)
    {
        if (renderer != nullptr)
        {
            renderers.push_back(renderer);
        }
    }

    void HitFlashComponent::Flash()
    {
        if (shader == nullptr)
        {
            return;
        }

        amount = 1.f;

        for (auto renderer : renderers)
        {
            renderer->SetShader(shader);
            renderer->SetShaderFloat(HIT_FLASH_UNIFORM, amount);
        }
    }
}
