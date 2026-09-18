#pragma once

#include <istream>
#include <string>
#include <vector>

namespace RoguelikeGame
{
    // Реестр держит и сюжет, и отладочные карты, и карты режимов.
    enum class LevelMode : unsigned char
    {
        Campaign,
        Arena,
        Test
    };

    LevelMode ParseLevelMode(const std::string& word, bool& isKnown);

    struct LevelEntry
    {
        std::string id;
        std::string title;
        std::string filePath;
        bool isAct = false;
        LevelMode mode = LevelMode::Campaign;
    };

    class LevelCatalog
    {
    public:
        static LevelCatalog Load(const std::string& filePath);
        static LevelCatalog Parse(std::istream& input, const std::string& sourceName);

        const LevelEntry* Find(const std::string& id) const;
        const LevelEntry* GetFirst() const;
        const LevelEntry* GetAt(int index) const;
        int IndexOf(const std::string& id) const;

        // -1, если карт такого вида в реестре нет.
        int FirstIndex(LevelMode mode) const;
        int NextIndex(LevelMode mode, int afterIndex) const;

        std::size_t Size() const;
        bool IsEmpty() const;

        std::vector<LevelEntry>::const_iterator begin() const;
        std::vector<LevelEntry>::const_iterator end() const;

    private:
        std::vector<LevelEntry> levels;

        static std::string Trim(const std::string& line);
    };
}
