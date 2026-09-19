#pragma once

#include <istream>
#include <map>
#include <string>
#include "LevelData.h"

namespace RoguelikeGame
{
    class LevelLoader
    {
    public:
        static LevelData Load(const std::string& filePath);
        static LevelData Parse(std::istream& input, const std::string& sourceName);

    private:
        struct LegendEntry
        {
            TileType tile = TileType::Floor;
            std::string itemId;
            std::string propId;
            std::string patrolId;
            int patrolOrder = 0;
            bool patrolWatch = false;
            std::string doorId;
            std::string zoneId;
            std::string leverId;
            float holdTime = 0.f;
            std::string plateId;
            std::string hatchId;
            std::string escapeId;
            float propAngle = 0.f;
        };

        using Legend = std::map<char, LegendEntry>;

        // Секция одна на строку: пока их было пять, каждая гасила остальные
        // вручную, и шестая утонула бы в этом сама.
        enum class Section
        {
            Map,
            Level,
            Legend,
            Waves,
            Ambush,
            Pursuit,
            Overlay
        };

        static bool IsSection(const std::string& line, const std::string& sectionName);
        static void ReadInfoLine(const std::string& line, int lineNumber, LevelInfo& info);
        static void Validate(const LevelData& levelData, const std::string& sourceName);
        static void ReadLegendLine(const std::string& line, int lineNumber, Legend& legend);
        static void ReadWaveLine(const std::string& line, int lineNumber, const Legend& legend, LevelData& levelData);
        static void ReadAmbushLine(const std::string& line, int lineNumber, const Legend& legend, LevelData& levelData);
        static std::vector<WaveEntry> ReadWaveGroups(std::istringstream& stream, int lineNumber, const Legend& legend);
        static void ReadMapLine(const std::string& line, const Legend& legend, LevelData& levelData);
        static void ReadOverlayLine(const std::string& line, const Legend& legend, LevelData& levelData);
        static bool TryGetTileType(const std::string& name, TileType& tileType);
        static const Legend& GetDefaultLegend();
        static std::string Trim(const std::string& line);
    };
}
