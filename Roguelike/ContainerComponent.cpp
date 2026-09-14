#include "ContainerComponent.h"
#include "GameSettings.h"
#include "InventoryComponent.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <RectangleRendererComponent.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    ContainerComponent::ContainerComponent(XYZEngine::GameObject* gameObject) : InteractableComponent(gameObject) {}

    void ContainerComponent::Start()
    {
        renderer = gameObject->GetComponent<XYZEngine::RectangleRendererComponent>();
    }

    void ContainerComponent::Update(float deltaTime)
    {
    }

    void ContainerComponent::Render()
    {
    }

    void ContainerComponent::SetTitle(std::string newTitle)
    {
        title = std::move(newTitle);
    }

    void ContainerComponent::SetKeyItem(std::string itemId, std::string newKeyName)
    {
        keyItem = std::move(itemId);
        keyName = std::move(newKeyName);

        if (keyName.empty())
        {
            keyName = keyItem;
        }
    }

    void ContainerComponent::SetOpenedColor(const sf::Color& newOpenedColor)
    {
        openedColor = newOpenedColor;
    }

    void ContainerComponent::SetReach(XYZEngine::ColliderComponent* reach)
    {
        BindReach(reach);
    }

    bool ContainerComponent::IsLocked() const
    {
        return !keyItem.empty();
    }

    bool ContainerComponent::IsOpen() const
    {
        return isOpen;
    }

    bool ContainerComponent::IsOpenableBy(XYZEngine::GameObject* actor) const
    {
        if (!IsLocked())
        {
            return true;
        }

        if (actor == nullptr)
        {
            return false;
        }

        auto inventory = actor->GetComponent<InventoryComponent>();

        return inventory != nullptr && inventory->Contains(keyItem);
    }

    std::string ContainerComponent::GetPrompt(XYZEngine::GameObject* actor) const
    {
        if (isOpen)
        {
            return {};
        }

        if (!IsOpenableBy(actor))
        {
            return std::string(CONTAINER_LOCKED_PREFIX) + keyName;
        }

        return std::string(CONTAINER_OPEN_PREFIX) + title;
    }

    std::string ContainerComponent::GetRefusal(XYZEngine::GameObject* actor) const
    {
        if (isOpen || IsOpenableBy(actor))
        {
            return {};
        }

        return std::string(CONTAINER_LOCKED_PREFIX) + keyName;
    }

    bool ContainerComponent::IsAvailable() const
    {
        return !isOpen;
    }

    bool ContainerComponent::Interact(XYZEngine::GameObject* actor)
    {
        if (isOpen || actor == nullptr)
        {
            return false;
        }

        if (IsLocked())
        {
            auto inventory = actor->GetComponent<InventoryComponent>();
            if (inventory == nullptr || !inventory->RemoveById(keyItem))
            {
                LOG_INFO(gameObject->GetName() + " is locked, " + keyItem + " is missing");
                return false;
            }
        }

        isOpen = true;

        if (renderer != nullptr)
        {
            renderer->SetColor(openedColor);
        }

        LOG_INFO(actor->GetName() + " opens " + gameObject->GetName());

        openedEvent.Invoke(gameObject->GetTransform()->GetWorldPosition());

        return true;
    }

    XYZEngine::SubscriptionId ContainerComponent::SubscribeOpened(std::function<void(const XYZEngine::Vector2Df&)> onOpened)
    {
        return openedEvent.Subscribe(std::move(onOpened));
    }
}
