#include "ReloadIndicatorComponent.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <GameWorld.h>
#include <ResourceSystem.h>
#include <LoggerRegistry.h>
#include <algorithm>

namespace RoguelikeGame
{
    ReloadIndicatorComponent::ReloadIndicatorComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        crosshairTexture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(CROSSHAIR_TEXTURE);

        for (int frame = 0; frame < RELOAD_MAG_FRAMES; frame++)
        {
            magazineFrames.push_back(XYZEngine::ResourceSystem::Instance()->GetTextureMapElementShared(RELOAD_MAG_TEXTURE, frame));
        }

        if (magazineFrames.empty() || magazineFrames[0] == nullptr)
        {
            LOG_ERROR("Reload indicator has no magazine frames");
        }
    }

    void ReloadIndicatorComponent::Update(float deltaTime)
    {
        if (renderer == nullptr)
        {
            renderer = gameObject->GetComponent<XYZEngine::SpriteRendererComponent>();
            if (renderer == nullptr)
            {
                return;
            }
        }

        if (weapon == nullptr)
        {
            FindWeapon();
        }

        if (weapon == nullptr || !weapon->IsReloading())
        {
            ShowCrosshair();
            return;
        }

        int lastFrame = static_cast<int>(magazineFrames.size()) - 1;
        int frame = static_cast<int>(weapon->GetReloadProgress() * (lastFrame + 1));
        ShowMagazineFrame(std::min(std::max(frame, 0), lastFrame));
    }

    void ReloadIndicatorComponent::Render()
    {
    }

    void ReloadIndicatorComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
        weapon = nullptr;
    }

    void ReloadIndicatorComponent::FindWeapon()
    {
        if (targetName.empty())
        {
            return;
        }

        XYZEngine::GameObject* target = XYZEngine::GameWorld::Instance()->FindGameObject(targetName);
        if (target == nullptr)
        {
            return;
        }

        weapon = target->GetComponent<XYZEngine::WeaponComponent>();
    }

    void ReloadIndicatorComponent::ShowMagazineFrame(int frame)
    {
        if (isIndicatorShown && frame == currentFrame)
        {
            return;
        }

        const sf::Texture* texture = magazineFrames[frame];
        if (texture == nullptr)
        {
            return;
        }

        renderer->SetTexture(*texture);
        renderer->SetPixelSize(RELOAD_INDICATOR_SIZE, RELOAD_INDICATOR_SIZE);
        renderer->SetColor(RELOAD_INDICATOR_COLOR);

        currentFrame = frame;
        isIndicatorShown = true;
    }

    void ReloadIndicatorComponent::ShowCrosshair()
    {
        if (!isIndicatorShown || crosshairTexture == nullptr)
        {
            return;
        }

        renderer->SetTexture(*crosshairTexture);
        renderer->SetPixelSize(CROSSHAIR_SIZE, CROSSHAIR_SIZE);
        renderer->SetColor(CROSSHAIR_COLOR);

        currentFrame = -1;
        isIndicatorShown = false;
    }
}
