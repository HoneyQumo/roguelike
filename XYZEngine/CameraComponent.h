#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include "TransformComponent.h"

namespace XYZEngine
{
    class CameraComponent : public Component
    {
    public:
        CameraComponent(GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetWindow(sf::RenderWindow* newWindow);
        void ZoomBy(float newZoom);

        void SetBaseResolution(int width, int height);
        void SetRotationEnabled(bool newIsRotationEnabled);

    private:
        TransformComponent* transform = nullptr;
        sf::RenderWindow* window = nullptr;
        sf::View view = sf::View(sf::FloatRect(0.f, 0.f, 800.f, -600.f));

        bool isRotationEnabled = true;
    };
}
