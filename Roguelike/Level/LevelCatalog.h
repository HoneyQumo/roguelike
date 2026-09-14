#pragma once

#include <istream>
#include <string>
#include <vector>

namespace RoguelikeGame
{
    struct LevelEntry
    {
        std::string id;
        std::string title;
        std::string filePath;
        bool isAct = false;
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

        std::size_t Size() const;
        bool IsEmpty() const;

        std::vector<LevelEntry>::const_iterator begin() const;
        std::vector<LevelEntry>::const_iterator end() const;

    private:
        std::vector<LevelEntry> levels;

        static std::string Trim(const std::string& line);
    };
}
