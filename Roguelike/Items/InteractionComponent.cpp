#include "InteractionComponent.h"
#include "GameSettings.h"
#include "InteractableComponent.h"
#include <GameObject.h>
#include <InputComponent.h>
#include <UiManager.h>
#include <LoggerRegistry.h>
#include <TextUtils.h>
#include <TransformComponent.h>
#include <algorithm>

namespace RoguelikeGame
{
    namespace
    {
        std::vector<InteractionComponent*> living;
    }

    InteractionComponent::InteractionComponent(XYZEngine::GameObject* gameObject) : Component(gameObject)
    {
        transform = gameObject->GetTransform();
        living.push_back(this);
    }

    const std::vector<InteractionComponent*>& InteractionComponent::GetLiving()
    {
        return living;
    }

    InteractionComponent::~InteractionComponent()
    {
        living.erase(std::remove(living.begin(), living.end(), this), living.end());
    }

    void InteractionComponent::Start()
    {
        input = gameObject->GetComponent<XYZEngine::InputComponent>();
    }

    void InteractionComponent::AddCandidate(InteractableComponent* candidate)
    {
        if (candidate == nullptr || std::find(candidates.begin(), candidates.end(), candidate) != candidates.end())
        {
            return;
        }

        candidates.push_back(candidate);
    }

    void InteractionComponent::RemoveCandidate(InteractableComponent* candidate)
    {
        candidates.erase(std::remove(candidates.begin(), candidates.end(), candidate), candidates.end());

        if (held == candidate)
        {
            BreakHold();
        }

        if (target == candidate)
        {
            SetTarget(nullptr);
        }
    }

    void InteractionComponent::Update(float deltaTime)
    {
        // Сумка забирает ввод себе, и мир при этом не стоит: без этой проверки
        // рубильник продолжал бы греметь, пока игрок перекладывает предметы.
        if (XYZEngine::UiManager::Instance()->IsInputCaptured())
        {
            BreakHold();
            return;
        }

        UpdateTarget();

        if (held != nullptr)
        {
            KeepHolding(deltaTime);
            return;
        }

        if (input == nullptr || target == nullptr)
        {
            return;
        }

        if (!input->WasActionPressed(target->GetAction()))
        {
            return;
        }

        if (target->GetHoldTime() > 0.f)
        {
            StartHold(target);
            return;
        }

        Interact();
    }

    bool InteractionComponent::IsHolding() const
    {
        return held != nullptr;
    }

    float InteractionComponent::GetHoldPart() const
    {
        return heldPart;
    }

    void InteractionComponent::StartHold(InteractableComponent* wanted)
    {
        held = wanted;
        heldTime = 0.f;
        heldFrom = transform->GetWorldPosition();
        SetHoldPart(0.f);
    }

    /**
    *	Счёт идёт, пока клавишу держат и пока стоят на месте. Сдвинулся -
    *	сорвалось: в этом вся цена действия, иначе рубильник дёргали бы на бегу.
    */
    void InteractionComponent::KeepHolding(float deltaTime)
    {
        if (input == nullptr || !held->IsAvailable()
            || !input->IsActionHeld(held->GetAction())
            || (transform->GetWorldPosition() - heldFrom).GetLengthSquared() > HOLD_SLIP * HOLD_SLIP)
        {
            BreakHold();
            return;
        }

        float holdTime = held->GetHoldTime();
        heldTime += deltaTime;
        SetHoldPart(holdTime > 0.f ? heldTime / holdTime : 1.f);
        held->OnHold(heldPart, deltaTime);

        if (heldTime < holdTime)
        {
            return;
        }

        InteractableComponent* done = held;
        held = nullptr;
        SetHoldPart(0.f);

        if (!done->Interact(gameObject))
        {
            refusedEvent.Invoke(done->GetRefusal(gameObject));
        }

        UpdateTarget();
    }

    void InteractionComponent::BreakHold()
    {
        InteractableComponent* broken = held;
        held = nullptr;
        SetHoldPart(0.f);

        if (broken != nullptr)
        {
            broken->OnHoldBroken();
        }
    }

    void InteractionComponent::SetHoldPart(float part)
    {
        float wanted = part < 0.f ? 0.f : (part > 1.f ? 1.f : part);
        if (wanted == heldPart)
        {
            return;
        }

        heldPart = wanted;
        holdChangedEvent.Invoke(heldPart);
    }

    XYZEngine::SubscriptionId InteractionComponent::SubscribeHoldChanged(std::function<void(float)> onHoldChanged)
    {
        return holdChangedEvent.Subscribe(std::move(onHoldChanged));
    }

    void InteractionComponent::Render()
    {
    }

    void InteractionComponent::UpdateTarget()
    {
        candidates.erase(std::remove_if(candidates.begin(), candidates.end(),
            [](InteractableComponent* candidate) { return candidate == nullptr || !candidate->IsAvailable(); }), candidates.end());

        SetTarget(FindNearest());
    }

    InteractableComponent* InteractionComponent::FindNearest() const
    {
        InteractableComponent* nearest = nullptr;
        float nearestDistance = 0.f;

        for (InteractableComponent* candidate : candidates)
        {
            float distance = (candidate->GetGameObject()->GetTransform()->GetWorldPosition()
                - transform->GetWorldPosition()).GetLengthSquared();

            if (nearest == nullptr || distance < nearestDistance)
            {
                nearest = candidate;
                nearestDistance = distance;
            }
        }

        return nearest;
    }

    void InteractionComponent::SetTarget(InteractableComponent* newTarget)
    {
        target = newTarget;

        std::string wanted = target != nullptr ? target->GetPrompt(gameObject) : std::string();
        if (wanted == prompt)
        {
            return;
        }

        prompt = std::move(wanted);
        promptChangedEvent.Invoke(prompt);
    }

    InteractableComponent* InteractionComponent::GetTarget() const
    {
        return target;
    }

    std::string InteractionComponent::GetPrompt() const
    {
        return prompt;
    }

    bool InteractionComponent::Interact()
    {
        if (target == nullptr)
        {
            return false;
        }

        InteractableComponent* chosen = target;
        if (!chosen->Interact(gameObject))
        {
            refusedEvent.Invoke(chosen->GetRefusal(gameObject));
            return false;
        }

        UpdateTarget();

        return true;
    }

    XYZEngine::SubscriptionId InteractionComponent::SubscribePromptChanged(std::function<void(const std::string&)> onPromptChanged)
    {
        return promptChangedEvent.Subscribe(std::move(onPromptChanged));
    }

    XYZEngine::SubscriptionId InteractionComponent::SubscribeRefused(std::function<void(const std::string&)> onRefused)
    {
        return refusedEvent.Subscribe(std::move(onRefused));
    }
}
