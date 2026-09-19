#pragma once

#include <string>
#include <Component.h>
#include <InputSystem.h>

namespace XYZEngine
{
    class ColliderComponent;
    class GameObject;
    struct Trigger;
}

namespace RoguelikeGame
{
    class InteractionComponent;

    class InteractableComponent : public XYZEngine::Component
    {
    public:
        InteractableComponent(XYZEngine::GameObject* gameObject);

        /**
        *	Уходя, сказать. Выход из триггера при удалении объекта не приходит:
        *	физика вычёркивает пару молча, и тот, кто нас слушал, остаётся
        *	с указателем на освобождённую память.
        */
        ~InteractableComponent() override;

        virtual std::string GetPrompt(XYZEngine::GameObject* actor) const = 0;
        virtual XYZEngine::InputAction GetAction() const;
        virtual std::string GetRefusal(XYZEngine::GameObject* actor) const;
        virtual bool IsAvailable() const = 0;
        virtual bool Interact(XYZEngine::GameObject* actor) = 0;

        // Сколько держать клавишу. Ноль - действие мгновенное, как было всегда.
        // Считает время InteractionComponent: он один знает, отпустили её или нет.
        virtual float GetHoldTime() const;

        // Пока счёт идёт - доля выполненного, 0..1. Сорвалось - OnHoldBroken.
        virtual void OnHold(float part, float deltaTime);
        virtual void OnHoldBroken();

    protected:
        void BindReach(XYZEngine::ColliderComponent* reach);

    private:
        XYZEngine::ColliderComponent* reachCollider = nullptr;

        void OnReachEnter(const XYZEngine::Trigger& trigger);
        void OnReachExit(const XYZEngine::Trigger& trigger);
        XYZEngine::GameObject* GetPlayerOf(const XYZEngine::Trigger& trigger) const;
    };
}
