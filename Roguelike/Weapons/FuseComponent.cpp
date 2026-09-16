#include "FuseComponent.h"
#include <GameObject.h>

namespace RoguelikeGame
{
    FuseComponent::FuseComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void FuseComponent::Update(float deltaTime)
    {
        if (!isLit)
        {
            return;
        }

        fuse.Tick(deltaTime);
        if (fuse.IsRunning())
        {
            return;
        }

        isLit = false;
        hasBurnedOut = true;
        burnedOutEvent.Invoke();
    }

    void FuseComponent::Render()
    {
    }

    // Повторный поджиг ничего не меняет: горящий фитиль не удлиняется от новых попаданий.
    void FuseComponent::Light(float seconds)
    {
        if (isLit || hasBurnedOut)
        {
            return;
        }

        isLit = true;
        fuse.Start(seconds);
    }

    bool FuseComponent::IsLit() const
    {
        return isLit;
    }

    bool FuseComponent::HasBurnedOut() const
    {
        return hasBurnedOut;
    }

    float FuseComponent::GetLeft() const
    {
        return fuse.GetLeft();
    }

    XYZEngine::SubscriptionId FuseComponent::SubscribeBurnedOut(std::function<void()> onBurnedOut)
    {
        return burnedOutEvent.Subscribe(std::move(onBurnedOut));
    }
}
