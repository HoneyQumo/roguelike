#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include "EventList.h"
#include "TransformComponent.h"

namespace XYZEngine
{
    struct CameraShake
    {
        float amplitude = 0.f;
        float duration = 0.f;
        float frequency = 16.f;
    };

    class CameraComponent : public Component
    {
    public:
        CameraComponent(GameObject* gameObject);
        ~CameraComponent() override;

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void ZoomBy(float newZoom);

        void SetViewHeight(float newViewHeight);

        // Ширина и высота кадра в мире: по ним решают, где кончается видимое.
        Vector2Df GetViewSize() const;
        void SetRotationEnabled(bool newIsRotationEnabled);

        void Shake(const CameraShake& newShake);
        void StopShake();
        const Vector2Df& GetShakeOffset() const;
        void SetMaxShakeAmplitude(float newMaxShakeAmplitude);

    private:
        TransformComponent* transform = nullptr;
        sf::View view = sf::View(sf::FloatRect(0.f, 0.f, 800.f, -600.f));

        float viewHeight = 600.f;
        float zoom = 1.f;
        bool isRotationEnabled = true;
        SubscriptionId resizeSubscription = NO_SUBSCRIPTION;

        CameraShake shake;
        float shakeTime = 0.f;
        float maxShakeAmplitude = 48.f;
        Vector2Df shakeOffset = {0.f, 0.f};

        void ApplyViewSize();
        void UpdateShake(float deltaTime);
        float GetShakeDecay() const;
    };
}
