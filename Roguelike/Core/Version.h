#pragma once

#include <string>

namespace RoguelikeGame
{
    // Числа читает не только компилятор, но и сборка релиза - она достаёт их регуляркой.
    constexpr int VERSION_MAJOR = 0;
    constexpr int VERSION_MINOR = 1;
    constexpr int VERSION_PATCH = 0;

    // Пусто у обычного выпуска, alpha или rc.1 - у предрелизного.
    constexpr const char* VERSION_STAGE = "alpha";

    constexpr const char* GAME_NAME = "Roguelike by HoneyQumo";

    inline std::string FormatVersion(int major, int minor, int patch, const char* stage)
    {
        std::string version = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);

        if (stage != nullptr && *stage != '\0')
        {
            version += "-";
            version += stage;
        }

        return version;
    }

    inline std::string VersionString()
    {
        return FormatVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_STAGE);
    }

    inline std::string GameTitle()
    {
        return std::string(GAME_NAME) + " " + VersionString();
    }
}
