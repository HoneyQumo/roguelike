#include "LootCatalog.h"
#include <LoggerRegistry.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace RoguelikeGame
{
    namespace
    {
        constexpr char LOOT_COMMENT_SYMBOL = ';';
        const std::string LOOT_WHITESPACE = " \t";
        const std::string LOOT_UTF8_BOM = "\xEF\xBB\xBF";
        const std::string LOOT_BLOCK_PREFIX = "[table ";
    }

    bool LootDrop::IsEmpty() const
    {
        return itemId.empty() || count <= 0;
    }

    int LootTable::GetTotalWeight() const
    {
        int total = emptyWeight;
        for (const LootEntry& entry : entries)
        {
            total += entry.weight;
        }

        return total;
    }

    LootDrop LootTable::Pick(int roll) const
    {
        int total = GetTotalWeight();
        if (total <= 0)
        {
            return {};
        }

        int left = std::clamp(roll, 0, total - 1);
        for (const LootEntry& entry : entries)
        {
            if (left < entry.weight)
            {
                return {entry.itemId, entry.count};
            }

            left -= entry.weight;
        }

        return {};
    }

    LootCatalog LootCatalog::Load(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_ERROR("Can't open loot catalog: " + filePath);
            throw std::runtime_error("Loot catalog is not available: " + filePath);
        }

        return Parse(file, filePath);
    }

    LootCatalog LootCatalog::Parse(std::istream& input, const std::string& sourceName)
    {
        LootCatalog catalog;
        LootTable current;
        bool hasCurrent = false;

        std::string line;
        int lineNumber = 0;

        while (std::getline(input, line))
        {
            lineNumber++;

            if (lineNumber == 1 && line.compare(0, LOOT_UTF8_BOM.size(), LOOT_UTF8_BOM) == 0)
            {
                line.erase(0, LOOT_UTF8_BOM.size());
            }

            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            line = Trim(line);
            if (line.empty() || line.front() == LOOT_COMMENT_SYMBOL)
            {
                continue;
            }

            if (line.compare(0, LOOT_BLOCK_PREFIX.size(), LOOT_BLOCK_PREFIX) == 0 && line.back() == ']')
            {
                if (hasCurrent)
                {
                    catalog.tables.push_back(current);
                }

                current = LootTable();
                current.id = Trim(line.substr(LOOT_BLOCK_PREFIX.size(), line.size() - LOOT_BLOCK_PREFIX.size() - 1));
                hasCurrent = true;

                if (current.id.empty())
                {
                    LOG_ERROR("Loot table has no id, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Loot table has no id in " + sourceName);
                }

                continue;
            }

            if (!hasCurrent)
            {
                LOG_ERROR("Loot line outside of a table, line " + std::to_string(lineNumber) + " in " + sourceName);
                throw std::runtime_error("Loot line outside of a table in " + sourceName);
            }

            std::istringstream stream(line);
            std::string key;
            stream >> key;

            if (key == "drop")
            {
                LootEntry entry;
                if (!(stream >> entry.itemId))
                {
                    LOG_ERROR("Loot drop has no item id, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Loot drop has no item id in " + sourceName);
                }

                if (!(stream >> entry.weight))
                {
                    entry.weight = 1;
                }

                if (!(stream >> entry.count))
                {
                    entry.count = 1;
                }

                if (entry.weight <= 0 || entry.count <= 0)
                {
                    LOG_ERROR("Loot drop must be positive, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Loot drop must be positive in " + sourceName);
                }

                current.entries.push_back(entry);
                continue;
            }

            if (key == "nothing")
            {
                if (!(stream >> current.emptyWeight) || current.emptyWeight < 0)
                {
                    LOG_ERROR("Loot empty weight must not be negative, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Loot empty weight must not be negative in " + sourceName);
                }

                continue;
            }

            if (key == "rolls")
            {
                if (!(stream >> current.rolls) || current.rolls <= 0)
                {
                    LOG_ERROR("Loot rolls must be positive, line " + std::to_string(lineNumber) + " in " + sourceName);
                    throw std::runtime_error("Loot rolls must be positive in " + sourceName);
                }

                continue;
            }

            LOG_WARN("Unknown loot field at line " + std::to_string(lineNumber) + ": " + key);
        }

        if (hasCurrent)
        {
            catalog.tables.push_back(current);
        }

        LOG_INFO("Loot tables loaded: " + std::to_string(catalog.tables.size()));
        return catalog;
    }

    const LootCatalog& LootCatalog::Empty()
    {
        static const LootCatalog empty;
        return empty;
    }

    const LootTable* LootCatalog::Find(const std::string& id) const
    {
        if (id.empty())
        {
            return nullptr;
        }

        for (const LootTable& table : tables)
        {
            if (table.id == id)
            {
                return &table;
            }
        }

        return nullptr;
    }

    std::size_t LootCatalog::Size() const
    {
        return tables.size();
    }

    bool LootCatalog::IsEmpty() const
    {
        return tables.empty();
    }

    std::vector<LootTable>::const_iterator LootCatalog::begin() const
    {
        return tables.begin();
    }

    std::vector<LootTable>::const_iterator LootCatalog::end() const
    {
        return tables.end();
    }

    std::string LootCatalog::Trim(const std::string& line)
    {
        size_t first = line.find_first_not_of(LOOT_WHITESPACE);
        if (first == std::string::npos)
        {
            return "";
        }

        size_t last = line.find_last_not_of(LOOT_WHITESPACE);

        return line.substr(first, last - first + 1);
    }
}
