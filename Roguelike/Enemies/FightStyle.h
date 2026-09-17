#pragma once

#include <istream>
#include <stdexcept>
#include <string>
#include <LoggerRegistry.h>

namespace RoguelikeGame
{
    /**
    *	Чем враг занят в бою, кроме стрельбы.
    *
    *	Тактика уместна не везде: преследователи в погоне должны давить, а не
    *	отступать и пережидать. Два отдельных выключателя, а не один, чтобы
    *	«не пятится» и «не прячется» могли разойтись.
    */
    struct FightStyle
    {
        bool keepsDistance = true;
        bool takesCover = true;
    };

    inline constexpr FightStyle TACTICAL_FIGHT = {};
    inline constexpr FightStyle RELENTLESS_FIGHT = {false, false};

    inline constexpr const char* TACTICAL_FIGHT_NAME = "tactical";
    inline constexpr const char* RELENTLESS_FIGHT_NAME = "relentless";

    inline bool ParseFightStyle(const std::string& name, FightStyle& style)
    {
        if (name == TACTICAL_FIGHT_NAME)
        {
            style = TACTICAL_FIGHT;
            return true;
        }

        if (name == RELENTLESS_FIGHT_NAME)
        {
            style = RELENTLESS_FIGHT;
            return true;
        }

        return false;
    }

    // Один разбор на все секции конфига: и у карты, и у волн, и у погони ключ один.
    inline void ReadFightStyle(std::istream& stream, int lineNumber, FightStyle& style)
    {
        std::string name;
        if (!(stream >> name) || !ParseFightStyle(name, style))
        {
            LOG_ERROR("Line " + std::to_string(lineNumber) + " has an unknown fight style: " + name);
            throw std::runtime_error("Unknown fight style");
        }
    }
}
