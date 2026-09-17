#include "FogRevealComponent.h"
#include "FogOfWar.h"
#include "LevelGrid.h"
#include <GameObject.h>
#include <GameWorld.h>

namespace RoguelikeGame
{
    FogRevealComponent::FogRevealComponent(XYZEngine::GameObject* gameObject)
        : Component(gameObject)
    {
    }

    void FogRevealComponent::Update(float deltaTime)
    {
        FogOfWar& fog = FogOfWar::Current();
        if (!fog.IsEnabled())
        {
            return;
        }

        if (target == nullptr && !targetName.empty())
        {
            target = XYZEngine::GameWorld::Instance()->FindGameObject(targetName);
        }

        if (target == nullptr || target->GetTransform() == nullptr)
        {
            return;
        }

        const LevelGrid& grid = LevelGrid::Current();
        XYZEngine::Vector2Df position = target->GetTransform()->GetWorldPosition();

        int cellColumn = 0;
        int cellRow = 0;
        grid.ToCell(position, cellColumn, cellRow);

        // Обзор меняется, только когда игрок сменил клетку или открылась дверь:
        // считать линию видимости до каждой клетки каждый кадр незачем.
        bool isMoved = !hasCell || cellColumn != column || cellRow != row;
        bool isGridChanged = grid.GetVersion() != gridVersion;
        if (!isMoved && !isGridChanged)
        {
            return;
        }

        hasCell = true;
        column = cellColumn;
        row = cellRow;
        gridVersion = grid.GetVersion();
        revealCount++;

        fog.Reveal(grid, position);
    }

    void FogRevealComponent::SetTargetName(const std::string& newTargetName)
    {
        targetName = newTargetName;
        target = nullptr;
    }

    int FogRevealComponent::GetRevealCount() const
    {
        return revealCount;
    }
}
