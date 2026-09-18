#pragma once

#include <vector>
#include <SFML/Graphics/Color.hpp>
#include <Component.h>

namespace XYZEngine
{
    class VertexArrayRendererComponent;
}

namespace RoguelikeGame
{
    /**
    *	Красит квады своего куска полотна по памяти игрока: видимое в полную силу,
    *	разведанное приглушённо, неразведанное прозрачным. Геометрия не трогается,
    *	отрисовка куска остаётся одним вызовом.
    *
    *	Кусок, в котором нет ни одной хотя бы разведанной клетки, не рисуется вовсе -
    *	это вторая ступень отсечения после экранной.
    */
    class TileFogComponent : public XYZEngine::Component
    {
    public:
        struct Cell
        {
            std::size_t quad = 0u;
            int column = 0;
            int row = 0;
            sf::Color base;

            // Показанная яркость углов в том же порядке, что у SetQuadCorners.
            float shown[4] = {0.f, 0.f, 0.f, 0.f};
        };

        TileFogComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override {}

        void SetRenderer(XYZEngine::VertexArrayRendererComponent* newRenderer);
        void AddCell(std::size_t quad, int column, int row, const sf::Color& base);

        std::size_t GetCellsCount() const;
        int GetPaintCount() const;

    private:
        XYZEngine::VertexArrayRendererComponent* renderer = nullptr;
        std::vector<Cell> cells;

        unsigned int paintedVersion = 0u;
        bool isPainted = false;
        bool isMoving = false;
        int paintCount = 0;

        bool Approach(float deltaTime);
        void Paint();
    };
}
