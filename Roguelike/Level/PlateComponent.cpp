#include "PlateComponent.h"
#include "FactionComponent.h"
#include "GameSettings.h"
#include "TreadComponent.h"
#include <GameObject.h>

namespace RoguelikeGame
{
    PlateComponent::PlateComponent(XYZEngine::GameObject* gameObject) : SwitchComponent(gameObject)
    {
        texturePrefix = PLATE_TEXTURE_PREFIX;
    }

    void PlateComponent::SetTread(TreadComponent* tread)
    {
        if (tread == nullptr)
        {
            return;
        }

        tread->SubscribeStepped([this](XYZEngine::GameObject* walker) { OnStep(walker); });
    }

    std::string PlateComponent::GetPrompt(XYZEngine::GameObject* actor) const
    {
        return {};
    }

    bool PlateComponent::IsAvailable() const
    {
        return false;
    }

    bool PlateComponent::Interact(XYZEngine::GameObject* actor)
    {
        return false;
    }

    void PlateComponent::OnStep(XYZEngine::GameObject* walker)
    {
        // Плитка ждёт беглеца: иначе первый же патруль вскрыл бы тайник.
        if (GetFactionOf(walker) != Faction::Player)
        {
            return;
        }

        Pull();
    }
}
