#pragma once

#include <istream>
#include <string>
#include <vector>

namespace RoguelikeGame
{
    /**
    *	Зачем локация лежит в реестре.
    *
    *	Реестр держит и сюжет, и отладочные карты, и карты режимов. Без вида
    *	порядок строк служит им всем сразу, и любая карта, положенная в список,
    *	молча становится частью прохождения.
    */
    enum class LevelMode : unsigned char
    {
        Campaign,
        Arena,
        Test
    };

    // Неизвестное слово - предупреждение и campaign: опечатка в реестре не должна ронять игру.
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

        // С неё начинается забег или режим; -1, если карт такого вида в реестре нет.
        int FirstIndex(LevelMode mode) const;

        // Следующая по порядку локация того же вида; -1, если она последняя.
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
