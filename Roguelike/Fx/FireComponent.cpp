#include "FireComponent.h"
#include "AreaDamage.h"
#include "DamageInfo.h"
#include "GameSettings.h"
#include <GameObject.h>
#include <LoggerRegistry.h>
#include <TransformComponent.h>

namespace RoguelikeGame
{
    FireComponent::FireComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void FireComponent::Start()
    {
        transform = gameObject->GetTransform();
    }

    void FireComponent::Update(float deltaTime)
    {
        if (IsFlying())
        {
            Fly(deltaTime);
            return;
        }

        Burn(deltaTime);
    }

    void FireComponent::Render()
    {
    }

    void FireComponent::Light(float seconds)
    {
        left = seconds > 0.f ? seconds : 0.f;
    }

    // Очаг вылетает из эпицентра и садится на своё место по дуге.
    void FireComponent::Throw(const XYZEngine::Vector2Df& newFrom, const XYZEngine::Vector2Df& newTo, float seconds)
    {
        from = newFrom;
        to = newTo;
        flightTime = seconds > 0.f ? seconds : 0.f;
        inFlight = 0.f;

        if (transform != nullptr)
        {
            transform->SetWorldPosition(from);
        }
    }

    void FireComponent::SetRadius(float newRadius)
    {
        radius = newRadius;
    }

    void FireComponent::SetDamage(float newDamage)
    {
        damage = newDamage;
    }

    void FireComponent::SetBeatTime(float newBeatTime)
    {
        beatTime = newBeatTime;
    }

    bool FireComponent::IsBurning() const
    {
        return left > 0.f;
    }

    bool FireComponent::IsFlying() const
    {
        return inFlight < flightTime;
    }

    float FireComponent::GetLeft() const
    {
        return left;
    }

    XYZEngine::SubscriptionId FireComponent::SubscribeBurnedOut(std::function<void()> onBurnedOut)
    {
        return burnedOutEvent.Subscribe(std::move(onBurnedOut));
    }

    void FireComponent::Fly(float deltaTime)
    {
        inFlight += deltaTime;

        float part = flightTime > 0.f ? inFlight / flightTime : 1.f;
        part = part > 1.f ? 1.f : part;

        XYZEngine::Vector2Df place = from + part * (to - from);

        // Горка: очаг подлетает и опускается, а не ползёт по прямой.
        float hop = FIRE_THROW_HOP * part * (1.f - part) * 4.f;
        place.y += hop;

        if (transform != nullptr)
        {
            transform->SetWorldPosition(place);
        }
    }

    void FireComponent::Burn(float deltaTime)
    {
        if (left <= 0.f)
        {
            return;
        }

        left -= deltaTime;
        sinceBeat += deltaTime;

        if (beatTime > 0.f && sinceBeat >= beatTime)
        {
            sinceBeat -= beatTime;
            Sting();
        }

        if (left <= 0.f)
        {
            left = 0.f;
            burnedOutEvent.Invoke();
        }
    }

    // Огонь ничей: горит и врагов, и игрока.
    void FireComponent::Sting()
    {
        if (transform == nullptr || radius <= 0.f || damage <= 0.f)
        {
            return;
        }

        AreaQuery found = QueryDamageArea(transform->GetWorldPosition(), radius, gameObject);
        for (const AreaTarget& target : found.targets)
        {
            if (target.health == nullptr)
            {
                continue;
            }

            DamageSource source;
            source.kind = DamageKind::Burn;
            source.attackerName = gameObject->GetName();
            source.attackerFaction = Faction::Neutral;
            source.position = transform->GetWorldPosition();
            source.direction = target.direction;

            target.health->TakeDamage(damage, source);
        }
    }
}
