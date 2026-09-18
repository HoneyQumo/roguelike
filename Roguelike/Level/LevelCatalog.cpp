#include "LevelCatalog.h"
#include <LoggerRegistry.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace RoguelikeGame
{
    namespace
    {
        constexpr char LEVEL_COMMENT_SYMBOL = ';';
        const std::string LEVEL_WHITESPACE = " \t";
        const std::string LEVEL_UTF8_BOM = "\xEF\xBB\xBF";
        const std::string LEVEL_BLOCK_PREFIX = "[level ";

        const std::string LEVEL_MODE_CAMPAIGN = "campaign";
        const std::string LEVEL_MODE_ARENA = "arena";
        const std::string LEVEL_MODE_TEST = "test";
    }

    LevelMode ParseLevelMode(const std::string& word, bool& isKnown)
    {
        isKnown = true;

        if (word == LEVEL_MODE_CAMPAIGN)
        {
            return LevelMode::Campaign;
        }

        if (word == LEVEL_MODE_ARENA)
        {
            return LevelMode::Arena;
        }

        if (word == LEVEL_MODE_TEST)
        {
            return LevelMode::Test;
        }

        isKnown = false;
        return LevelMode::Campaign;
    }

    LevelCatalog LevelCatalog::Load(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            LOG_ERROR("Can't open level catalog: " + filePath);
            throw std::runtime_error("Level catalog is not available: " + filePath);
        }

        return Parse(file, filePath);
    }

    LevelCatalog LevelCatalog::Parse(std::istream& input, const std::string& sourceName)
    {
        LevelCatalog catalog;
        LevelEntry current;
        bool hasCurrent = false;

        std::string line;
        int lineNumber = 0;

        while (std::getline(input, line))
        {
            lineNumber++;

            if (lineNumber == 1 && line.compare(0, LEVEL_UTF8_BOM.size(), LEVEL_UTF8_BOM) == 0)
            {
                line.erase(0, LEVEL_UTF8_BOM.size());
            }

            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            line = Trim(line);
            if (line.empty() || line.front() == LEVEL_COMMENT_SYMBOL)
            {
                continue;
            }

            if (line.compare(0, LEVEL_BLOCK_PREFIX.size(), LEVEL_BLOCK_PREFIX) == 0 && line.back() == ']')
            {
                if (hasCurrent)
                {
                    if (current.filePath.empty())
                    {
                        LOG_ERROR("Level " + current.id + " has no file");
                        throw std::runtime_error("Level has no file: " + current.id);
                    }

                    catalog.levels.push_back(current);
                }

                std::string id = Trim(line.substr(LEVEL_BLOCK_PREFIX.size(), line.size() - LEVEL_BLOCK_PREFIX.size() - 1));
                if (id.empty())
                {
                    LOG_ERROR("Level block has no id in " + sourceName + ", line " + std::to_string(lineNumber));
                    throw std::runtime_error("Level block has no id");
                }

                if (catalog.Find(id) != nullptr)
                {
                    LOG_ERROR("Duplicate level id in " + sourceName + ": " + id);
                    throw std::runtime_error("Duplicate level id: " + id);
                }

                current = LevelEntry();
                current.id = id;
                hasCurrent = true;
                continue;
            }

            if (!hasCurrent)
            {
                LOG_ERROR("Level field outside of a block in " + sourceName + ", line " + std::to_string(lineNumber));
                throw std::runtime_error("Level field outside of a block");
            }

            std::istringstream stream(line);
            std::string key;
            stream >> key;

            if (key == "title")
            {
                std::string rest;
                std::getline(stream, rest);
                current.title = Trim(rest);
                continue;
            }

            if (key == "file")
            {
                stream >> current.filePath;
                current.isAct = false;
                continue;
            }

            if (key == "act")
            {
                stream >> current.filePath;
                current.isAct = true;
                continue;
            }

            if (key == "mode")
            {
                std::string word;
                stream >> word;

                bool isKnown = false;
                current.mode = ParseLevelMode(word, isKnown);

                if (!isKnown)
                {
                    LOG_WARN("Unknown level mode at line " + std::to_string(lineNumber) + ": " + word);
                }

                continue;
            }

            LOG_WARN("Unknown level catalog field at line " + std::to_string(lineNumber) + ": " + key);
        }

        if (hasCurrent)
        {
            if (current.filePath.empty())
            {
                LOG_ERROR("Level " + current.id + " has no file");
                throw std::runtime_error("Level has no file: " + current.id);
            }

            catalog.levels.push_back(current);
        }

        return catalog;
    }

    const LevelEntry* LevelCatalog::Find(const std::string& id) const
    {
        for (const LevelEntry& level : levels)
        {
            if (level.id == id)
            {
                return &level;
            }
        }

        return nullptr;
    }

    const LevelEntry* LevelCatalog::GetFirst() const
    {
        return levels.empty() ? nullptr : &levels.front();
    }

    const LevelEntry* LevelCatalog::GetAt(int index) const
    {
        return index >= 0 && index < static_cast<int>(levels.size()) ? &levels[index] : nullptr;
    }

    int LevelCatalog::IndexOf(const std::string& id) const
    {
        for (int index = 0; index < static_cast<int>(levels.size()); index++)
        {
            if (levels[index].id == id)
            {
                return index;
            }
        }

        return -1;
    }

    int LevelCatalog::FirstIndex(LevelMode mode) const
    {
        return NextIndex(mode, -1);
    }

    int LevelCatalog::NextIndex(LevelMode mode, int afterIndex) const
    {
        // Индекс приходит снаружи, отрицательный увёл бы обход за начало вектора.
        int start = afterIndex < 0 ? 0 : afterIndex + 1;

        for (int index = start; index < static_cast<int>(levels.size()); index++)
        {
            if (levels[index].mode == mode)
            {
                return index;
            }
        }

        return -1;
    }

    std::size_t LevelCatalog::Size() const
    {
        return levels.size();
    }

    bool LevelCatalog::IsEmpty() const
    {
        return levels.empty();
    }

    std::vector<LevelEntry>::const_iterator LevelCatalog::begin() const
    {
        return levels.begin();
    }

    std::vector<LevelEntry>::const_iterator LevelCatalog::end() const
    {
        return levels.end();
    }

    std::string LevelCatalog::Trim(const std::string& line)
    {
        size_t first = line.find_first_not_of(LEVEL_WHITESPACE);
        if (first == std::string::npos)
        {
            return "";
        }

        size_t last = line.find_last_not_of(LEVEL_WHITESPACE);
        return line.substr(first, last - first + 1);
    }
}
