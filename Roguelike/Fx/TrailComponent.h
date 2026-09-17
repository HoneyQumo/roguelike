#pragma once

#include <vector>
#include <SFML/Graphics/Color.hpp>
#include <Component.h>
#include <Vector.h>
#include "Trail.h"

namespace XYZEngine
{
    class VertexArrayRendererComponent;
}

namespace RoguelikeGame
{
    /**
    *	Сплошной след на земле: шлейф от покрышек, кровавый волок, дымовая полоса.
    *
    *	Точки копятся по расстоянию, а не по кадрам: иначе на медленном ходу
    *	лента вырождается в кучу точек в одном месте, а на быстром рвётся.
    *
    *	Вся лента - одна геометрия и один вызов отрисовки, поэтому длина ей
    *	почти ничего не стоит.
    */
    class TrailComponent : public XYZEngine::Component
    {
    public:
        TrailComponent(XYZEngine::GameObject* gameObject);

        void Start() override;
        void Update(float deltaTime) override;
        void Render() override;

        void SetColour(const sf::Color& newColour);
        void SetWidth(float newWidth);
        void SetStep(float newStep);

        // Добавляет точку, если от последней ушли дальше шага. Вернёт true, если добавил.
        bool Add(const XYZEngine::Vector2Df& place, float alpha = 1.f);
        void Cut();

        std::size_t GetPointsCount() const;
        const std::vector<TrailPoint>& GetPoints() const;

    private:
        void Rebuild();

        std::vector<TrailPoint> points;
        XYZEngine::VertexArrayRendererComponent* ribbon = nullptr;
        sf::Color colour = sf::Color::White;
        float width = 1.f;
        float step = 1.f;
        bool isDirty = false;
    };
}
