#pragma once

#include <functional>
#include <EventList.h>
#include "InteractableComponent.h"
#include "InventoryComponent.h"
#include "ItemDefinition.h"

namespace XYZEngine
{
    class ColliderComponent;
    class SpriteRendererComponent;
}

namespace RoguelikeGame
{
    class ItemPickupComponent : public InteractableComponent
    {
    public:
        ItemPickupComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetDefinition(const ItemDefinition* newDefinition);
        const ItemDefinition* GetDefinition() const;

        // Стопка лежит одним предметом: иначе пять аптечек - это пять подсказок в одной точке.
        void SetStack(int newCount, int newCharge = NO_CHARGE);
        int GetCount() const;
        int GetCharge() const;

        bool IsPickedUp() const;
        bool TryPickUp(XYZEngine::GameObject* collector);

        std::string GetPrompt(XYZEngine::GameObject* actor) const override;
        bool IsAvailable() const override;
        bool Interact(XYZEngine::GameObject* actor) override;

        XYZEngine::SubscriptionId SubscribePickedUp(std::function<void(const ItemDefinition&, XYZEngine::GameObject*)> onPickedUp);

    private:
        ItemDefinition definition;
        bool hasDefinition = false;
        int count = 1;
        int charge = NO_CHARGE;
        XYZEngine::ColliderComponent* collider = nullptr;
        XYZEngine::SpriteRendererComponent* renderer = nullptr;
        bool isPickedUp = false;

        XYZEngine::EventList<const ItemDefinition&, XYZEngine::GameObject*> pickedUpEvent;

        bool Take(const ItemDefinition& item, XYZEngine::GameObject* collector);
        void Hide();
    };
}
