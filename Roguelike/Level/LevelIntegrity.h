#pragma once

#include <string>
#include <vector>
#include "LevelData.h"

namespace RoguelikeGame
{
    class ItemCatalog;
    class PropCatalog;

    enum class LevelFault
    {
        NoStart,
        ManyStarts,
        OpenBorder,
        Unreachable,
        NoExit,
        ExitUnreachable,
        LockedOut
    };

    struct FaultName
    {
        const char* name;
        LevelFault fault;
    };

    constexpr FaultName FAULT_NAMES[] = {
        {"NoStart", LevelFault::NoStart},
        {"ManyStarts", LevelFault::ManyStarts},
        {"OpenBorder", LevelFault::OpenBorder},
        {"Unreachable", LevelFault::Unreachable},
        {"NoExit", LevelFault::NoExit},
        {"ExitUnreachable", LevelFault::ExitUnreachable},
        {"LockedOut", LevelFault::LockedOut}
    };

    const char* NameOf(LevelFault fault);

    struct LevelIssue
    {
        LevelFault fault = LevelFault::NoStart;
        int column = 0;
        int row = 0;
        std::string detail;
    };

    struct LevelReport
    {
        std::vector<LevelIssue> issues;

        bool IsClean() const;
        int Count(LevelFault fault) const;
        std::string Describe() const;
    };

    LevelReport CheckLevel(const LevelData& levelData, const ItemCatalog& items, const PropCatalog& props);
    LevelReport CheckLevel(const LevelData& levelData, const ItemCatalog& items);
    LevelReport CheckLevel(const LevelData& levelData);
}
