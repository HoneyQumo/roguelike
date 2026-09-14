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
        if (shader == nullptr)
        {
            return;
        }

        if (amount > 0.f)
        {
            amount = std::max(0.f, amount - deltaTime / HIT_FLASH_DURATION);
        }

        Apply(std::max(amount, glow));
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
        Apply(std::max(amount, glow));
    }

    void HitFlashComponent::SetGlow(float newGlow)
    {
        glow = std::max(newGlow, 0.f);
    }

    void HitFlashComponent::Apply(float value)
    {
        if (value <= 0.f && !isShaderApplied)
        {
            return;
        }

        isShaderApplied = value > 0.f;

        for (auto renderer : renderers)
        {
            if (!isShaderApplied)
            {
                renderer->SetShader(nullptr);
            }
            else
            {
                renderer->SetShader(shader);
                renderer->SetShaderFloat(HIT_FLASH_UNIFORM, value);
            }
        }
    }
}
