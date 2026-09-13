#pragma once

#include <string>
#include <Component.h>

namespace XYZEngine
{
    class ColliderComponent;
    class GameObject;
    class Trigger;
}

namespace RoguelikeGame
{
    class InteractableComponent : public XYZEngine::Component
    {
    public:
        InteractableComponent(XYZEngine::GameObject* gameObject);

        virtual std::string GetPrompt(XYZEngine::GameObject* actor) const = 0;
        virtual std::string GetRefusal(XYZEngine::GameObject* actor) const;
        virtual bool IsAvailable() const = 0;
        virtual bool Interact(XYZEngine::GameObject* actor) = 0;

    protected:
        void BindReach(XYZEngine::ColliderComponent* reach);

    private:
        XYZEngine::ColliderComponent* reachCollider = nullptr;

        void OnReachEnter(const XYZEngine::Trigger& trigger);
        void OnReachExit(const XYZEngine::Trigger& trigger);
        XYZEngine::GameObject* GetPlayerOf(const XYZEngine::Trigger& trigger) const;
    };
}
