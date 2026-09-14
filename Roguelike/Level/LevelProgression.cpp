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

        int index = currentIndex + 1;
        if (levels.GetAt(index) == nullptr)
        {
            return {LevelStepKind::Finished, -1};
        }

        return {LevelStepKind::Next, index};
    }
}
