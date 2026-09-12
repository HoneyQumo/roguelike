#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include "EventList.h"
#include "TransformComponent.h"

namespace XYZEngine
{
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
        void SetRotationEnabled(bool newIsRotationEnabled);

    private:
        TransformComponent* transform = nullptr;
        sf::View view = sf::View(sf::FloatRect(0.f, 0.f, 800.f, -600.f));

        float viewHeight = 600.f;
        float zoom = 1.f;
        bool isRotationEnabled = true;
        SubscriptionId resizeSubscription = NO_SUBSCRIPTION;

        void ApplyViewSize();
    };
}
