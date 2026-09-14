#pragma once

#include <istream>
#include <string>
#include <vector>

namespace RoguelikeGame
{
    struct LootEntry
    {
        std::string itemId;
        int weight = 1;
        int count = 1;
    };

    struct LootDrop
    {
        std::string itemId;
        int count = 0;

        bool IsEmpty() const;
    };

    struct LootTable
    {
        std::string id;
        int rolls = 1;
        int emptyWeight = 0;
        std::vector<LootEntry> entries;

        int GetTotalWeight() const;
        LootDrop Pick(int roll) const;
    };

    class LootCatalog
    {
    public:
        static LootCatalog Load(const std::string& filePath);
        static LootCatalog Parse(std::istream& input, const std::string& sourceName);
        static const LootCatalog& Empty();

        const LootTable* Find(const std::string& id) const;

        std::size_t Size() const;
        bool IsEmpty() const;

        std::vector<LootTable>::const_iterator begin() const;
        std::vector<LootTable>::const_iterator end() const;

    private:
        std::vector<LootTable> tables;

        static std::string Trim(const std::string& line);
    };
}
