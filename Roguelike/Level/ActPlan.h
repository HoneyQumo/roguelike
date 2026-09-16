#pragma once

#include <istream>
#include <map>
#include <string>
#include <vector>
#include "LevelData.h"

namespace RoguelikeGame
{
    struct RoomPlacement
    {
        std::string roomId;
        int column = 0;
        int row = 0;
        int quarters = 0;
        bool isMirrored = false;
    };

    struct ActPlan
    {
        LevelInfo info;
        std::map<std::string, std::string> library;
        std::vector<RoomPlacement> rooms;
        std::vector<WaveSpec> waves;
        PursuitSpec pursuit;

        bool IsEmpty() const { return rooms.empty(); }
    };

    class ActLoader
    {
    public:
        static ActPlan Load(const std::string& filePath);
        static ActPlan Parse(std::istream& input, const std::string& sourceName);
    };
}
