#include "LevelProgression.h"

namespace RoguelikeGame
{
    LevelStep ResolveNextLevel(const LevelCatalog& levels, int currentIndex, const std::string& nextLevelId)
    {
        if (!nextLevelId.empty())
        {
            int index = levels.IndexOf(nextLevelId);
            if (index < 0)
            {
                return {LevelStepKind::Unknown, -1};
            }

            return {LevelStepKind::Next, index};
        }

        // Шаг по умолчанию идёт по своей цепочке: иначе сюжет утекает в первую же
        // отладочную карту, положенную рядом в реестре, и конца забега не наступает.
        const LevelEntry* current = levels.GetAt(currentIndex);
        LevelMode mode = current != nullptr ? current->mode : LevelMode::Campaign;

        int index = levels.NextIndex(mode, currentIndex);
        if (index < 0)
        {
            return {LevelStepKind::Finished, -1};
        }

        return {LevelStepKind::Next, index};
    }
}
