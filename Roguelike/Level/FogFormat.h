#pragma once

#include <istream>
#include <string>
#include <LoggerRegistry.h>
#include "GameSettings.h"

namespace RoguelikeGame
{
    inline const std::string FOG_KEYWORD = "fog";

    /**
    *	Туман - свойство локации, а не игры: в подземелье он нужен, на мосту
    *	посреди погони - нет. Радиус задаётся в клетках, ноль его выключает.
    */
    inline bool ParseFogRadius(const std::string& value, int& radius)
    {
        if (value == "off")
        {
            radius = 0;
            return true;
        }

        if (value == "on")
        {
            radius = FOG_SIGHT_RADIUS;
            return true;
        }

        if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
        {
            return false;
        }

        radius = std::stoi(value);

        return true;
    }

    inline void ReadFogRadius(std::istream& stream, int lineNumber, int& radius)
    {
        std::string value;
        stream >> value;

        if (!ParseFogRadius(value, radius))
        {
            LOG_WARN("Unknown fog value at line " + std::to_string(lineNumber) + ": " + value);
        }
    }
}
