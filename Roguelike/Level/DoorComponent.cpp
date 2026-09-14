#include "DoorComponent.h"
#include "DoorRules.h"
#include "GameSettings.h"
#include "InventoryComponent.h"
#include "LevelGrid.h"
#include "PathService.h"
#include "PropVisualComponent.h"
#include <ColliderComponent.h>
#include <Collision.h>
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    DoorComponent::DoorComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void DoorComponent::Start()
    {
        collider = gameObject->GetComponent<XYZEngine::ColliderComponent>();
        if (collider == nullptr)
        {
            LOG_ERROR("Door needs a collider on " + gameObject->GetName());
            return;
        }

        collider->SubscribeCollision([this](const XYZEngine::Collision& collision) { OnCollision(collision); });
    }

    void DoorComponent::Update(float deltaTime)
    {
    }

    void DoorComponent::Render()
    {
    }

    void DoorComponent::SetDoorId(const std::string& newDoorId)
    {
        doorId = newDoorId;
    }

    const std::string& DoorComponent::GetDoorId() const
    {
        return doorId;
    }

    void DoorComponent::SetVisual(PropVisualComponent* newVisual)
    {
        visual = newVisual;
    }

    bool DoorComponent::IsOpen() const
    {
        return isOpen;
    }

    void DoorComponent::OnCollision(const XYZEngine::Collision& collision)
    {
        XYZEngine::ColliderComponent* other = collision.GetFirst();
        if (other == collider)
        {
            other = collision.GetSecond();
        }

        if (other != nullptr)
        {
            TryOpenFor(other->GetGameObject());
        }
    }

    bool DoorComponent::TryOpenFor(XYZEngine::GameObject* actor)
    {
        if (isOpen || actor == nullptr)
        {
            return false;
        }

        auto inventory = actor->GetComponent<InventoryComponent>();
        if (inventory == nullptr)
        {
            return false;
        }

        std::vector<const ItemDefinition*> carried;
        for (int slot = 0; slot < inventory->GetCapacity(); slot++)
        {
            carried.push_back(inventory->GetSlot(slot).item);
        }

        const ItemDefinition* key = FindKeyFor(carried, doorId);
        if (key == nullptr)
        {
            refusedEvent.Invoke(gameObject->GetTransform()->GetWorldPosition());
            return false;
        }

        inventory->RemoveById(key->id);

        isOpen = true;
        if (collider != nullptr)
        {
            collider->SetTrigger(true);
        }

        if (visual != nullptr)
        {
            visual->ShowSpent();
        }

        XYZEngine::Vector2Df where = gameObject->GetTransform()->GetWorldPosition();
        LevelGrid::OpenCell(where);
        PathService::Reset();

        LOG_INFO("Door " + doorId + " is unlocked with " + key->id);
        openedEvent.Invoke(where);

        return true;
    }

    XYZEngine::SubscriptionId DoorComponent::SubscribeOpened(std::function<void(const XYZEngine::Vector2Df&)> onOpened)
    {
        return openedEvent.Subscribe(std::move(onOpened));
    }

    XYZEngine::SubscriptionId DoorComponent::SubscribeRefused(std::function<void(const XYZEngine::Vector2Df&)> onRefused)
    {
        return refusedEvent.Subscribe(std::move(onRefused));
    }
}
