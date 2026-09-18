#include "TileFogComponent.h"
#include "FogOfWar.h"
#include "GameSettings.h"
#include <VertexArrayRendererComponent.h>

namespace RoguelikeGame
{
    namespace
    {
        sf::Color Dim(const sf::Color& color, float light)
        {
            return {static_cast<sf::Uint8>(color.r * light), static_cast<sf::Uint8>(color.g * light),
                static_cast<sf::Uint8>(color.b * light), color.a};
        }

        sf::Color ColorFor(const sf::Color& base, float light)
        {
            return light <= 0.f ? sf::Color::Transparent : Dim(base, light);
        }
    }

    TileFogComponent::TileFogComponent(XYZEngine::GameObject* gameObject)
        : Component(gameObject)
    {
    }

    void TileFogComponent::Update(float deltaTime)
    {
        const FogOfWar& fog = FogOfWar::Current();
        if (!fog.IsEnabled() || renderer == nullptr)
        {
            return;
        }

        if (isPainted && fog.GetVersion() == paintedVersion)
        {
            return;
        }

        paintedVersion = fog.GetVersion();
        isPainted = true;

        Paint();
    }

    void TileFogComponent::Paint()
    {
        const FogOfWar& fog = FogOfWar::Current();
        bool hasAnything = false;

        for (const Cell& cell : cells)
        {
            hasAnything = hasAnything || fog.GetState(cell.column, cell.row) != FogState::Unseen;

            // Углы порознь: градиент между ними видеокарта растягивает сама,
            // поэтому мягкий край не стоит ни шейдера, ни второго слоя.
            renderer->SetQuadCorners(cell.quad,
                ColorFor(cell.base, fog.GetCornerLight(cell.column, cell.row + 1)),
                ColorFor(cell.base, fog.GetCornerLight(cell.column + 1, cell.row + 1)),
                ColorFor(cell.base, fog.GetCornerLight(cell.column + 1, cell.row)),
                ColorFor(cell.base, fog.GetCornerLight(cell.column, cell.row)));
        }

        renderer->SetEnabled(hasAnything);
        paintCount++;
    }

    void TileFogComponent::SetRenderer(XYZEngine::VertexArrayRendererComponent* newRenderer)
    {
        renderer = newRenderer;
    }

    void TileFogComponent::AddCell(std::size_t quad, int column, int row, const sf::Color& base)
    {
        cells.push_back({quad, column, row, base});
    }

    std::size_t TileFogComponent::GetCellsCount() const
    {
        return cells.size();
    }

    int TileFogComponent::GetPaintCount() const
    {
        return paintCount;
    }
}
