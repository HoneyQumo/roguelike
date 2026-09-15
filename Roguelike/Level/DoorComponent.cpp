#include "DoorComponent.h"
#include "DoorRules.h"
#include "GameSettings.h"
#include "InventoryComponent.h"
#include "LevelGrid.h"
#include "PathService.h"
#include "PropVisualComponent.h"
#include <AudioComponent.h>
#include <ColliderComponent.h>
#include <ResourceSystem.h>
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>
#include <algorithm>

namespace RoguelikeGame
{
    namespace
    {
        float Eased(float progress)
        {
            return progress * progress * (3.f - 2.f * progress);
        }

        const ItemDefinition* CarriedKey(XYZEngine::GameObject* actor, const std::string& doorId)
        {
            if (actor == nullptr)
            {
                return nullptr;
            }

            auto inventory = actor->GetComponent<InventoryComponent>();
            if (inventory == nullptr)
            {
                return nullptr;
            }

            std::vector<const ItemDefinition*> carried;
            for (int slot = 0; slot < inventory->GetCapacity(); slot++)
            {
                carried.push_back(inventory->GetSlot(slot).item);
            }

            return FindKeyFor(carried, doorId);
        }
    }

    DoorComponent::DoorComponent(XYZEngine::GameObject* gameObject) : InteractableComponent(gameObject) {}

    void DoorComponent::Start()
    {
        collider = gameObject->GetComponent<XYZEngine::ColliderComponent>();
        if (collider == nullptr)
        {
            LOG_ERROR("Door needs a collider on " + gameObject->GetName());
        }
    }

    void DoorComponent::Update(float deltaTime)
    {
        if (!IsSwinging())
        {
            return;
        }

        swingTime = std::min(swingTime + deltaTime, DOOR_SWING_TIME);
        TurnLeaf();
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

    void DoorComponent::SetKeyName(const std::string& newKeyName)
    {
        keyName = newKeyName;
    }

    void DoorComponent::SetAudio(XYZEngine::AudioComponent* newAudio)
    {
        audio = newAudio;
    }

    void DoorComponent::SetVisual(PropVisualComponent* newVisual)
    {
        visual = newVisual;
    }

    void DoorComponent::SetLeaf(XYZEngine::TransformComponent* newLeaf)
    {
        leaf = newLeaf;
        TurnLeaf();
    }

    void DoorComponent::SetHinge(const DoorHinge& newHinge)
    {
        hinge = newHinge;
        TurnLeaf();
    }

    void DoorComponent::SetReach(XYZEngine::ColliderComponent* reach)
    {
        BindReach(reach);
    }

    std::string DoorComponent::GetPrompt(XYZEngine::GameObject* actor) const
    {
        if (isOpen)
        {
            return {};
        }

        if (!HasKey(actor))
        {
            return std::string(DOOR_LOCKED_PREFIX) + (keyName.empty() ? DOOR_UNKNOWN_KEY_NAME : keyName);
        }

        return DOOR_OPEN_PROMPT;
    }

    std::string DoorComponent::GetRefusal(XYZEngine::GameObject* actor) const
    {
        if (isOpen || HasKey(actor))
        {
            return {};
        }

        return std::string(DOOR_LOCKED_PREFIX) + (keyName.empty() ? DOOR_UNKNOWN_KEY_NAME : keyName);
    }

    bool DoorComponent::IsAvailable() const
    {
        return !isOpen;
    }

    bool DoorComponent::Interact(XYZEngine::GameObject* actor)
    {
        return TryOpenFor(actor);
    }

    bool DoorComponent::IsOpen() const
    {
        return isOpen;
    }

    bool DoorComponent::IsSwinging() const
    {
        return isOpen && swingTime < DOOR_SWING_TIME;
    }

    float DoorComponent::GetLeafAngle() const
    {
        if (!isOpen)
        {
            return hinge.closedAngle;
        }

        float progress = DOOR_SWING_TIME > 0.f ? std::min(swingTime / DOOR_SWING_TIME, 1.f) : 1.f;

        return hinge.closedAngle + (hinge.openAngle - hinge.closedAngle) * Eased(progress);
    }

    bool DoorComponent::HasKey(XYZEngine::GameObject* actor) const
    {
        return CarriedKey(actor, doorId) != nullptr;
    }

    bool DoorComponent::TryOpenFor(XYZEngine::GameObject* actor)
    {
        if (isOpen || actor == nullptr)
        {
            return false;
        }

        const ItemDefinition* key = CarriedKey(actor, doorId);
        if (key == nullptr)
        {
            refusedEvent.Invoke(gameObject->GetTransform()->GetWorldPosition());
            return false;
        }

        actor->GetComponent<InventoryComponent>()->RemoveById(key->id);
        LOG_INFO("Door " + doorId + " is unlocked with " + key->id);

        Open();

        return true;
    }

    void DoorComponent::Open()
    {
        if (isOpen)
        {
            return;
        }

        isOpen = true;
        swingTime = 0.f;

        if (audio != nullptr)
        {
            const sf::SoundBuffer* sound = XYZEngine::ResourceSystem::Instance()->GetSound(DOOR_OPEN_SOUND);
            if (sound != nullptr)
            {
                audio->SetSound(sound);
                audio->SetVolume(DOOR_VOLUME);
                audio->Play();
            }
        }

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

        TurnLeaf();
        openedEvent.Invoke(where);
    }

    void DoorComponent::TurnLeaf()
    {
        if (leaf == nullptr)
        {
            return;
        }

        leaf->SetWorldPosition(hinge.pivot);
        leaf->SetWorldRotation(GetLeafAngle());
    }

    XYZEngine::SubscriptionId DoorComponent::SubscribeOpened(std::function<void(const XYZEngine::Vector2Df&)> onOpened)
    {
        return openedEvent.Subscribe(std::move(onOpened));
    }

    XYZEngine::SubscriptionId DoorComponent::SubscribeRefused(std::function<void(const XYZEngine::Vector2Df&)> onRefused)
    {
        return refusedEvent.Subscribe(std::move(onRefused));
    }

    void LinkDoors(const std::vector<DoorComponent*>& doors)
    {
        for (DoorComponent* door : doors)
        {
            if (door == nullptr)
            {
                continue;
            }

            door->SubscribeOpened([door, doors](const XYZEngine::Vector2Df&)
            {
                for (DoorComponent* other : doors)
                {
                    if (other != nullptr && other != door && other->GetDoorId() == door->GetDoorId())
                    {
                        other->Open();
                    }
                }
            });
        }
    }
}
