#pragma once

#include <vector>
#include <Component.h>

namespace XYZEngine
{
    class VertexArrayRendererComponent;
}

namespace RoguelikeGame
{
    /**
    *	Крутит кадры у части квадов общего массива тайлов: сама геометрия не трогается,
    *	меняются только текстурные координаты, поэтому отрисовка остаётся одним вызовом.
    *
    *	Компонент ничего не знает про воду - ему дают строку атласа и список клеток.
    */
    class TileAnimationComponent : public XYZEngine::Component
    {
    public:
        struct Cell
        {
            std::size_t quad = 0u;
            unsigned int phase = 0u;
        };

        TileAnimationComponent(XYZEngine::GameObject* gameObject);

        void Update(float deltaTime) override;
        void Render() override;

        void SetRenderer(XYZEngine::VertexArrayRendererComponent* newRenderer);
        void SetStrip(int atlasRow, int framesCount, float frameTime);
        void AddCell(std::size_t quad, unsigned int phase);

        int GetStep() const;
        std::size_t GetCellsCount() const;

    private:
        XYZEngine::VertexArrayRendererComponent* renderer = nullptr;
        std::vector<Cell> cells;

        int row = 0;
        int frames = 1;
        float frameTime = 0.f;
        float sinceFrame = 0.f;
        int step = 0;

        void ShowStep();
    };
}
