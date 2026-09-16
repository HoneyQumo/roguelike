#include "TileAnimationComponent.h"
#include "TileAtlas.h"
#include <GameObject.h>
#include <VertexArrayRendererComponent.h>

namespace RoguelikeGame
{
    TileAnimationComponent::TileAnimationComponent(XYZEngine::GameObject* gameObject) : Component(gameObject) {}

    void TileAnimationComponent::Update(float deltaTime)
    {
        if (renderer == nullptr || cells.empty() || frames <= 1 || frameTime <= 0.f)
        {
            return;
        }

        sinceFrame += deltaTime;
        if (sinceFrame < frameTime)
        {
            return;
        }

        // Пропущенные кадры не копим: после долгого затыка вода не должна отматывать назад.
        int passed = static_cast<int>(sinceFrame / frameTime);
        sinceFrame -= passed * frameTime;
        step = (step + passed) % frames;

        ShowStep();
    }

    void TileAnimationComponent::Render()
    {
    }

    void TileAnimationComponent::SetRenderer(XYZEngine::VertexArrayRendererComponent* newRenderer)
    {
        renderer = newRenderer;
    }

    void TileAnimationComponent::SetStrip(int atlasRow, int framesCount, float newFrameTime)
    {
        row = atlasRow;
        frames = framesCount > 1 ? framesCount : 1;
        frameTime = newFrameTime;
    }

    void TileAnimationComponent::AddCell(std::size_t quad, unsigned int phase)
    {
        cells.push_back({quad, phase});
    }

    int TileAnimationComponent::GetStep() const
    {
        return step;
    }

    std::size_t TileAnimationComponent::GetCellsCount() const
    {
        return cells.size();
    }

    void TileAnimationComponent::ShowStep()
    {
        for (const Cell& cell : cells)
        {
            int frame = static_cast<int>((cell.phase + static_cast<unsigned int>(step)) % static_cast<unsigned int>(frames));
            renderer->SetQuadFrame(cell.quad, TileFrameRect(row, frame));
        }
    }
}
