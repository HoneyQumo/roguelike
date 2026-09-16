#include "SettleComponent.h"
#include "Freeze.h"
#include <GameObject.h>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    SettleComponent::SettleComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void SettleComponent::Start()
    {
    }

    void SettleComponent::Update(float deltaTime)
    {
        if (isSettled || (isReady != nullptr && !isReady()))
        {
            return;
        }

        Settle();
    }

    void SettleComponent::Render()
    {
    }

    void SettleComponent::SetReadyCheck(std::function<bool()> newIsReady)
    {
        isReady = std::move(newIsReady);
    }

    bool SettleComponent::IsSettled() const
    {
        return isSettled;
    }

    void SettleComponent::Settle()
    {
        if (isSettled)
        {
            return;
        }

        isSettled = true;

        int quieted = 0;
        for (XYZEngine::Component* component : gameObject->GetComponentsInChildren<XYZEngine::Component>())
        {
            if (component == this || component == nullptr || KeepsDrawing(component) || !component->IsEnabled())
            {
                continue;
            }

            component->SetEnabled(false);
            quieted++;
        }

        SetEnabled(false);

        LOG_INFO(gameObject->GetName() + " settles down, " + std::to_string(quieted) + " components are quiet");
    }
}
